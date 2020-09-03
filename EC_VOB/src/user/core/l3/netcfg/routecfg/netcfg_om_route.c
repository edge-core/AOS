/* Module Name:NETCFG_OM_ROUTE.C
 * Purpose: To store configuration route entry.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       12/18/2007 --  Vai Wang    Created
 *
 * Copyright(C)      Accton Corporation, 2007.
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_sort_lst.h"
#include "netcfg_om_route.h"
#include "ip_lib.h"
#include "eh_mgr.h"
#include "eh_type.h"
#include "l_stdlib.h"
#include "l_radix.h"
#include "l_bitmap.h"
#include "leaf_2932.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
#define UNUSED __attribute__ ((__unused__))
/***************************************************************/
static UNUSED unsigned long DEBUG_FLAG = 0;//DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

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
#define BACKDOOR_SET_DEBUG_FLAG() do{\
    UI8_T   ch;\
    char currentflag[4];\
    currentflag[0]=(DEBUG_FLAG & DEBUG_FLAG_BIT_DEBUG)?'D':'-';\
    currentflag[1]=(DEBUG_FLAG & DEBUG_FLAG_BIT_INFO)?'I':'-';\
    currentflag[2]=(DEBUG_FLAG & DEBUG_FLAG_BIT_NOTE)?'N':'-';\
    currentflag[3]=0;\
    BACKDOOR_MGR_Printf("Select debug level [%s](1~3, set 0 to clear)\n",currentflag);\
    switch (ch)\
    {\
        case '0':\
            DEBUG_FLAG =0;\
            break;\
        case '1':\
            DEBUG_FLAG = DEBUG_FLAG_BIT_NOTE;\
            break;\
        case '2':\
            DEBUG_FLAG = DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO;\
            break;\
        case '3':\
            DEBUG_FLAG = DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;\
            break;\
        default:\
            break;\
    }\
}while(0)
/*END Simon's debug function*/
/* DATA TYPE DECLARATIONS
 */
/* Keys: ifindex, inet_gateway, distance
 */
typedef struct NETCFG_OM_ROUTE_NextHop_S
{
    /* ROUTE_NH_TYPE_XXX */
    UI32_T type;

    UI32_T ifindex;

    //UI32_T inet_gateway;
    //UI8_T inet6_gateway[SYS_ADPT_IPV6_ADDR_LEN];

    L_INET_AddrIp_T nexthop_addr; /* nexthop address for v4/v6 */

    /* Administrative Distance */
    UI32_T distance;

    UI32_T row_status;

}NETCFG_OM_ROUTE_NextHop_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* compare functions of sort_list */

static int NETCFG_OM_ROUTE_CompareIpForwardingStatus(void *elm1, void *elm2);

static int NETCFG_OM_ROUTE_CompareNextHops(void *elm1, void *elm2);
static BOOL_T NETCFG_OM_ROUTE_IsSameNextHop (NETCFG_OM_ROUTE_NextHop_T *nh, L_INET_AddrIp_T *gateway,
                                        UI32_T type, UI32_T ifindex);
static BOOL_T NETCFG_OM_ROUTE_GetNextHopType (L_INET_AddrIp_T *gateway, UI32_T ifindex, UI32_T *type);

#if (SYS_CPNT_IPV6 == TRUE)
static int NETCFG_OM_ROUTE_CompareIPV6NextHops(void *elm1, void *elm2);
#endif

//static void NETCFG_OM_ROUTE_DumpRoute(ROUTE_MGR_IpCidrRouteEntry_T entry);


/* STATIC VARIABLE DECLARATIONS
 */
static L_RADIX_Table_T static_route_table;
#if (SYS_CPNT_IPV6 == TRUE)
static L_RADIX_Table_T static_ipv6_route_table;
#endif

static UI32_T            netcfg_om_route_sem_id;

#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
static UI8_T   management_vlan_default_gateway[SYS_ADPT_IPV4_ADDR_LEN];
#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

typedef enum
{
    NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE = 0,
    NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP,
    NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_MAXINDEX,
} NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_T;
L_INET_AddrIp_T netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_MAXINDEX]; //index=NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_T
L_INET_AddrIp_T netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_MAXINDEX]; //index=NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_T


static UI32_T netcfg_route_static_route_number;
#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T netcfg_route_static_ipv6_route_number;
#endif
static UI32_T netcfg_route_mroute_status = VAL_ipMRouteEnable_disabled;
static UI32_T netcfg_route_m6route_status = VAL_ipMRouteEnable_disabled;
//static int routecfg_om_debug = 0;

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    static UI32_T   netcfg_route_ecmp_lbmode;
    static UI32_T   netcfg_route_ecmp_hsl_id;
#endif

L_SORT_LST_List_T *ip_forwarding_status_list;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : NETCFG_OM_ROUTE_InitateProcessResources
 * PURPOSE:
 *          Initialize semophore & create a list to store route entry.
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
void NETCFG_OM_ROUTE_InitateProcessResources(void)
{
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    netcfg_route_ecmp_lbmode = netcfg_route_ecmp_hsl_id = 0;
#endif

    L_RADIX_Create(&static_route_table);
#if (SYS_CPNT_IPV6 == TRUE)
    L_RADIX_Create(&static_ipv6_route_table);
#endif
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
    /* reset management vlan default gateway to be zero */
    management_vlan_default_gateway = 0;
#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

    memset(netcfg_route_default_gateway_compatable, 0, sizeof(netcfg_route_default_gateway_compatable));
    memset(netcfg_route_ipv6_default_gateway_compatable,0, sizeof(netcfg_route_ipv6_default_gateway_compatable));

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &netcfg_om_route_sem_id) != SYSFUN_OK)
    {
        DBG_PrintText("NETCFG_OM_ROUTE_Init: can't create semaphore \n");
        while (TRUE);
    }
    netcfg_route_static_route_number=0;
#if (SYS_CPNT_IPV6 == TRUE)
    netcfg_route_static_ipv6_route_number=0;
#endif

    /* Initialize the ip_forwarding_status_list */
    ip_forwarding_status_list = (L_SORT_LST_List_T *)malloc(sizeof (L_SORT_LST_List_T));
    if (NULL == ip_forwarding_status_list)
    {
        printf("ip_forwarding_status_list initialization failure\n");
        return;
    }
    memset(ip_forwarding_status_list, 0, sizeof(L_SORT_LST_List_T));

    if (L_SORT_LST_Create(ip_forwarding_status_list, SYS_ADPT_MAX_NUMBER_OF_VRF_IN_SYSTEM,
                sizeof(NETCFG_OM_ROUTE_IpForwardingStatus_T),
                NETCFG_OM_ROUTE_CompareIpForwardingStatus) == FALSE)
    {
        free(ip_forwarding_status_list);
        printf("ip_forwarding_status_list creation failure\n");
        return;
    }

}   /*  end of NETCFG_OM_ROUTE_Init  */


/* FUNCTION NAME : NETCFG_OM_ROUTE_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_OM_ROUTE.
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
BOOL_T NETCFG_OM_ROUTE_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_OM_ROUTE_IPCMsg_T *netcfg_om_route_msg_p;
    
    if (ipcmsg_p == NULL)
        return FALSE;
    
    netcfg_om_route_msg_p= (NETCFG_OM_ROUTE_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(netcfg_om_route_msg_p->type.cmd)
    {            
        case NETCFG_OM_ROUTE_IPC_GETNEXTSTATICIPCIDRROUTE:
            netcfg_om_route_msg_p->type.result_ui32 = NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&netcfg_om_route_msg_p->data.arg_route_entry);
            ipcmsg_p->msg_size = NETCFG_OM_ROUTE_GET_MSG_SIZE(arg_route_entry);
            break;
        case NETCFG_OM_ROUTE_IPC_GETIPFORWARDINGSTATUS:
            netcfg_om_route_msg_p->type.result_ui32 = NETCFG_OM_ROUTE_GetIpForwardingStatus(&netcfg_om_route_msg_p->data.arg_ip_forwarding);
            ipcmsg_p->msg_size = NETCFG_OM_ROUTE_GET_MSG_SIZE(arg_ip_forwarding);
            break;
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
        case NETCFG_OM_ROUTE_IPC_GETECMPBALANCEMODE:
            netcfg_om_route_msg_p->type.result_ui32 = NETCFG_OM_ROUTE_GetEcmpBalanceMode(
                &netcfg_om_route_msg_p->data.u32a1_u32a2.u32_a1,
                &netcfg_om_route_msg_p->data.u32a1_u32a2.u32_a2);
            ipcmsg_p->msg_size = NETCFG_OM_ROUTE_GET_MSG_SIZE(u32a1_u32a2);
            break;
        case NETCFG_OM_ROUTE_IPC_GETRUNNINGECMPBALANCEMODE:
            netcfg_om_route_msg_p->type.result_ui32 = NETCFG_OM_ROUTE_GetRunningEcmpBalanceMode(
                &netcfg_om_route_msg_p->data.u32a1_u32a2.u32_a1,
                &netcfg_om_route_msg_p->data.u32a1_u32a2.u32_a2);
            ipcmsg_p->msg_size = NETCFG_OM_ROUTE_GET_MSG_SIZE(u32a1_u32a2);
            break;
#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            netcfg_om_route_msg_p->type.result_ui32 = NETCFG_TYPE_FAIL;
            ipcmsg_p->msg_size = NETCFG_OM_ROUTE_MSGBUF_TYPE_SIZE;
            DBGprintf("Unknow message!");
            return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_ROUTE_EnableIpForwarding
 * PURPOSE:
 *      Enable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id       : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully enable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */

UI32_T NETCFG_OM_ROUTE_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type)
{
 NETCFG_OM_ROUTE_IpForwardingStatus_T ifs;
 ifs.vr_id = vr_id;

 if(NETCFG_OM_ROUTE_GetIpForwardingStatus(&ifs) == NETCFG_TYPE_FAIL)
  ifs.status_bitmap = 0;        // Initialize the bitamp since no entry for vr_id exists

 if(addr_type == L_INET_ADDR_TYPE_IPV4)
  SET_FLAG(ifs.status_bitmap, NETCFG_OM_ROUTE_FLAGS_IPV4);
 else if(addr_type == L_INET_ADDR_TYPE_IPV6)
  SET_FLAG(ifs.status_bitmap, NETCFG_OM_ROUTE_FLAGS_IPV6);

 if(L_SORT_LST_Set(ip_forwarding_status_list, &ifs))
  return NETCFG_TYPE_OK;
 return NETCFG_TYPE_FAIL;

}

/* FUNCTION NAME : NETCFG_OM_ROUTE_DisableIpForwarding
 * PURPOSE:
 *      Disable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id       : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully disable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type)
{
 NETCFG_OM_ROUTE_IpForwardingStatus_T ifs;
 ifs.vr_id = vr_id;

 if(NETCFG_OM_ROUTE_GetIpForwardingStatus(&ifs) == NETCFG_TYPE_FAIL)
  ifs.status_bitmap = 0;        // Initialize the bitamp since no entry for vr_id exists

 if(addr_type == L_INET_ADDR_TYPE_IPV4)
  UNSET_FLAG(ifs.status_bitmap, NETCFG_OM_ROUTE_FLAGS_IPV4);
 else if(addr_type == L_INET_ADDR_TYPE_IPV6)
  UNSET_FLAG(ifs.status_bitmap, NETCFG_OM_ROUTE_FLAGS_IPV6);

 if(L_SORT_LST_Set(ip_forwarding_status_list, &ifs))
  return NETCFG_TYPE_OK;
 return NETCFG_TYPE_FAIL;

}

/* FUNCTION NAME : NETCFG_OM_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding function status.
 *
 * INPUT:
 *      vr_id       : virtual router id
 *      address_type: IPv4/IPv6 to query
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_GetIpForwardingStatus(NETCFG_OM_ROUTE_IpForwardingStatus_T *ifs)
{

#if 0
 NETCFG_OM_ROUTE_IpForwardingStatus_T element;
 while(L_SORT_LST_Get_Next(ip_forwarding_status_list, &element) == TRUE)
 {
  if(element.vr_id == ifs->vr_id)
   {
    ifs->status_bitmap = element.status_bitmap;
  return NETCFG_TYPE_OK;
   }
 }
 return NETCFG_TYPE_FAIL;
#else
    if (L_SORT_LST_Get(ip_forwarding_status_list, ifs))
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;
#endif
}


/* ROUTINE NAME: NETCFG_OM_ROUTE_AddStaticIpCidrRoute
 *
 * FUNCTION: Add static route.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES: None.
 */
UI32_T NETCFG_OM_ROUTE_AddStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry)
{
    UI32_T rc = NETCFG_TYPE_CAN_NOT_ADD, orig_priority;
    L_PREFIX_T p;
    L_RADIX_Node_T *node;
    L_SORT_LST_List_T *nh_list;
    NETCFG_OM_ROUTE_NextHop_T nexthop;
    UI32_T mask, dst_ip;
    BOOL_T create_new = FALSE;

    /*  BODY
     */
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    /* Todo:
     * Vai, should check the endian (address order)
     */
    memset(&nexthop, 0, sizeof(nexthop));
    if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {
        L_PREFIX_MaskLen2Ip(entry->ip_cidr_route_dest.preflen, &mask);
        memcpy(&dst_ip, entry->ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);
        L_PREFIX_InetAddr2Prefix(dst_ip, mask, &p);
        L_PREFIX_ApplyMask(&p);

        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        /* Search or create a new node */
        if (L_RADIX_GetNode(&static_route_table, &p, &node) == TRUE)
        {
            /* A new created node without any nexthop */
            if (NULL == node->info)
            {
                /* Initialize the nexthop list */
                nh_list = (L_SORT_LST_List_T *)malloc(sizeof (L_SORT_LST_List_T));
                if (NULL == nh_list)
                {
                    L_RADIX_UnlockNode(node);
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
                    return rc;
                }
                memset(nh_list, 0, sizeof(L_SORT_LST_List_T));

                if (L_SORT_LST_Create(nh_list, SYS_ADPT_MAX_NBR_OF_HOST_ROUTE,
                            sizeof(NETCFG_OM_ROUTE_NextHop_T),
                            NETCFG_OM_ROUTE_CompareNextHops) == FALSE)
                {
                    free(nh_list);
                    L_RADIX_UnlockNode(node);
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
                    return rc;
                }
                node->info = nh_list;
                create_new = TRUE;
            }

            /* Append a new nexthop */
            NETCFG_OM_ROUTE_GetNextHopType(&entry->ip_cidr_route_next_hop,
                                            entry->ip_cidr_route_if_index,
                                            &nexthop.type);
            nexthop.distance = entry->ip_cidr_route_distance;
            nexthop.ifindex = entry->ip_cidr_route_if_index;
            //memcpy(&nexthop.inet_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN);
            nexthop.nexthop_addr = entry->ip_cidr_route_next_hop;
            nexthop.row_status = entry->ip_cidr_route_status;

            L_SORT_LST_Set(node->info, &nexthop);

            if (create_new == FALSE)
                L_RADIX_UnlockNode(node);

            rc = NETCFG_TYPE_OK;
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
        L_PREFIX_Inet6Addr2Prefix(entry->ip_cidr_route_dest.addr, entry->ip_cidr_route_dest.preflen, &p);
        L_PREFIX_ApplyMask(&p);

        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        /* Search or create a new node */
        if (L_RADIX_GetNode(&static_ipv6_route_table, &p, &node) == TRUE)
        {
            /* A new created node without any nexthop */
            if (NULL == node->info)
            {
                /* Initialize the nexthop list */
                nh_list = (L_SORT_LST_List_T *)malloc(sizeof (L_SORT_LST_List_T));
                if (NULL == nh_list)
                {
                    L_RADIX_UnlockNode(node);
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

                    return rc;
                }
                memset(nh_list, 0, sizeof(L_SORT_LST_List_T));

                if (L_SORT_LST_Create(nh_list, SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE,
                            sizeof(NETCFG_OM_ROUTE_NextHop_T),
                            NETCFG_OM_ROUTE_CompareIPV6NextHops) == FALSE)
                {
                    free(nh_list);
                    L_RADIX_UnlockNode(node);
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

                    return rc;
                }
                node->info = nh_list;
                create_new = TRUE;

            }

            /* Append a new nexthop */
            NETCFG_OM_ROUTE_GetNextHopType(&entry->ip_cidr_route_next_hop,
                                            entry->ip_cidr_route_if_index,
                                            &nexthop.type);
            nexthop.distance = entry->ip_cidr_route_distance;
            nexthop.ifindex= entry->ip_cidr_route_if_index;
            //memcpy(nexthop.inet6_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV6_ADDR_LEN);
            nexthop.nexthop_addr = entry->ip_cidr_route_next_hop;
            nexthop.row_status = entry->ip_cidr_route_status;
            L_SORT_LST_Set(node->info, &nexthop);

            if (create_new == FALSE)
                L_RADIX_UnlockNode(node);

            rc = NETCFG_TYPE_OK;

        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    }
#endif

    return rc;
}   /* end of NETCFG_OM_ROUTE_AddRoute */


/* ROUTINE NAME: NETCFG_OM_ROUTE_DeleteStaticIpCidrRoute
 *
 * FUNCTION:
 *      Delete route using 4 keys.
 *
 * INPUT:
 *      entry.ip_cidr_route_dest
 *      entry.ip_cidr_route_mask  - IP mask of static route
 *      entry.ip_cidr_route_metric
 *      entry.ip_cidr_route_next_hop
 *
 * OUTPUT: None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *
 * NOTES:
 */
UI32_T NETCFG_OM_ROUTE_DeleteStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry)
{
    UI32_T rc = NETCFG_TYPE_CAN_NOT_DELETE, orig_priority;
    L_PREFIX_T p;
    L_RADIX_Node_T *node;
    L_SORT_LST_List_T *nh_list;
    NETCFG_OM_ROUTE_NextHop_T nexthop;
    UI32_T dst_ip;
    UI32_T mask;
    /*  BODY
     */

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    /* Todo:
     * Vai, should check the endianess (address order)
     */
    if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {
        L_PREFIX_MaskLen2Ip(entry->ip_cidr_route_dest.preflen, &mask);
        memcpy(&dst_ip, entry->ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);
        L_PREFIX_InetAddr2Prefix(dst_ip, mask, &p);
        L_PREFIX_ApplyMask(&p);

        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        if (L_RADIX_LookupNode(&static_route_table, &p, &node) == TRUE)
        {
            nh_list = (L_SORT_LST_List_T *)node->info;
            if (NULL == nh_list)
            {
                L_RADIX_UnlockNode(node);
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
                return NETCFG_TYPE_ENTRY_NOT_EXIST;
            }
            nexthop.ifindex = entry->ip_cidr_route_if_index;
            nexthop.distance = entry->ip_cidr_route_distance;
            //memcpy(&nexthop.inet_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN);
            nexthop.nexthop_addr = entry->ip_cidr_route_next_hop;
            if (L_SORT_LST_Delete(nh_list, &nexthop) == TRUE)
            {
                rc = NETCFG_TYPE_OK;
            }

            /* All nexthop are deleted? */
            if (nh_list->nbr_of_element == 0)
            {
                free(nh_list);
                node->info = NULL;
                L_RADIX_UnlockNode(node);
            }

            L_RADIX_UnlockNode(node);
        }
        else
        {
            rc = NETCFG_TYPE_ENTRY_NOT_EXIST;
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);


    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
        L_PREFIX_Inet6Addr2Prefix(entry->ip_cidr_route_dest.addr, entry->ip_cidr_route_dest.preflen, &p);
    L_PREFIX_ApplyMask(&p);

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

    if (L_RADIX_LookupNode(&static_ipv6_route_table, &p, &node) == TRUE)
    {
        nh_list = (L_SORT_LST_List_T *)node->info;
        if (NULL == nh_list)
        {
            L_RADIX_UnlockNode(node);
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
            return NETCFG_TYPE_ENTRY_NOT_EXIST;
        }

        memset(&nexthop, 0, sizeof (NETCFG_OM_ROUTE_NextHop_T));
        nexthop.ifindex = entry->ip_cidr_route_if_index;
        nexthop.distance = entry->ip_cidr_route_distance;
        //        memcpy(nexthop.inet6_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV6_ADDR_LEN);
        nexthop.nexthop_addr = entry->ip_cidr_route_next_hop; // peter_yu
        NETCFG_OM_ROUTE_GetNextHopType(&entry->ip_cidr_route_next_hop,
                                        entry->ip_cidr_route_if_index,
                                        &nexthop.type);
        if (L_SORT_LST_Delete(nh_list, &nexthop) == TRUE)
        {
            rc = NETCFG_TYPE_OK;
        }

        /* All nexthop are deleted? */
        if (nh_list->nbr_of_element == 0)
        {
            free(nh_list);
            node->info = NULL;
            L_RADIX_UnlockNode(node);
        }

        L_RADIX_UnlockNode(node);
    }
    else
    {
        rc = NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);


    }
#endif

    return rc;
}


/* ROUTINE NAME: NETCFG_OM_ROUTE_DeleteAllStaticIpCidrRoutes
 *
 * FUNCTION:
 *      Delete all static routing configuration.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES: None.
 */
UI32_T NETCFG_OM_ROUTE_DeleteAllStaticIpCidrRoutes(UI32_T action_flags)
{
    UI32_T rc = NETCFG_TYPE_FAIL,orig_priority;
    L_RADIX_Node_T *node;
    L_RADIX_Node_T *next;
    L_SORT_LST_List_T *nh_list;
    /*  BODY
     */

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

    if(action_flags == L_INET_ADDR_TYPE_IPV4)
    {
        if (L_RADIX_Get_1st(&static_route_table, &node) == TRUE)
        {
            next = node;
            do
            {
                if (NULL != next->info)
                {
                    nh_list = (L_SORT_LST_List_T *)next->info;
                    /* Delete all nexthops */
                    L_SORT_LST_Delete_All(nh_list);
                    free(nh_list);
                    next->info = NULL;
                }

                /* Delete route node */
                node = next;
                L_RADIX_UnlockNode(next);

            } while (L_RADIX_GetNextNode(node, &next) == TRUE);
        }
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if(action_flags == L_INET_ADDR_TYPE_IPV6)
    {
        if (L_RADIX_Get_1st(&static_ipv6_route_table, &node) == TRUE)
        {
            next = node;
            do
            {
                if (NULL != next->info)
                {
                    nh_list = (L_SORT_LST_List_T *)next->info;
                    /* Delete all nexthops */
                    L_SORT_LST_Delete_All(nh_list);
                    free(nh_list);
                    next->info = NULL;
                }

                /* Delete route node */
                node = next;
                L_RADIX_UnlockNode(next);

            } while (L_RADIX_GetNextNode(node, &next) == TRUE);
        }

    }
#endif

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return rc;
}

/* ROUTINE NAME: NETCFG_OM_ROUTE_GetSameRoute
 *
 * FUNCTION:
 *      Get a route entry by keys.
 *      If a same route entry with different distance, delete nexthop
 *      if nh_replace is set to TRUE;
 *
 * INPUT:
 *      entry
 *      nh_replace
 *
 * OUTPUT:
 *      entry->ip_cidr_route_distance --- if the same route is found
 *                      with different distance and nh_replace is
 *                      set to FALSE.
 *
 * RETURN:
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      keys(entry.ip_cidr_route_dest, entry.ip_cidr_route_mask,
 *          entry.ip_cidr_route_next_hop).
 */
UI32_T NETCFG_OM_ROUTE_GetSameRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry, BOOL_T nh_replace)
{
    UI32_T rc = NETCFG_TYPE_ENTRY_NOT_EXIST, orig_priority;
    L_PREFIX_T p;
    L_RADIX_Node_T *node;
    L_SORT_LST_List_T *nh_list;
    NETCFG_OM_ROUTE_NextHop_T nexthop;
    UI32_T dest;
    UI32_T mask;
    UI32_T type;
    /*  BODY
     */

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {
        type = ROUTE_NH_TYPE_IP4;
        NETCFG_OM_ROUTE_GetNextHopType(&entry->ip_cidr_route_next_hop,
                                    entry->ip_cidr_route_if_index,
                                    &type);

        L_PREFIX_MaskLen2Ip(entry->ip_cidr_route_dest.preflen, &mask);
        memcpy(&dest, entry->ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);
        L_PREFIX_InetAddr2Prefix(dest, mask, &p);
        L_PREFIX_ApplyMask(&p);


        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        if (L_RADIX_LookupNode(&static_route_table, &p, &node) == TRUE)
        {
            nh_list = (L_SORT_LST_List_T *)node->info;
            if (NULL == nh_list)
            {
                L_RADIX_UnlockNode(node);
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
                return rc;
            }

            memset(&nexthop, 0, sizeof (NETCFG_OM_ROUTE_NextHop_T));

            while(L_SORT_LST_Get_Next(nh_list, &nexthop) == TRUE)
            {
                if (NETCFG_OM_ROUTE_IsSameNextHop(&nexthop,
                                                    &entry->ip_cidr_route_next_hop,
                                                type,
                                                entry->ip_cidr_route_if_index) == TRUE)
                {
                    if (entry->ip_cidr_route_distance == nexthop.distance)
                    {
                        rc = NETCFG_TYPE_ENTRY_EXIST;
                    }
                    else
                    {
                        if (nh_replace == TRUE)
                        {
                            L_SORT_LST_Delete(nh_list, &nexthop);
                        }
                        else
                        {
                            rc = NETCFG_TYPE_ENTRY_EXIST;
                            /* Same route exists with different distance value */
                            entry->ip_cidr_route_distance = nexthop.distance;
                        }
                    }

                    break;
                }
            }

            L_RADIX_UnlockNode(node);
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);


    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
        type = ROUTE_NH_TYPE_IP6;
        NETCFG_OM_ROUTE_GetNextHopType(&entry->ip_cidr_route_next_hop,
                                        entry->ip_cidr_route_if_index,
                                        &type);


        L_PREFIX_Inet6Addr2Prefix(entry->ip_cidr_route_dest.addr, entry->ip_cidr_route_dest.preflen, &p);
        L_PREFIX_ApplyMask(&p);

        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        if (L_RADIX_LookupNode(&static_ipv6_route_table, &p, &node) == TRUE)
        {
            nh_list = (L_SORT_LST_List_T *)node->info;
            if (NULL == nh_list)
            {
                L_RADIX_UnlockNode(node);
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
                return rc;
            }

            memset(&nexthop, 0, sizeof(NETCFG_OM_ROUTE_NextHop_T));

            while(L_SORT_LST_Get_Next(nh_list, &nexthop) == TRUE)
            {
                if (NETCFG_OM_ROUTE_IsSameNextHop(&nexthop,
                                                &entry->ip_cidr_route_next_hop,
                                                type,
                                                entry->ip_cidr_route_if_index) == TRUE)
                {
                    if (entry->ip_cidr_route_distance == nexthop.distance)
                    {
                        rc = NETCFG_TYPE_ENTRY_EXIST;
                    }
                    else
                    {
                        if (nh_replace == TRUE)
                        {
                            L_SORT_LST_Delete(nh_list, &nexthop);
                        }
                        else
                        {
                            rc = NETCFG_TYPE_ENTRY_EXIST;
                            /* Same route exists with different distance value */
                            entry->ip_cidr_route_distance = nexthop.distance;
                        }
                    }

                    break;
                }
            }

            L_RADIX_UnlockNode(node);
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);


    }
#endif

    return rc;
}   /*  end of ROUTECFG_OM_GetRoute  */

/* ROUTINE NAME: NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute
 *
 * FUNCTION:
 *      Get total number of nexthops of a route.
 *
 * INPUT:
 *      entry.
 *
 * OUTPUT:
 *      nh_nr.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry, UI32_T *nh_nr)
{
    UI32_T orig_priority;
    L_PREFIX_T p;
    L_RADIX_Node_T *node;
    L_SORT_LST_List_T *nh_list = NULL;

    *nh_nr = 0;

    if ((entry == NULL) || (nh_nr == NULL))
        return FALSE;

    if (L_INET_ADDR_TYPE_IPV4  == entry->ip_cidr_route_dest.type ||
        L_INET_ADDR_TYPE_IPV4Z == entry->ip_cidr_route_dest.type)
    {
        UI32_T dest, mask;

        memcpy(&dest, entry->ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);

        L_PREFIX_MaskLen2Ip(entry->ip_cidr_route_dest.preflen, &mask);
        L_PREFIX_InetAddr2Prefix(dest, mask, &p);
        L_PREFIX_ApplyMask(&p);

        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        if (L_RADIX_LookupNode(&static_route_table, &p, &node) == TRUE)
        {
            nh_list = (L_SORT_LST_List_T *)node->info;
            if (NULL != nh_list)
            {
                *nh_nr = nh_list->nbr_of_element;
            }

            L_RADIX_UnlockNode(node);
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
    }
    else if (L_INET_ADDR_TYPE_IPV6  == entry->ip_cidr_route_dest.type ||
             L_INET_ADDR_TYPE_IPV6Z == entry->ip_cidr_route_dest.type)
    {
        L_PREFIX_Inet6Addr2Prefix(entry->ip_cidr_route_dest.addr, entry->ip_cidr_route_dest.preflen, &p);
        L_PREFIX_ApplyMask(&p);

        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        if (L_RADIX_LookupNode(&static_ipv6_route_table, &p, &node) == TRUE)
        {
            nh_list = (L_SORT_LST_List_T *)node->info;
            if (NULL != nh_list)
            {
                *nh_nr = nh_list->nbr_of_element;
            }

            L_RADIX_UnlockNode(node);
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
    }

    return TRUE;
}



/* ROUTINE NAME: NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute
 *
 * FUNCTION:
 *      Get next route using 4 keys.
 *
 * INPUT:
 *      entry.
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      key(entry->ip_cidr_route_dest,
 *             entry->ip_cidr_route_mask,
 *             entry->ip_cidr_route_next_hop,
 *             entry->ip_cidr_route_distance)
 */
UI32_T NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry)
{
    UI32_T rc = NETCFG_TYPE_ENTRY_NOT_EXIST, orig_priority;
    L_PREFIX_T p;
    L_RADIX_Node_T *node;
    L_RADIX_Node_T *best = NULL;
    L_SORT_LST_List_T *nh_list;
    NETCFG_OM_ROUTE_NextHop_T nexthop;
    UI32_T dest, dest_ip, next_hop;
    UI32_T mask;
    BOOL_T no_key = FALSE;
    int cmp_result = 0;

    /*  BODY
     */

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {

        L_PREFIX_MaskLen2Ip(entry->ip_cidr_route_dest.preflen, &mask);
        memcpy(&dest_ip, entry->ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);
        L_PREFIX_InetAddr2Prefix(dest_ip, mask, &p);
        L_PREFIX_ApplyMask(&p);
        IP_LIB_ArraytoUI32(entry->ip_cidr_route_dest.addr, &dest);

        memset(&nexthop, 0, sizeof (NETCFG_OM_ROUTE_NextHop_T));
        nexthop.distance = entry->ip_cidr_route_distance;
        nexthop.ifindex = entry->ip_cidr_route_if_index;
        //memcpy(&nexthop.inet_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN );
        nexthop.nexthop_addr = entry->ip_cidr_route_next_hop;

        IP_LIB_ArraytoUI32(entry->ip_cidr_route_next_hop.addr, &next_hop);


        if (dest == 0 && mask == 0 && next_hop == 0 &&
            nexthop.distance == 0)
        {
            no_key = TRUE;
        }

        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        for (node = L_RADIX_GetTableTop(&static_route_table); node != NULL;
                node = L_RADIX_GetNext(node))
        {
            cmp_result = memcmp(&p.u.prefix, &node->p.u.prefix, SYS_ADPT_IPV4_ADDR_LEN);
            if (cmp_result > 0)
                continue;

            if (node->info != NULL)
            {
                nh_list = (L_SORT_LST_List_T *)node->info;
                if (L_SORT_LST_Get_1st(nh_list, &nexthop) == TRUE)
                {
                    /* Get 1st */
                    if ((no_key == TRUE) || (cmp_result < 0))
                    {
                        best = node;
                        break;
                    }

                    /* Get the Next nexthop */
                    if (cmp_result == 0)
                    {
                        do
                        {
                            //if (memcmp(&nexthop.inet_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN) <= 0)
                            //if (memcmp(&(nexthop.nexthop_addr), &(entry->ip_cidr_route_next_hop), sizeof(L_INET_AddrIp_T)) <= 0)
                            /* compare only L_INET_AddrIp_T.addr part */
                            if (memcmp(nexthop.nexthop_addr.addr, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN) <= 0)
                            {
                                continue;
                            }
                            else
                            {
                                best = node;
                                goto end;
                            }
                        } while (L_SORT_LST_Get_Next(nh_list, &nexthop) == TRUE);
                    }
                }
            }
        }

end:
        if (best != NULL)
        {
            memcpy(entry->ip_cidr_route_dest.addr, &(best->p.u.prefix), SYS_ADPT_IPV4_ADDR_LEN);
            entry->ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
            entry->ip_cidr_route_dest.preflen = best->p.prefixlen;
            entry->ip_cidr_route_distance = nexthop.distance;
            entry->ip_cidr_route_if_index = nexthop.ifindex;
            entry->ip_cidr_route_status = nexthop.row_status;
            //memcpy(entry->ip_cidr_route_next_hop.addr, &nexthop.inet_gateway, SYS_ADPT_IPV4_ADDR_LEN);
            //entry->ip_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;
            entry->ip_cidr_route_next_hop = nexthop.nexthop_addr;

            L_RADIX_UnlockNode(best);
            rc = NETCFG_TYPE_OK;
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
        L_PREFIX_Inet6Addr2Prefix(entry->ip_cidr_route_dest.addr, entry->ip_cidr_route_dest.preflen, &p);
        L_PREFIX_ApplyMask(&p);

        memset(&nexthop, 0, sizeof (NETCFG_OM_ROUTE_NextHop_T));
        nexthop.distance = entry->ip_cidr_route_distance;
        nexthop.ifindex = entry->ip_cidr_route_if_index;
        //memcpy(nexthop.inet6_gateway, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV6_ADDR_LEN);
        nexthop.nexthop_addr = entry->ip_cidr_route_next_hop;

        //if (entry->ip_cidr_route_dest.addr[0] == 0 && nexthop.nexthop_addr.addr[0] == 0 && nexthop.distance == 0)
        if (IP_LIB_IsIPv6UnspecifiedAddr(entry->ip_cidr_route_dest.addr)
            && IP_LIB_IsIPv6UnspecifiedAddr(nexthop.nexthop_addr.addr)
            && (nexthop.distance == 0))
        {
            no_key = TRUE;
        }

        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

        for (node = L_RADIX_GetTableTop(&static_ipv6_route_table); node != NULL;
                node = L_RADIX_GetNext(node))
        {
            cmp_result = memcmp(&p.u.prefix, &node->p.u.prefix, SYS_ADPT_IPV6_ADDR_LEN);
            if (cmp_result > 0)
                continue;

            if (node->info != NULL)
            {
                nh_list = (L_SORT_LST_List_T *)node->info;
                if (L_SORT_LST_Get_1st(nh_list, &nexthop) == TRUE)
                {
                    /* Get 1st */
                    if ((no_key == TRUE) || (cmp_result < 0))
                    {
                        best = node;
                        break;
                    }

                    /* Get the Next nexthop */
                    if (cmp_result == 0)
                    {
                        do
                        {
                            //if (memcmp(&(nexthop.nexthop_addr), &(entry->ip_cidr_route_next_hop), sizeof(L_INET_AddrIp_T)) <= 0)
                            /* compare only L_INET_AddrIp_T.addr part */
                            if (memcmp(nexthop.nexthop_addr.addr, entry->ip_cidr_route_next_hop.addr, SYS_ADPT_IPV6_ADDR_LEN) <= 0)
                            {
                                continue;
                            }
                            else
                            {
                                best = node;
                                goto end6;
                            }
                        } while (L_SORT_LST_Get_Next(nh_list, &nexthop) == TRUE);
                    }
                }
            }
        }

end6:
        if (best != NULL)
        {
            memcpy(entry->ip_cidr_route_dest.addr, &(best->p.u.prefix), SYS_ADPT_IPV6_ADDR_LEN);
            //L_PREFIX_MaskLen2Ip(best->p.prefixlen, (UI32_T *)entry->ip_cidr_route_mask);
            entry->ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
            entry->ip_cidr_route_dest.preflen = best->p.prefixlen;
            entry->ip_cidr_route_distance = nexthop.distance;
            entry->ip_cidr_route_if_index = nexthop.ifindex;
            entry->ip_cidr_route_status = nexthop.row_status;
            //memcpy(entry->ip_cidr_route_next_hop.addr, nexthop.inet6_gateway, SYS_ADPT_IPV6_ADDR_LEN);
            //entry->ip_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
            entry->ip_cidr_route_next_hop = nexthop.nexthop_addr;

            if(nexthop.type ==  ROUTE_NH_TYPE_IFINDEX)
            {
               entry->ip_cidr_route_type = VAL_ipCidrRouteType_local;
            }

            L_RADIX_UnlockNode(best);
            rc = NETCFG_TYPE_OK;
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
    }
#endif
    return rc;
}   /*  end of ROUTECFG_OM_GetRoute  */


/* ROUTINE NAME: NETCFG_OM_ROUTE_IsRouteExisted
 *
 * FUNCTION:
 *      Get route using 4 keys.
 *
 * INPUT:
 *      entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      key(entry.ip_cidr_route_dest, entry.ip_cidr_route_mask,
 *          entry.ip_cidr_route_metric,  entry.ip_cidr_route_next_hop).
 */
UI32_T NETCFG_OM_ROUTE_IsRouteExisted(ROUTE_MGR_IpCidrRouteEntry_T *entry)
{
    UI32_T rc = NETCFG_TYPE_ENTRY_NOT_EXIST, orig_priority;
    L_PREFIX_T p;
    L_RADIX_Node_T *node;
    UI32_T dest;
    UI32_T mask;
    UI32_T type;
    /*  BODY
     */

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {
    type = ROUTE_NH_TYPE_IP4;
        NETCFG_OM_ROUTE_GetNextHopType(&entry->ip_cidr_route_next_hop,
                                    entry->ip_cidr_route_if_index,
                                    &type);

    /* Todo:
     * Vai, should check the endianess (address order)
     */
        L_PREFIX_MaskLen2Ip(entry->ip_cidr_route_dest.preflen, &mask);
        memcpy(&dest, entry->ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);
    L_PREFIX_InetAddr2Prefix(dest, mask, &p);
    L_PREFIX_ApplyMask(&p);


    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

    if (L_RADIX_LookupNode(&static_route_table, &p, &node) == TRUE)
    {
        /* TODO:
         * vai
         * add nexthop and distance searching
         */
        L_RADIX_UnlockNode(node);
        rc = NETCFG_TYPE_ENTRY_EXIST;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);


    }
    else if(entry->ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
    type = ROUTE_NH_TYPE_IP6;

    }
    return rc;
}   /*  end of ROUTECFG_OM_IsRouteExisted  */


static int NETCFG_OM_ROUTE_CompareNextHops(void *elm1, void *elm2)
{
    NETCFG_OM_ROUTE_NextHop_T *v1, *v2;
    //UI32_T val1, val2;
    int ret;

    v1 = (NETCFG_OM_ROUTE_NextHop_T*)elm1;
    v2 = (NETCFG_OM_ROUTE_NextHop_T*)elm2;

    if ((v1->type != ROUTE_NH_TYPE_IFINDEX) &&
        (v2->type != ROUTE_NH_TYPE_IFINDEX))
    {
        ret = memcmp(&(v1->nexthop_addr), &(v2->nexthop_addr), sizeof(L_INET_AddrIp_T));

    }
    /* IFINDEX */
    else
    {
        ret = v1->ifindex - v2->ifindex;
    }

    if (ret != 0)
        return ret;

    /* Administrative distance is the secondary KEY */
    return (v1->distance - v2->distance);
}

static int NETCFG_OM_ROUTE_CompareIpForwardingStatus(void *elm1, void *elm2)
{
    NETCFG_OM_ROUTE_IpForwardingStatus_T *v1, *v2;
    UI32_T val1, val2;

    v1 = (NETCFG_OM_ROUTE_IpForwardingStatus_T *)elm1;
    v2 = (NETCFG_OM_ROUTE_IpForwardingStatus_T *)elm2;

    val1 = v1->vr_id;
    val2 = v2->vr_id;

   if (val1 < val2)
    {
        return -1;
    }
    else if (val1 > val2)
    {
        return 1;
    }

    return 0;
}



#if (SYS_CPNT_IPV6 == TRUE)
static int NETCFG_OM_ROUTE_CompareIPV6NextHops(void *elm1, void *elm2)
{
    NETCFG_OM_ROUTE_NextHop_T *v1, *v2;
    int ret;

    v1 = (NETCFG_OM_ROUTE_NextHop_T*)elm1;
    v2 = (NETCFG_OM_ROUTE_NextHop_T*)elm2;

    if ((v1->type != ROUTE_NH_TYPE_IFINDEX) &&
        (v2->type != ROUTE_NH_TYPE_IFINDEX))
    {
        //ret = memcmp(v1->inet6_gateway, v2->inet6_gateway, SYS_ADPT_IPV6_ADDR_LEN);
        ret = memcmp(&(v1->nexthop_addr), &(v2->nexthop_addr), sizeof(L_INET_AddrIp_T));
    }
    else /* IFINDEX */
    {
        ret = v1->ifindex - v2->ifindex;
    }

    if (ret != 0)
        return ret;

    /* Administrative distance is the secondary KEY
     */
    return (v1->distance - v2->distance);
}
#endif

/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_IsSameNextHop
 *
 * FUNCTION: Check if the input gateway IP or ifname is the same as
 *           those in nh (NETCFG_OM_ROUTE_NextHop_T)
 *
 * INPUT:
 *      nh
 *      gateway -- input gateway IP
 *      type    -- the type of nexthop
 *      ifindex -- interface index of nexthop (for future use)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
static BOOL_T NETCFG_OM_ROUTE_IsSameNextHop (NETCFG_OM_ROUTE_NextHop_T *nh, L_INET_AddrIp_T *gateway,
                                        UI32_T type, UI32_T ifindex)
{
    if (nh->type != type)
        return FALSE;

    if (0 != ifindex)
    {
        if (ifindex != nh->ifindex)
            return FALSE;
    }

    if (NULL != gateway)
    {
        if (0 != L_INET_CompareInetAddr((L_INET_Addr_T *) &nh->nexthop_addr,
            (L_INET_Addr_T *) gateway, 0))
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_GetNextHopType
 *
 * FUNCTION: Get the type of nexthop
 *
 * INPUT:
 *      gateway -- input gateway IP
 *      ifindex  -- interface index of nexthop (for future use)
 *
 * OUTPUT:
 *      type    -- the type of nexthop
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
static BOOL_T NETCFG_OM_ROUTE_GetNextHopType (L_INET_AddrIp_T *gateway, UI32_T ifindex, UI32_T *type)
{
    UI8_T zero_addr4[SYS_ADPT_IPV4_ADDR_LEN] = {};

    if (NULL == type)
        return FALSE;
    switch(gateway->type)
    {
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
            if (memcmp(gateway->addr, zero_addr4, SYS_ADPT_IPV4_ADDR_LEN) && (0 != ifindex))
                *type = ROUTE_NH_TYPE_IP4_IFINDEX;
            else if (memcmp(gateway->addr, zero_addr4, SYS_ADPT_IPV4_ADDR_LEN))
                *type = ROUTE_NH_TYPE_IP4;
            else if (0 != ifindex)
                *type = ROUTE_NH_TYPE_IFINDEX;
            break;

        case L_INET_ADDR_TYPE_IPV6 :
        case L_INET_ADDR_TYPE_IPV6Z :
            if (!IP_LIB_IsIPv6UnspecifiedAddr(gateway->addr)&& (0 != ifindex))
                *type = ROUTE_NH_TYPE_IP6_IFINDEX;
            else if (!IP_LIB_IsIPv6UnspecifiedAddr(gateway->addr))
                *type = ROUTE_NH_TYPE_IP6;
            else if (0 != ifindex)
                *type = ROUTE_NH_TYPE_IFINDEX;
            break;
        default:
            break;
    }
    return TRUE;
}


#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
/*
 * ROUTINE NAME: ROUTECFG_SetManagementVlanDefaultGateway
 *
 * FUNCTION: Set default gateway for management vlan, only one gateway address can be set in system
 *      This gateway is only for remote management packet
 *
 * INPUT:
 *      mgmt_default_gateway  : next hop of default route.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES:
 *      1. The management vlan function is for L3 product. Only L3 product should call this function.
 *      2. Currently, just support one default gateway.
 */
UI32_T ROUTECFG_OM_SetManagementVlanDefaultGateway(UI8_T mgmt_default_gateway[SYS_ADPT_IPV4_ADDR_LEN])
{
    UI32_T orig_priority;

    /*  BODY
     */
    if(mgmt_default_gateway == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);
    memcpy(management_vlan_default_gateway, mgmt_default_gateway, sizeof(management_vlan_default_gateway));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}   /* end of ROUTECFG_SetManagementVlanDefaultGateway */


/*
 * ROUTINE NAME: ROUTECFG_OM_GetManagementVlanDefaultGateway
 *
 * FUNCTION: Get default gateway address for management vlan.
 *
 * INPUT: None.
 *
 * OUTPUT: mgmt_default_gateway_p - pointer to store default gateway for management vlan
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_EXIST : gateway address do not exist
 *      NETCFG_TYPE_INVALID_ARG : Invalid argument
 *
 * NOTES:
 *      1. The management vlan function is for L3 product. Only L3 product should call this function.
 *      2. Currently, just support one default gateway.
 */
UI32_T ROUTECFG_OM_GetManagementVlanDefaultGateway(UI8_T mgmt_default_gateway_p[SYS_ADPT_IPV4_ADDR_LEN])
{
    UI32_T rc = NETCFG_TYPE_NOT_EXIST, orig_priority;

    /*  BODY
     */
    if(mgmt_default_gateway_p == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);
    memcpy(mgmt_default_gateway_p, management_vlan_default_gateway, SYS_ADPT_IPV4_ADDR_LEN);
    if(!ROUTECFG_IP_IS_ALL_ZEROS(management_vlan_default_gateway))
    {
        rc = NETCFG_TYPE_OK;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
    return rc;
}   /* end of ROUTECFG_OM_GetManagementVlanDefaultGateway */

/* ROUTINE NAME: ROUTECFG_OM_DeleteManagementVlanDefaultGateway
 *
 * FUNCTION: Delete default gateway for management vlan.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *
 *
 * NOTES:
 *      1. The management vlan function is for L3 product. Only L3 product should call this function.
 *      2. Currently, just support one default gateway.
 */
UI32_T ROUTECFG_OM_DeleteManagementVlanDefaultGateway(void)
{
    UI32_T orig_priority;

    if(ROUTECFG_IP_IS_ALL_ZEROS(management_vlan_default_gateway))
    {
        return NETCFG_TYPE_OK;
    }

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);
    memset(management_vlan_default_gateway, 0, sizeof(management_vlan_default_gateway));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}   /* end of ROUTECFG_OM_DeleteManagementVlanDefaultGateway */

#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_GetStaticIpCidrRouteNumber
 *
 * FUNCTION: Get number of static route.
 *
 * INPUT: None.
 *
 * OUTPUT: static_route_num - Number of static route.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES: None.
 */
BOOL_T NETCFG_OM_ROUTE_GetStaticIpCidrRouteNumber(UI32_T action_flags, UI32_T *static_route_num)
{
    UI32_T orig_priority;

    if(static_route_num == NULL)
        return FALSE;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);
    if(action_flags == L_INET_ADDR_TYPE_IPV4)
    *static_route_num=netcfg_route_static_route_number;
#if (SYS_CPNT_IPV6 == TRUE)
    else if(action_flags == L_INET_ADDR_TYPE_IPV6)
        *static_route_num=netcfg_route_static_ipv6_route_number;
#endif
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return TRUE;
}

/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_UpdateStaticIpCidrRouteNumber
 *
 * FUNCTION: Update the number of static route based on the current count.
 *
 * INPUT: changed_count  -- The changed count that will apply to the current
 *                          number of static route. Positive value will increase
 *                          the count and negative value will decrease the count.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES: 1. This is private om api that will only called by functions in
 *           routecfg.c
 */
void NETCFG_OM_ROUTE_UpdateStaticIpCidrRouteNumber(UI32_T action_flags, I32_T changed_count)
{
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);
    if(action_flags == L_INET_ADDR_TYPE_IPV4)
    netcfg_route_static_route_number = (I32_T)netcfg_route_static_route_number + changed_count;
#if (SYS_CPNT_IPV6 == TRUE)
    else if(action_flags == L_INET_ADDR_TYPE_IPV6)
        netcfg_route_static_ipv6_route_number = (I32_T)netcfg_route_static_ipv6_route_number + changed_count;
#endif

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return;

}

/*
 * ROUTINE NAME: NETCFG_OM_ROUTE_SetStaticIpCidrRouteNumber
 *
 * FUNCTION: Set the number of static route.
 *
 * INPUT: count  -- The count to be set.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:
 */
void NETCFG_OM_ROUTE_SetStaticIpCidrRouteNumber(UI32_T action_flags, UI32_T count)
{
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);
    if(action_flags == L_INET_ADDR_TYPE_IPV4)
    netcfg_route_static_route_number = count;
#if (SYS_CPNT_IPV6 == TRUE)
    else if(action_flags == L_INET_ADDR_TYPE_IPV6)
        netcfg_route_static_ipv6_route_number = count;
#endif
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return;
}

/* MACRO FUNCTION DECLARATIONS
 */
#define NETCFG_OM_ROUTE_IP_IS_ALL_ZEROS(ip_addr) \
    ((ip_addr[0]==0) && \
     (ip_addr[1]==0) && \
     (ip_addr[2]==0) && \
     (ip_addr[3]==0))

#define NETCFG_OM_ROUTE_IP6_IS_ALL_ZEROS(ip_addr) \
    ((ip_addr[0]==0) && \
     (ip_addr[1]==0) && \
     (ip_addr[2]==0) && \
     (ip_addr[3]==0) && \
     (ip_addr[4]==0) && \
     (ip_addr[5]==0) && \
     (ip_addr[6]==0) && \
     (ip_addr[7]==0) && \
     (ip_addr[8]==0) && \
     (ip_addr[9]==0) && \
     (ip_addr[10]==0) && \
     (ip_addr[11]==0) && \
     (ip_addr[12]==0) && \
     (ip_addr[13]==0) && \
     (ip_addr[14]==0) && \
     (ip_addr[15]==0))




BOOL_T  ROUTECFG_OM_SetDefaultGatewayCompatible(L_INET_AddrIp_T *gateway)
{
    if(gateway->type == L_INET_ADDR_TYPE_IPV4 || gateway->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE] = *gateway;
        netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE].addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    }
    else
    {
        netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE] = *gateway;
        netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE].addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    }
    return TRUE;
}
BOOL_T  ROUTECFG_OM_SetDhcpDefaultGateway(L_INET_AddrIp_T *gateway)
{
    if(gateway->type == L_INET_ADDR_TYPE_IPV4 || gateway->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP] = *gateway;
        netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP].addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    }
    else
    {
        netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP] = *gateway;
        netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP].addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    }

    return TRUE;
}

BOOL_T  ROUTECFG_OM_GetDefaultGatewayCompatible(L_INET_AddrIp_T *gateway)
{
    if(gateway->type == L_INET_ADDR_TYPE_IPV4 || gateway->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        if(NETCFG_OM_ROUTE_IP_IS_ALL_ZEROS(netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE].addr))
            return FALSE;
        else
            *gateway=netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE];
    }
    else
    {
        if(NETCFG_OM_ROUTE_IP6_IS_ALL_ZEROS(netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE].addr))
            return FALSE;
        else
            *gateway=netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE];
    }
    return TRUE;
}

BOOL_T  ROUTECFG_OM_GetDhcpDefaultGateway(L_INET_AddrIp_T *gateway)
{

    if(gateway->type == L_INET_ADDR_TYPE_IPV4 || gateway->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        if(NETCFG_OM_ROUTE_IP_IS_ALL_ZEROS(netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP].addr))
            return FALSE;
        else
            *gateway=netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP];
    }
    else
    {
        if(NETCFG_OM_ROUTE_IP6_IS_ALL_ZEROS(netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP].addr))
            return FALSE;
        else
            *gateway=netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP];
    }
    return TRUE;
}

BOOL_T  ROUTECFG_OM_DeleteDefaultGatewayCompatible()
{
    memset(&netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE], 0, sizeof(L_INET_AddrIp_T));
    return TRUE;
}

BOOL_T  ROUTECFG_OM_DeleteIpv6DefaultGatewayCompatible()
{
    memset(&netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_COMPATIBLE], 0, sizeof(L_INET_AddrIp_T));
    return TRUE;
}

BOOL_T  ROUTECFG_OM_DeleteDhcpDefaultGateway()
{
    memset(&netcfg_route_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP], 0, sizeof(L_INET_AddrIp_T));
    return TRUE;
}

BOOL_T  ROUTECFG_OM_DeleteIpv6DhcpDefaultGateway()
{
    memset(&netcfg_route_ipv6_default_gateway_compatable[NETCFG_ROUTE_DEFAULT_GATEWAY_TYPE_DHCP], 0, sizeof(L_INET_AddrIp_T));
    return TRUE;
}

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_OM_ROUTE_GetRunningEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get running ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_OM_ROUTE_GetRunningEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    if (NETCFG_TYPE_OK != NETCFG_OM_ROUTE_GetEcmpBalanceMode(mode_p, idx_p))
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        if (SYS_DFLT_ECMP_BALANCE_MODE != *mode_p)
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_OM_ROUTE_GetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_OM_ROUTE_GetEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p)
{
    UI32_T orig_priority, ret = NETCFG_TYPE_OK;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

    *mode_p = netcfg_route_ecmp_lbmode;
    *idx_p =  netcfg_route_ecmp_hsl_id;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_OM_ROUTE_SetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode.
 * INPUT   : mode - which mode to set
 *           idx  - which idx to set if mode is NETCFG_TYPE_ECMP_HASH_SELECTION
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
void NETCFG_OM_ROUTE_SetEcmpBalanceMode(UI32_T mode, UI32_T idx)
{
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(netcfg_om_route_sem_id);

    netcfg_route_ecmp_lbmode = mode;

    netcfg_route_ecmp_hsl_id =
        (NETCFG_TYPE_ECMP_HASH_SELECTION == mode) ? idx : 0;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(netcfg_om_route_sem_id, orig_priority);
}
#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

