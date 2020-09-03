/* Module Name:NETCFG_OM_IP.C
 * Purpose: To store configuration ip interface entry.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       01/22/2008 --  Vai Wang    Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "l_cvrt.h"
#include "l_sort_lst.h"
#include "netcfg_om_ip.h"
#include "ip_lib.h"
#include "eh_mgr.h"
#include "eh_type.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "vlan_type.h"

/* NAME CONSTANT DECLARATIONS
 */
#define NETCFG_OM_IP_MAX_RIF_NBR            SYS_ADPT_MAX_NBR_OF_RIF     /*  Max RIF number */
/*  max vlan and routing table in this system */
#define NETCFG_OM_IP_MAX_INTERFACE_NBR      SYS_ADPT_MAX_NBR_OF_L3_INTERFACE

const static UI8_T ipv4_zero_addr[SYS_ADPT_IPV4_ADDR_LEN] = {0};
const static UI8_T ipv6_zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {0};

/* MACRO FUNCTION DECLARATIONS
 */
/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
#define DEBUG_FLAG_BIT_TUNNEL 0x08
/***************************************************************/
static unsigned long DEBUG_FLAG = 0;// DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define TUNNELprintf(format,args...)((DEBUG_FLAG_BIT_TUNNEL & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

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
typedef struct NETCFG_OM_IP_Shmem_Data_S
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    L_SORT_LST_ShMem_List_T rif_list;
}NETCFG_OM_IP_Shmem_Data_T;

typedef struct NETCFG_OM_RifSearchInfo_S
{

    L_INET_AddrIp_T              ipaddr;
    NETCFG_OM_IP_InetRifConfig_T result;
    UI32_T                       ifindex;
    BOOL_T                       found;
    BOOL_T                       chk_ifidx;
} NETCFG_OM_RifSearchInfo_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static int NETCFG_OM_IP_CompareInterface(void* rec1, void* rec2);
static UI32_T NETCFG_OM_IP_CompareSubnet(const void *rec, void *cookie);
static UI32_T NETCFG_OM_IP_CompareSubnetAndIfindex(const void *rec, void *cookie);
static UI32_T NETCFG_OM_IP_GetIpAddressMode(UI32_T ifindex, UI32_T *mode_p);
static UI32_T NETCFG_OM_IP_GetNextIpAddressMode(UI32_T *ifindex, UI32_T *mode_p);
static UI32_T NETCFG_OM_IP_InternalLookupPrimaryRif(NETCFG_OM_IP_InetRifConfig_T *primary_rif, UI32_T ifindex);
static UI32_T NETCFG_OM_IP_InternalLookupNextSecondaryRif(NETCFG_OM_IP_InetRifConfig_T *primary_rif, UI32_T ifindex);
static UI32_T NETCFG_OM_IP_AttachInetRifToInterface(NETCFG_OM_L3_Interface_T *intf, NETCFG_OM_IP_InetRifConfig_T *rif);
static UI32_T NETCFG_OM_IP_DetachInetRifFromInterface(NETCFG_OM_L3_Interface_T *intf_p, NETCFG_OM_IP_InetRifConfig_T *rif_p);

static UI32_T NETCFG_OM_IP_GetInterfaceInetRif(NETCFG_OM_L3_Interface_T *intf,
                                    NETCFG_OM_IP_InetRifConfig_T *rif, BOOL_T get_next);
static UI32_T NETCFG_OM_IP_GetPrimaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
static UI32_T NETCFG_OM_IP_GetNextSecondaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

static UI32_T NETCFG_OM_IP_GetIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p);
static UI32_T NETCFG_OM_IP_GetNextIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p);

/* STATIC VARIABLE DECLARATIONS
 */
static NETCFG_OM_IP_Shmem_Data_T *shmem_data_p;
/* Store all L3 interface information in a sorted list, use
 * ifindex as the soring and searching KEY
 */
static L_SORT_LST_List_T intf_list;
/* Routing information, use ip address as the sorting
 * and searching KEY
 */
#define rif_list        (shmem_data_p->rif_list)

static UI32_T ip_om_sem_id;

/*static UI8_T ip_om_debug_flag = 0; *//* for debug */

#if (SYS_CPNT_IPV6 == TRUE)
static IPAL_Ipv6Statistics_T    ip6stat_base;
static IPAL_Icmpv6Statistics_T  icmp6stat_base;
static IPAL_Udpv6Statistics_T   udp6stat_base;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
static NETCFG_OM_Craft_Interface_T craft_interface;

#endif /* SYS_CPNT_CRAFT_PORT */

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
void NETCFG_OM_IP_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_NETCFG_OM_IP_SHMEM_SEGID;
    *seglen_p = sizeof(NETCFG_OM_IP_Shmem_Data_T)  +
                            L_SORT_LST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(NETCFG_OM_IP_MAX_RIF_NBR,
                                        sizeof(NETCFG_OM_IP_InetRifConfig_T));

    return;
}


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
void NETCFG_OM_IP_InitateSystemResources(void)
{
    shmem_data_p = (NETCFG_OM_IP_Shmem_Data_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_NETCFG_OM_IP_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &ip_om_sem_id);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    return;
}


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
void NETCFG_OM_IP_AttachSystemResources(void)
{
    shmem_data_p = (NETCFG_OM_IP_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_NETCFG_OM_IP_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &ip_om_sem_id);

    return;
}


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
void NETCFG_OM_IP_InitateProcessResources(void)
{
    if (L_SORT_LST_Create (&intf_list,
                           NETCFG_OM_IP_MAX_INTERFACE_NBR,
                           sizeof(NETCFG_OM_L3_Interface_T),
                           NETCFG_OM_IP_CompareInterface)==FALSE)
    {
        /* DEBUG */
        //printf("NETCFG_OM_IP_InitateProcessResources -- create l3 interface table failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_IP_Init : Can't create IP interface List.\n");
    }

    if (L_SORT_LST_ShMem_Create (&rif_list,
                           L_CVRT_GET_PTR(shmem_data_p, sizeof(NETCFG_OM_IP_Shmem_Data_T)),
                           NETCFG_OM_IP_MAX_RIF_NBR,
                           sizeof(NETCFG_OM_IP_InetRifConfig_T),
                           L_SORT_LST_SHMEM_COMPARE_FUNC_ID_NETCFG_OM_IP)==FALSE)
    {
        /* DEBUG */
        //printf("NETCFG_OM_IP_InitateProcessResources -- create routing information table failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_IP_Init : Can't create RIF List.\n");
    }

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &ip_om_sem_id) != SYSFUN_OK)
    {
        DBG_PrintText("NETCFG_OM_IP_Init: can't get semaphore \n");
        while (TRUE);
    }
    /* init craft_interface */
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    memset(&craft_interface, 0, sizeof craft_interface);
#endif


}


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
BOOL_T NETCFG_OM_IP_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_OM_IP_IPCMsg_T *netcfg_om_msg_p;
    BOOL_T           need_respond;

    if(ipcmsg_p==NULL)
        return FALSE;

    netcfg_om_msg_p= (NETCFG_OM_IP_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(netcfg_om_msg_p->type.cmd)
    {
        case NETCFG_OM_IP_IPCCMD_GETL3INTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetL3Interface(
                &(netcfg_om_msg_p->data.l3_interface_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(l3_interface_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTL3INTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextL3Interface(
                &(netcfg_om_msg_p->data.l3_interface_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(l3_interface_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETIPADDRESSMODE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIpAddressMode(
                netcfg_om_msg_p->data.u32a1_u32a2.u32_a1,
                &(netcfg_om_msg_p->data.u32a1_u32a2.u32_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_u32a2);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTIPADDRESSMODE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextIpAddressMode(
                &(netcfg_om_msg_p->data.u32a1_u32a2.u32_a1),
                &(netcfg_om_msg_p->data.u32a1_u32a2.u32_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_u32a2);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETRIFCONFIG:
            /* v4 only, call NETCFG_OM_IP_GetIPv4RifConfig */
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv4RifConfig(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTRIFCONFIG:
            /* v4 only, call NETCFG_OM_IP_GetNextIPv4RifConfig */
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextIPv4RifConfig(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;
#if 0 // peter, no caller
        case NETCFG_OM_IP_IPCCMD_GETINETRIFCONFIG:
            /* v4/v6 */
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetInetRifConfig(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTINETRIFCONFIG:
            /* v4/v6 */
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextInetRifConfig(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;
#endif
        case NETCFG_OM_IP_IPCCMD_GETRIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetRifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETPRIMARYRIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetPrimaryRifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTSECONDARYRIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextSecondaryRifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTRIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextRifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTINETRIFOFINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextInetRifOfInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETRIFFROMIP:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetRifFromIp(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;
        case NETCFG_OM_IP_IPCCMD_GETRIFFROMEXACTIP:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetRifFromExactIp(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETIPADDRENTRY:
            netcfg_om_msg_p->type.result_bool = NETCFG_OM_IP_GetIpAddrEntry(
                &(netcfg_om_msg_p->data.ip_addr_entry_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(ip_addr_entry_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTIPADDRENTRY:
            netcfg_om_msg_p->type.result_bool = NETCFG_OM_IP_GetNextIpAddrEntry(
                &(netcfg_om_msg_p->data.ip_addr_entry_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(ip_addr_entry_v);
            need_respond = TRUE;
            break;

#if (SYS_CPNT_PROXY_ARP == TRUE)
        case NETCFG_OM_IP_IPCCMD_GETARPPROXYSTATUS:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIpNetToMediaProxyStatus(
                netcfg_om_msg_p->data.u32a1_bla2.u32_a1,
                &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTARPPROXYSTATUS:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextIpNetToMediaProxyStatus(
                &(netcfg_om_msg_p->data.u32a1_bla2.u32_a1),
                &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)
        case NETCFG_OM_IP_IPCCMD_GETIPV6ENABLESTATUS:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv6EnableStatus(
                netcfg_om_msg_p->data.u32a1_bla2.u32_a1,
                &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETIPV6ADDRAUTOCONFIGENABLESTATUS:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(
                netcfg_om_msg_p->data.u32a1_bla2.u32_a1,
                &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTIPV6ADDRAUTOCONFIGENABLESTATUS:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextIPv6AddrAutoconfigEnableStatus(
                &(netcfg_om_msg_p->data.u32a1_bla2.u32_a1),
                &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETIPV6RIFCONFIG:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv6RifConfig(
                &netcfg_om_msg_p->data.inet_rif_config_v);
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTIPV6RIFCONFIG:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextIPv6RifConfig(
                &netcfg_om_msg_p->data.inet_rif_config_v);
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;


        case NETCFG_OM_IP_IPCCMD_GETIPV6RIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv6RifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETNEXTIPV6RIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextIPv6RifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;

        case NETCFG_OM_IP_IPCCMD_GETLINKLOCALRIFFROMINTERFACE:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetLinkLocalRifFromInterface(
                &(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;
        case NETCFG_OM_IP_IPCCMD_GETIPV6INTERFACEMTU:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv6InterfaceMTU(
                netcfg_om_msg_p->data.u32a1_u32a2.u32_a1,
                &(netcfg_om_msg_p->data.u32a1_u32a2.u32_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_u32a2);
            need_respond = TRUE;
            break;

#endif /* SYS_CPNT_IPV6 */
#if (SYS_CPNT_CRAFT_PORT == TRUE)
        case NETCFG_OM_IP_IPCCMD_GETCRAFTINTERFACEINETADDRESS:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetCraftInterfaceInetAddress(
                &(netcfg_om_msg_p->data.craft_addr));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(craft_addr);
            need_respond = TRUE;
            break;
        case NETCFG_OM_IP_IPCCMD_GETIPV6ENABLESTATUS_CRAFT:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetIPv6EnableStatus_Craft(
                netcfg_om_msg_p->data.u32a1_bla2.u32_a1, &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;
#endif /* SYS_CPNT_CRAFT_PORT */


#if (SYS_CPNT_DHCP_INFORM == TRUE)
        case NETCFG_OM_IP_IPCCMD_GET_DHCP_INFORM:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetDhcpInform(
                netcfg_om_msg_p->data.u32a1_bla2.u32_a1, &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;
        case NETCFG_OM_IP_IPCCMD_GET_RUNNING_DHCP_INFORM:
            netcfg_om_msg_p->type.result_running_cfg =
                NETCFG_OM_IP_GetRunningDhcpInform(netcfg_om_msg_p->data.u32a1_bla2.u32_a1,
                                                  &(netcfg_om_msg_p->data.u32a1_bla2.bl_a2));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;
#endif /* SYS_CPNT_DHCP_INFORM */
#if (SYS_CPNT_VIRTUAL_IP == TRUE)
        case NETCFG_OM_IP_IPCCMD_GETNEXTVIRTUALRIFBYIFINDEX:
            netcfg_om_msg_p->type.result_ui32 = NETCFG_OM_IP_GetNextVirtualRifByIfindex(&(netcfg_om_msg_p->data.inet_rif_config_v));
            ipcmsg_p->msg_size=NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
            need_respond = TRUE;
            break;
#
#endif
        default:
            printf("%s() %ld : Invalid cmd.\n", __FUNCTION__, (long)netcfg_om_msg_p->type.cmd);
            /* Unknow command. There is no way to idntify whether this
             * ipc message need or not need a response. If we response to
             * a asynchronous msg, then all following synchronous msg will
             * get wrong responses and that might not easy to debug.
             * If we do not response to a synchronous msg, the requester
             * will be blocked forever. It should be easy to debug that
             * error.
             */
            need_respond=FALSE;
    }
    return need_respond;
}

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
BOOL_T NETCFG_OM_IP_IsL3InterfaceTableFull(void)
{
    UI32_T orig_priority;
    BOOL_T rc = FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if(intf_list.nbr_of_element == intf_list.max_element_count)
    {
        rc = TRUE;
        DBGprintf("NETCFG_OM_IP L3If table full. (%lu)\r\n", (unsigned long)intf_list.max_element_count);
    }
    else
    {
        rc = FALSE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return rc;
}

/* FUNCTION NAME : NETCFG_OM_IP_CreateInterface
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
UI32_T NETCFG_OM_IP_CreateL3Interface(NETCFG_TYPE_L3_Interface_T *intf)
{
    UI32_T orig_priority;
    NETCFG_OM_L3_Interface_T intf_om;

    if (NULL == intf)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf_om, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf_om.ifindex = intf->ifindex;
    intf_om.iftype = intf->iftype;
    intf_om.drv_l3_intf_index = intf->drv_l3_intf_index;
#if (SYS_CPNT_IPV6 == TRUE)
    intf_om.ipv6_enable = intf->ipv6_enable;
    intf_om.ipv6_autoconf_enable = intf->ipv6_autoconf_enable;
    intf_om.u.physical_intf.mtu6 = SYS_DFLT_IPV6_INTERFACE_MTU;
    intf_om.u.physical_intf.accept_ra_pinfo = TRUE;
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    intf_om.dhcp_inform = SYS_DFLT_DHCP_INFORM;
#endif

    if(intf->iftype == VLAN_L3_IP_IFTYPE)
    {
        intf_om.u.physical_intf.if_flags = intf->u.physical_intf.if_flags;

        memcpy(intf_om.u.physical_intf.hw_addr, intf->u.physical_intf.hw_addr, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(intf_om.u.physical_intf.logical_mac, intf->u.physical_intf.logical_mac, SYS_ADPT_MAC_ADDR_LEN);
        intf_om.u.physical_intf.logical_mac_set = intf->u.physical_intf.logical_mac_set;

        intf_om.u.physical_intf.mtu = intf->u.physical_intf.mtu;
        intf_om.u.physical_intf.bandwidth = intf->u.physical_intf.bandwidth;
#if (SYS_CPNT_PROXY_ARP == TRUE)
        intf_om.u.physical_intf.proxy_arp_enable = intf->u.physical_intf.proxy_arp_enable;
#endif
        intf_om.u.physical_intf.ipv4_address_mode = intf->u.physical_intf.ipv4_address_mode;
    }
    else
    {
        DBGprintf(" no iftype specified!");
        return NETCFG_TYPE_FAIL;
    }

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Set(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_DeleteL3Interface(NETCFG_TYPE_L3_Interface_T *intf)
{
    UI32_T orig_priority;
    NETCFG_OM_L3_Interface_T intf_om;

    if (NULL == intf)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf_om, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf_om.ifindex = intf->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Delete(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}


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
void NETCFG_OM_IP_DeleteAllInterface(void)
{
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    L_SORT_LST_Delete_All(&intf_list);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return;
}

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
UI32_T NETCFG_OM_IP_SetInterfaceAcceptRaPrefixInfo(UI32_T ifindex, BOOL_T status)
{
    UI32_T orig_priority;
    NETCFG_OM_L3_Interface_T intf_om;

    intf_om.ifindex = ifindex;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    intf_om.u.physical_intf.accept_ra_pinfo = status;
    if (L_SORT_LST_Set(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_SetInterfaceFlags
 * PURPOSE:
 *          Set flags of an ip interface entry.
 *
 * INPUT:
 *      intf  -- the interface entry.
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
UI32_T NETCFG_OM_IP_SetInterfaceFlags(NETCFG_TYPE_L3_Interface_T *intf, UI16_T flags)
{
    UI32_T orig_priority;
    NETCFG_OM_L3_Interface_T intf_om;

    if (NULL == intf)
        return NETCFG_TYPE_INVALID_ARG;

    intf_om.ifindex = intf->ifindex;
    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    intf_om.u.physical_intf.if_flags |= flags;
    if (L_SORT_LST_Set(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_UnsetInterfaceFlags
 * PURPOSE:
 *          Unset flags of an ip interface entry.
 *
 * INPUT:
 *      intf  -- the interface entry.
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
UI32_T NETCFG_OM_IP_UnsetInterfaceFlags(NETCFG_TYPE_L3_Interface_T *intf, UI16_T flags)
{
    UI32_T orig_priority;
    NETCFG_OM_L3_Interface_T intf_om;

    if (NULL == intf)
        return NETCFG_TYPE_INVALID_ARG;

    intf_om.ifindex = intf->ifindex;
    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    intf_om.u.physical_intf.if_flags &= ~(flags);
    if (L_SORT_LST_Set(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetL3Interface(NETCFG_TYPE_L3_Interface_T *intf)
{
    NETCFG_OM_L3_Interface_T intf_om;
    UI32_T orig_priority;

    if (NULL == intf)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf_om, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf_om.ifindex = intf->ifindex;
    NOTEprintf("search key=%ld", (long)intf_om.ifindex);
    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    intf->ifindex = intf_om.ifindex;
    intf->iftype = intf_om.iftype;
#if (SYS_CPNT_IPV6 == TRUE)
    intf->ipv6_enable = intf_om.ipv6_enable;
    intf->ipv6_autoconf_enable = intf_om.ipv6_autoconf_enable;
#endif
    intf->drv_l3_intf_index = intf_om.drv_l3_intf_index;

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    intf->dhcp_inform = intf_om.dhcp_inform;
#endif

    if(intf_om.iftype == VLAN_L3_IP_IFTYPE)
    {
        intf->u.physical_intf.if_flags = intf_om.u.physical_intf.if_flags;
        intf->u.physical_intf.mtu = intf_om.u.physical_intf.mtu;
#if (SYS_CPNT_IPV6 ==  TRUE)
        intf->u.physical_intf.mtu6 = intf_om.u.physical_intf.mtu6;
        intf->u.physical_intf.accept_ra_pinfo = intf_om.u.physical_intf.accept_ra_pinfo;
#endif
        intf->u.physical_intf.bandwidth = intf_om.u.physical_intf.bandwidth;
#if (SYS_CPNT_PROXY_ARP == TRUE)
        intf->u.physical_intf.proxy_arp_enable = intf_om.u.physical_intf.proxy_arp_enable;
#endif
        intf->u.physical_intf.ipv4_address_mode = intf_om.u.physical_intf.ipv4_address_mode;
        memcpy(intf->u.physical_intf.hw_addr, intf_om.u.physical_intf.hw_addr, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(intf->u.physical_intf.logical_mac, intf_om.u.physical_intf.logical_mac, SYS_ADPT_MAC_ADDR_LEN);
    }
    else
    {
        /*unknown address type*/
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        DBGprintf("Unknown if type = %d",intf_om.iftype);
        return NETCFG_TYPE_FAIL;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_GetNextL3Interface(NETCFG_TYPE_L3_Interface_T *intf)
{
    NETCFG_OM_L3_Interface_T intf_om;
    BOOL_T rc;
    UI32_T orig_priority;

    if (NULL == intf)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf_om, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf_om.ifindex = intf->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (intf_om.ifindex == 0)
    {
        rc = L_SORT_LST_Get_1st(&intf_list, &intf_om);
    }
    else
    {
        rc = L_SORT_LST_Get_Next(&intf_list, &intf_om);
    }

    if (rc == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_MORE_ENTRY;
    }

    intf->ifindex = intf_om.ifindex;
    intf->iftype = intf_om.iftype;
    intf->drv_l3_intf_index = intf_om.drv_l3_intf_index;
#if (SYS_CPNT_IPV6 == TRUE)
    intf->ipv6_enable = intf_om.ipv6_enable;
    intf->ipv6_autoconf_enable = intf_om.ipv6_autoconf_enable;
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    intf->dhcp_inform = intf_om.dhcp_inform;
#endif

    if( VLAN_L3_IP_IFTYPE ==intf->iftype)
    {
        intf->u.physical_intf.if_flags = intf_om.u.physical_intf.if_flags;
        intf->u.physical_intf.mtu = intf_om.u.physical_intf.mtu;
#if (SYS_CPNT_IPV6 ==  TRUE)
        intf->u.physical_intf.mtu6 = intf_om.u.physical_intf.mtu6;
        intf->u.physical_intf.accept_ra_pinfo = intf_om.u.physical_intf.accept_ra_pinfo;
#endif
        intf->u.physical_intf.bandwidth = intf_om.u.physical_intf.bandwidth;
#if (SYS_CPNT_PROXY_ARP == TRUE)
        intf->u.physical_intf.proxy_arp_enable = intf_om.u.physical_intf.proxy_arp_enable;
#endif
        intf->u.physical_intf.ipv4_address_mode = intf_om.u.physical_intf.ipv4_address_mode;
        memcpy(intf->u.physical_intf.hw_addr, intf_om.u.physical_intf.hw_addr, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(intf->u.physical_intf.logical_mac, intf_om.u.physical_intf.logical_mac, SYS_ADPT_MAC_ADDR_LEN);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_SetIpAddressMode
 * PURPOSE:
 *      Set the IPv4 addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      mode
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
UI32_T NETCFG_OM_IP_SetIpAddressMode(UI32_T ifindex, UI32_T mode)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T entry;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    UI32_T orig_priority;

    /* BODY
     */
    memset(&entry, 0, sizeof(NETCFG_OM_L3_Interface_T));
    entry.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &entry);
    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    /* Update address mode of related interface */
    if( VLAN_L3_IP_IFTYPE == mem_intf->iftype)
        mem_intf->u.physical_intf.ipv4_address_mode = mode;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_GetIpAddressMode
 * PURPOSE:
 *      Get the IPv4 addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      mode_p      -- pointer to the mode.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This is only for IPv4.
 */
static UI32_T NETCFG_OM_IP_GetIpAddressMode(UI32_T ifindex, UI32_T *mode_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T entry;
    UI32_T orig_priority;

    /* BODY
     */
    if(NULL == mode_p)
    {
        return (NETCFG_TYPE_INVALID_ARG);
    }

    memset(&entry, 0, sizeof(NETCFG_OM_L3_Interface_T));
    entry.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &entry) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }
    if(entry.iftype == VLAN_L3_IP_IFTYPE)
        *mode_p = entry.u.physical_intf.ipv4_address_mode;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_GetNextIpAddressMode
 * PURPOSE:
 *      Get the IPv4 addr_mode for the L3 interface next to the specified one.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      ifindex     -- the ifindex of next L3 interface.
 *      mode_p      -- pointer to the mode.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. To get the first ip address mode, set *ifindex_p as 0.
 *      2. This is only for IPv4.
 */
UI32_T NETCFG_OM_IP_GetNextIpAddressMode(UI32_T *ifindex_p, UI32_T *mode_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T entry;
    BOOL_T rc;
    UI32_T orig_priority;

    /* BODY
     */
    if ((NULL == mode_p) || (NULL == ifindex_p))
    {
        return (NETCFG_TYPE_INVALID_ARG);
    }

    memset(&entry, 0, sizeof(NETCFG_OM_L3_Interface_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (0 == *ifindex_p)
    {
        rc = L_SORT_LST_Get_1st(&intf_list, &entry);
    }
    else
    {
        entry.ifindex = *ifindex_p;
        rc = L_SORT_LST_Get_Next(&intf_list, &entry);
    }

    if (rc == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    *ifindex_p = entry.ifindex;
    if(entry.iftype == VLAN_L3_IP_IFTYPE)
        *mode_p = entry.u.physical_intf.ipv4_address_mode;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_SetInetRif
 * PURPOSE:
 *      Add or update rif for the specified L3 interface(by ifindex)
 *
 * INPUT:
 *      rif_config_p->ifindex               -- specified L3 interface.
 *      rif_config_p->ipv4_role             -- role.
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
UI32_T NETCFG_OM_IP_SetInetRif(NETCFG_OM_IP_InetRifConfig_T *rif_config_p)
{
    UI32_T orig_priority;
    NETCFG_OM_IP_InetRifConfig_T local_rif;
    NETCFG_OM_L3_Interface_T local_interface;
    NETCFG_OM_IP_InetRifConfig_T *mem_rif = NULL;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    BOOL_T is_update = FALSE;

#if (SYS_CPNT_IPV6 == TRUE)
    NOTEprintf("SET on vlan %ld,(%d)%lx:%lx:%lx:%lx,status=%ld,v4=%ld, v6=%ld",
                    (long)rif_config_p->ifindex,
                    rif_config_p->addr.type, L_INET_EXPAND_IPV6(rif_config_p->addr.addr),
                    (long)rif_config_p->row_status,
                    (long)rif_config_p->ipv4_role ,
                    (long)rif_config_p->ipv6_addr_type
                );
#else
    NOTEprintf("SET on vlan %ld,(%d),status=%ld,v4=%ld",
                    (long)rif_config_p->ifindex,
                    rif_config_p->addr.type,
                    (long)rif_config_p->row_status,
                    (long)rif_config_p->ipv4_role
                );
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&local_rif, 0, sizeof (NETCFG_OM_IP_InetRifConfig_T));
    memset(&local_interface, 0, sizeof (NETCFG_OM_L3_Interface_T));
    memcpy(&(local_rif.addr), &(rif_config_p->addr), sizeof(L_INET_AddrIp_T));
    local_rif.ifindex = rif_config_p->ifindex;
    local_interface.ifindex = rif_config_p->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &local_interface);
    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_SUCH_INTERFACE;
    }

    if (L_SORT_LST_ShMem_Get(&rif_list, &local_rif) == TRUE)
    {
        if (rif_config_p->addr.addrlen == L_INET_ADDR_TYPE_IPV4 ||
            rif_config_p->addr.addrlen == L_INET_ADDR_TYPE_IPV4Z)
        {
            if (local_rif.ifindex != rif_config_p->ifindex)
            {
                /* the subnet exist, but not on same interface */
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_CAN_NOT_PERFORM_CHANGE;
            }

            /* Changing from primary to secondary is not allowed */
            if ((rif_config_p->ipv4_role == NETCFG_TYPE_MODE_SECONDARY) &&
                (local_rif.ipv4_role == NETCFG_TYPE_MODE_PRIMARY))
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_CAN_NOT_PERFORM_CHANGE;
            }
            /* Changing from secondary to primary is not allowed */
            if ((rif_config_p->ipv4_role == NETCFG_TYPE_MODE_PRIMARY) &&
                (local_rif.ipv4_role == NETCFG_TYPE_MODE_SECONDARY))
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_CAN_NOT_PERFORM_CHANGE;
            }

            if ((local_rif.ipv4_role == rif_config_p->ipv4_role) &&
                (local_rif.row_status == rif_config_p->row_status))
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_OK;
            }
            /* Can only update row_status */
            else if ((local_rif.ipv4_role == rif_config_p->ipv4_role) &&
                     (local_rif.row_status != rif_config_p->row_status))
            {
                L_SORT_LST_ShMem_Delete(&rif_list, &local_rif);
                is_update = TRUE;
            }
        }
        else /* IPV6 or IPV6Z */
        {
            if (local_rif.ifindex != rif_config_p->ifindex)
            {
                /* the subnet exist, but not on same interface */
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_CAN_NOT_PERFORM_CHANGE;
            }
            if (local_rif.ipv6_addr_type != rif_config_p->ipv6_addr_type ||
                local_rif.ipv6_addr_config_type != rif_config_p->ipv6_addr_config_type ||
                local_rif.row_status != rif_config_p->row_status)
            {
                L_SORT_LST_ShMem_Delete(&rif_list, &local_rif);
                is_update = TRUE;
            }
        }
    }

    if (L_SORT_LST_ShMem_Set(&rif_list, rif_config_p) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    if (is_update == FALSE)
    {
        mem_rif = L_SORT_LST_ShMem_GetPtr(&rif_list, rif_config_p);
        /* Then, attach the rif to the rif list of interface */
        if (NETCFG_OM_IP_AttachInetRifToInterface(mem_intf, mem_rif) != NETCFG_TYPE_OK)
        {
            /* Roll back */
            L_SORT_LST_ShMem_Delete(&rif_list, rif_config_p);
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            return NETCFG_TYPE_FAIL;
        }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_DeleteInetRif
 * PURPOSE:
 *      Delete rif by specified ifindex and ip_addr.
 *
 * INPUT:
 *      rif_config_p->ifindex           -- specified ifindex.
 *      rif_config_p->primary_interface -- role.
 *      rif_config_p->ip_addr           -- the ip address.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. It will check if there is a rif entry with specified ip in the specified interface.
 */
UI32_T NETCFG_OM_IP_DeleteInetRif(NETCFG_OM_IP_InetRifConfig_T *rif_config_p)
{
    UI32_T orig_priority;
    NETCFG_OM_IP_InetRifConfig_T local_rif;
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    NETCFG_OM_IP_InetRifConfig_T *mem_rif;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&local_rif, 0, sizeof (NETCFG_OM_IP_InetRifConfig_T));
    memset(&intf, 0, sizeof (NETCFG_OM_L3_Interface_T));
    memcpy(&local_rif.addr, &(rif_config_p->addr), sizeof(L_INET_AddrIp_T));
    local_rif.ifindex = rif_config_p->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = rif_config_p->ifindex;
    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf);
    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_SUCH_INTERFACE;
    }

    mem_rif = L_SORT_LST_ShMem_GetPtr(&rif_list, &local_rif);
    if (NULL == mem_rif)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    if ((mem_rif->row_status != rif_config_p->row_status) ||
        (mem_rif->ipv4_role != rif_config_p->ipv4_role))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    /* Detach rif from interface */
    if (NETCFG_OM_IP_DetachInetRifFromInterface(mem_intf, mem_rif) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    if (L_SORT_LST_ShMem_Delete(&rif_list, &local_rif) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }


    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_SetInetRifRowStatus
 * PURPOSE:
 *      Set row_status of the rif.
 *
 * INPUT:
 *      rif_config_p->ip_addr       -- ip address (key).
 *      rif_config_p->row_status    -- row_status.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. Only rowstatus value is updated, no any action.
 */
UI32_T NETCFG_OM_IP_SetInetRifRowStatus(NETCFG_OM_IP_InetRifConfig_T *rif_config_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T *mem_rif = NULL;
    UI32_T orig_priority;


    /* BODY
     */
    if (NULL == rif_config_p)
        return (NETCFG_TYPE_INVALID_ARG);

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_rif = L_SORT_LST_ShMem_GetPtr(&rif_list, rif_config_p);
    if (NULL != mem_rif)
    {
        mem_rif->row_status = rif_config_p->row_status;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}



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
 *      The KEY is only ip address.
 */
UI32_T NETCFG_OM_IP_GetInetRif(NETCFG_OM_IP_InetRifConfig_T *rif)
{
    UI32_T orig_priority;

    if (NULL == rif)
        return NETCFG_TYPE_INVALID_ARG;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_ShMem_Get(&rif_list, rif) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetIPv4RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    UI32_T orig_priority;
    NETCFG_OM_IP_InetRifConfig_T rif_om;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    /* are those fields enough ? */
    rif_om.ifindex = rif_config_p->ifindex;
    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_ShMem_Get(&rif_list, &rif_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->row_status = rif_om.row_status;
    rif_config_p->ipv4_role = rif_om.ipv4_role;
    rif_config_p->flags = rif_om.flags;
    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_GetNextIPv4RifConfig
 * PURPOSE:
 *      Get the Next rif by the ip address and interface index.
 *
 * INPUT:
 *      rif_config_p->ip_addr    -- ip address (key)
 *      rif_config_p->ifindex    -- interface index (key)
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
 *      1. The KEY is ip address and interface index.
 *      2. Also search in the ipv4 rif of craft interface.
 */
UI32_T NETCFG_OM_IP_GetNextIPv4RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T orig_priority;
    BOOL_T rif_found = FALSE; /* rifconfig is found */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    BOOL_T craft_found = FALSE;
    NETCFG_TYPE_CraftInetAddress_T craft_addr;
#endif

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));
    rif_om.ifindex = rif_config_p->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if ((L_SORT_LST_ShMem_Get_Next(&rif_list, &rif_om) == FALSE)
        || ((L_INET_ADDR_TYPE_IPV4 != rif_om.addr.type)
            && (L_INET_ADDR_TYPE_IPV4Z != rif_om.addr.type)))
    {
        rif_found = FALSE;
    }
    else
        rif_found = TRUE;

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    memset(&craft_addr, 0, sizeof(craft_addr));
    //craft_addr.addr.type = L_INET_ADDR_TYPE_IPV4;
    //if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_addr))
    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(craft_addr), (void*) &craft_addr))
    {
        craft_found = TRUE;
    }
    DBGprintf("rif_found:%d, craft_found:%d\n", rif_found, craft_found);
    if(DEBUG_FLAG & DEBUG_FLAG_BIT_DEBUG)
    {
        DUMP_INET_ADDR(rif_config_p->addr);
        DUMP_INET_ADDR(rif_om.addr);
        DUMP_INET_ADDR(craft_addr.addr);
    }
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    if(rif_found)
    {
        if(craft_found && (memcmp(rif_config_p->addr.addr, craft_addr.addr.addr, 4) <0)
            && (memcmp(craft_addr.addr.addr, rif_om.addr.addr, 4) <0))
            goto copy_craft;
        else
            goto copy_rif;
    }
    else
    {
        if(craft_found && (memcmp(rif_config_p->addr.addr, craft_addr.addr.addr, 4) <0))
            goto copy_craft;
        else
            goto no_entry;
    }

#else
    if(rif_found)
        goto copy_rif;
    else
        goto no_entry;
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
copy_craft:
    DBGprintf("label copy_craft\n");

    memcpy(&rif_config_p->addr, &craft_addr.addr, sizeof(L_INET_AddrIp_T));
    rif_config_p->ifindex = craft_addr.ifindex;
    rif_config_p->ipv4_role= NETCFG_TYPE_MODE_PRIMARY;
    rif_config_p->flags = craft_addr.flags;
    rif_config_p->row_status = craft_addr.row_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
#endif
copy_rif:
    DBGprintf("label copy_rif\n");

    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));
    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->ipv4_role= rif_om.ipv4_role;
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;

no_entry:
    DBGprintf("label no_entry\n");

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_NO_MORE_ENTRY;
}

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
 *      1. The input ifindex must not be 0 and this function will not go
 *      next interface.
 *      2. The sorting & searching KEYs are:
 *          (1) prefix type (IPv4 or IPv6)
 *          (2) prefix length
 *          (3) prefix address
 */
UI32_T NETCFG_OM_IP_GetNextInetRifByIfindex(NETCFG_OM_IP_InetRifConfig_T *rif, UI32_T ifindex)
{
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    UI32_T rc;
    UI32_T orig_priority;

    if ((NULL == rif) || (0 == ifindex))
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf, 0, sizeof (NETCFG_OM_L3_Interface_T));
    /* Specifies the search KEY */
    intf.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf);
    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_SUCH_INTERFACE;
    }
    if (NULL == mem_intf->inet_rif_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    /* peter_yu, if *rif is all zero( rif->addr.type is zero, too), we should return the first element */
    /* Get Next rif of the interface */
    rc = NETCFG_OM_IP_GetInterfaceInetRif(mem_intf, rif, TRUE);


    if (NETCFG_TYPE_OK != rc)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_MORE_ENTRY;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_GetNextInetRifEntryByIfindex
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
 *      1. The input ifindex must not be 0 and this function will not go
 *      next interface.
 *      2. The sorting & searching KEYs are:
 *          (1) prefix type (IPv4 or IPv6)
 *          (2) prefix length
 *          (3) prefix address
 *      3. The caller should apply the mutual-exclusive protection.
 */
static UI32_T NETCFG_OM_IP_GetNextInetRifEntryByIfindex(NETCFG_OM_IP_InetRifConfig_T *rif, UI32_T ifindex)
{
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    UI32_T rc;

    if ((NULL == rif) || (0 == ifindex))
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf, 0, sizeof (NETCFG_OM_L3_Interface_T));
    /* Specifies the search KEY */
    intf.ifindex = ifindex;

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf);
    if (NULL == mem_intf)
        return NETCFG_TYPE_NO_SUCH_INTERFACE;
    if( VLAN_L3_IP_IFTYPE == mem_intf->iftype)
        if (TRUE == IP_LIB_IsLoopbackInterface(mem_intf->u.physical_intf.if_flags))
        {
            if (NULL == mem_intf->inet_rif_p)
                return NETCFG_TYPE_NO_MORE_ENTRY;
        }

    if (NULL == mem_intf->inet_rif_p)
        return NETCFG_TYPE_FAIL;
#if 0
    if ((L_INET_ADDR_TYPE_IPV4 ==rif->addr.type)&& !memcmp(rif->addr.addr, ipv4_zero_addr, SYS_ADPT_IPV4_ADDR_LEN))
    {
        /* Return the first element */
        memcpy(rif, mem_intf->inet_rif_p, sizeof (NETCFG_OM_IP_InetRifConfig_T));
        rc = NETCFG_TYPE_OK;
    }
    else
    {
        /* Get Next rif of the interface */
        rc = NETCFG_OM_IP_GetInterfaceInetRif(mem_intf, rif, TRUE);
    }
#endif

    /* peter_yu, if *rif is all zero( rif->addr.type is zero, too), we should return the first element */
    /* Get Next rif of the interface */
    rc = NETCFG_OM_IP_GetInterfaceInetRif(mem_intf, rif, TRUE);

    if (NETCFG_TYPE_OK != rc)
        return NETCFG_TYPE_NO_MORE_ENTRY;

    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_LookupPrimaryRif(NETCFG_OM_IP_InetRifConfig_T *primary_rif, UI32_T ifindex)
{
    UI32_T orig_priority;

    if (NULL == primary_rif)
        return NETCFG_TYPE_INVALID_ARG;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (NETCFG_OM_IP_InternalLookupPrimaryRif(primary_rif, ifindex) !=
                NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_GetInterfaceRifCount(UI32_T ifindex, int *count)
{
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    NETCFG_OM_IP_InetRifConfig_T *rif_p;
    int nbr_rif = 0;
    UI32_T orig_priority;

    if (NULL == count)
        return NETCFG_TYPE_INVALID_ARG;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    memset(&intf, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf.ifindex = ifindex;
    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf);
    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_SUCH_INTERFACE;
    }

    for (rif_p = mem_intf->inet_rif_p; rif_p; rif_p = rif_p->next)
        nbr_rif++;

    *count = nbr_rif;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_CompareInterface
 * PURPOSE:
 *      Compare two L3 interfaces by ifindex.
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
 *      None.
 */
static int NETCFG_OM_IP_CompareInterface(void* rec1, void* rec2)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T *p1 = (NETCFG_OM_L3_Interface_T *) rec1;
    NETCFG_OM_L3_Interface_T *p2 = (NETCFG_OM_L3_Interface_T *) rec2;

    /* BODY
     */
    if (p1->ifindex > p2->ifindex)
        return 1;
    else if (p1->ifindex < p2->ifindex)
        return -1;
    else
        return 0;
}

/* FUNCTION NAME : NETCFG_OM_IP_CompareSubnet
 * PURPOSE:
 *      Compare whether IP address is inside a subnet
 *
 * INPUT:
 *      rec     -- the pointer to the rif entry.
 *      cookie  -- the pointer to the search info structure.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      L_SORT_LST_SEARCH_BREAK
 *      L_SORT_LST_SEARCH_CONTINUE
 * NOTES:
 *      Use for L_SORT_LST_Search() callback function
 */
static UI32_T NETCFG_OM_IP_CompareSubnet(const void *rec, void *cookie)
{
    const NETCFG_OM_IP_InetRifConfig_T *rif_om_p = rec;
    NETCFG_OM_RifSearchInfo_T *info_p = cookie;

    info_p->found = FALSE;

    /* The Rif list is sorted by (type, addr, zone_id)
     */
    if (rif_om_p->addr.type > info_p->ipaddr.type)
    {
        return L_SORT_LST_SEARCH_BREAK;
    }
    else if (rif_om_p->addr.type == info_p->ipaddr.type)
    {
        if (IP_LIB_IsIpBelongToSubnet(rif_om_p->addr.addr, rif_om_p->addr.preflen, info_p->ipaddr.addr))
        {
            if(L_INET_ADDR_TYPE_IPV4Z == info_p->ipaddr.type || L_INET_ADDR_TYPE_IPV6Z == info_p->ipaddr.type)
            {
                if (rif_om_p->addr.zoneid != info_p->ipaddr.zoneid)
                    return L_SORT_LST_SEARCH_CONTINUE;
            }

            memcpy(&info_p->result, rif_om_p, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            info_p->found = TRUE;
            return L_SORT_LST_SEARCH_BREAK;
        }
    }

    return L_SORT_LST_SEARCH_CONTINUE;
}

/* FUNCTION NAME : NETCFG_OM_IP_CompareSubnetAndIfindex
 * PURPOSE:
 *      Compare whether IP address is inside a subnet with specified ifindex
 *
 * INPUT:
 *      rec     -- the pointer to the rif entry.
 *      cookie  -- the pointer to the search info structure.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      L_SORT_LST_SEARCH_BREAK
 *      L_SORT_LST_SEARCH_CONTINUE
 * NOTES:
 *      1. Used for L_SORT_LST_Search() callback function
 *      2. For vrf use case
 */
static UI32_T NETCFG_OM_IP_CompareSubnetAndIfindex(const void *rec, void *cookie)
{
    const NETCFG_OM_IP_InetRifConfig_T  *rif_om_p = rec;
    NETCFG_OM_RifSearchInfo_T           *info_p = cookie;

    info_p->found = FALSE;

    /* The Rif list is sorted by (type, addr, zone_id, ifindex)
     */
    if (rif_om_p->addr.type > info_p->ipaddr.type)
    {
        return L_SORT_LST_SEARCH_BREAK;
    }
    else if (rif_om_p->addr.type == info_p->ipaddr.type)
    {
        if (IP_LIB_IsIpBelongToSubnet(rif_om_p->addr.addr, rif_om_p->addr.preflen, info_p->ipaddr.addr))
        {
            if(L_INET_ADDR_TYPE_IPV4Z == info_p->ipaddr.type || L_INET_ADDR_TYPE_IPV6Z == info_p->ipaddr.type)
            {
                if (rif_om_p->addr.zoneid != info_p->ipaddr.zoneid)
                    return L_SORT_LST_SEARCH_CONTINUE;
            }

            if (rif_om_p->ifindex != info_p->ifindex)
                return L_SORT_LST_SEARCH_CONTINUE;

            memcpy(&info_p->result, rif_om_p, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            info_p->found = TRUE;
            return L_SORT_LST_SEARCH_BREAK;
        }
    }

    return L_SORT_LST_SEARCH_CONTINUE;
}

/* FUNCTION NAME : NETCFG_OM_IP_CompareIpAndIfindex
 * PURPOSE:
 *      Compare whether IP address and ifindex is the same
 *
 * INPUT:
 *      rec     -- the pointer to the rif entry.
 *      cookie  -- the pointer to the search info structure.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      L_SORT_LST_SEARCH_BREAK
 *      L_SORT_LST_SEARCH_CONTINUE
 * NOTES:
 *      1. Used for L_SORT_LST_Search() callback function
 *      2. For vrf use case
 */
static UI32_T NETCFG_OM_IP_CompareIpAndIfindex(const void *rec, void *cookie)
{
    const NETCFG_OM_IP_InetRifConfig_T  *rif_om_p = rec;
    NETCFG_OM_RifSearchInfo_T           *info_p = cookie;

    info_p->found = FALSE;

    /* The Rif list is sorted by (type, addr, zone_id, ifindex)
     */
    if (rif_om_p->addr.type > info_p->ipaddr.type)
    {
        return L_SORT_LST_SEARCH_BREAK;
    }
    else if (rif_om_p->addr.type == info_p->ipaddr.type)
    {
        if (!memcmp(rif_om_p->addr.addr, info_p->ipaddr.addr, rif_om_p->addr.addrlen))
        {
            if (  (info_p->ipaddr.preflen != 0)
                &&(rif_om_p->addr.preflen != info_p->ipaddr.preflen)
               )
            {
                return L_SORT_LST_SEARCH_CONTINUE;
            }

            if (L_INET_ADDR_TYPE_IPV4Z == info_p->ipaddr.type || L_INET_ADDR_TYPE_IPV6Z == info_p->ipaddr.type)
            {
                if (rif_om_p->addr.zoneid != info_p->ipaddr.zoneid)
                    return L_SORT_LST_SEARCH_CONTINUE;
            }

            if (  (TRUE == info_p->chk_ifidx)
                &&(rif_om_p->ifindex != info_p->ifindex)
               )
                return L_SORT_LST_SEARCH_CONTINUE;

            memcpy(&info_p->result, rif_om_p, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            info_p->found = TRUE;
            return L_SORT_LST_SEARCH_BREAK;
        }
    }

    return L_SORT_LST_SEARCH_CONTINUE;
}

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
int NETCFG_OM_IP_CompareRif(void* rec1, void* rec2)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T *p1 = (NETCFG_OM_IP_InetRifConfig_T *) rec1;
    NETCFG_OM_IP_InetRifConfig_T *p2 = (NETCFG_OM_IP_InetRifConfig_T *) rec2;

    UI16_T type1, type2;
    UI16_T addrlen_check;
    UI16_T preflen1, preflen2;
    UI8_T* addr1, *addr2;
    UI32_T zoneid1, zoneid2;
    BOOL_T check_zoneid;
    I32_T rc;

    /* BODY
     */
    check_zoneid = FALSE;
    type1 = p1->addr.type;
    type2 = p2->addr.type;

    addr1 = p1->addr.addr;
    addr2 = p2->addr.addr;

    zoneid1 = p1->addr.zoneid;
    zoneid2 = p2->addr.zoneid;

    preflen1 = p1->addr.preflen;
    preflen2 = p2->addr.preflen;

    /* TO DO : should we remap the sequence to IPV4 < IPV4Z < IPV6 < IPV6Z ?*/
    /* check type */
    if (type1 < type2)
        return -1;
    else if (type1 > type2)
        return 1;

    switch(p1->addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4Z:
            check_zoneid = TRUE;
            /* fall through */
        case L_INET_ADDR_TYPE_IPV4:
            addrlen_check = SYS_ADPT_IPV4_ADDR_LEN;
            break;
        case L_INET_ADDR_TYPE_IPV6Z:
            check_zoneid = TRUE;
            /* fall through */
        case L_INET_ADDR_TYPE_IPV6:
        default:
            addrlen_check = SYS_ADPT_IPV6_ADDR_LEN;
            break;
    }

    /* check addr */
    rc = memcmp(addr1, addr2, addrlen_check);
    if(rc) /* rc!=0, return */
        return rc;

    if (preflen1 < preflen2)
        return -1;
    else if (preflen1 > preflen2)
        return 1;

    /*rc ==0 */
    /* check zoneid for link-local */
    if(check_zoneid)
    {
        rc = zoneid1 - zoneid2;
    }

    /* check ifindex for vrf */
    if (rc == 0)
        rc = p1->ifindex - p2->ifindex;

    return rc;
}

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
 *      NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED
 *      NETCFG_TYPE_INVALID_ARG
 *
 * NOTES:
 *      The overlapping check applied to:
 *      1. primary(input) to secondary(exist);
 *      2. secondary(input) to secondary(exist);
 *      3. seconary(input) to primary(exist);
 */
UI32_T NETCFG_OM_IP_CheckAddressOverlap(NETCFG_OM_L3_Interface_T *intf, NETCFG_OM_IP_InetRifConfig_T *rif)
{
    UI32_T orig_priority;
    NETCFG_OM_IP_InetRifConfig_T exist_rif;
    int overlapped = 0;

    if ((NULL == intf) || (NULL == rif))
        return NETCFG_TYPE_INVALID_ARG;

/* TODO: overlap in same vrf ???
 */
#if 0
    memset(&exist_rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* Traverse all rif entries */
    while(L_SORT_LST_ShMem_Get_Next(&rif_list, &exist_rif) == TRUE)
    {
        /* If it's a updating of primary rif, ignore the
         * overlapping check.
         */
        if (rif->ipv4_role == NETCFG_TYPE_MODE_PRIMARY)
        {
            if (exist_rif.ipv4_role == NETCFG_TYPE_MODE_PRIMARY)
            {
                if (rif->ifindex == exist_rif.ifindex)
                    continue;
            }
        }

        if (rif->addr.preflen < exist_rif.addr.preflen)
            overlapped = IP_LIB_IsIpBelongToSubnet(rif->addr.addr, rif->addr.preflen, exist_rif.addr.addr);
        else
            overlapped = IP_LIB_IsIpBelongToSubnet(exist_rif.addr.addr, exist_rif.addr.preflen, rif->addr.addr);

        if (overlapped == 1)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            return NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED;
        }
    }
#endif


#if (SYS_CPNT_CRAFT_PORT == TRUE)
    {
        NETCFG_TYPE_CraftInetAddress_T
            craft_addr_v4,
            craft_addr_v6_global,
            *craft_addr_p,
            *craft_addr_ar[2] = {};
        int i = 0;

        /* check overlape with craft interface ipv4 address */
        memset(&craft_addr_v4, 0, sizeof(craft_addr_v4));
        memset(&craft_addr_v6_global, 0, sizeof(craft_addr_v6_global));

        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(craft_addr_v4), (void*) &craft_addr_v4))
            craft_addr_ar[0] = &craft_addr_v4;

        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(craft_addr_v6_global), (void*) &craft_addr_v6_global))
            craft_addr_ar[1] = &craft_addr_v6_global;

        for(i=0; i<2; i++)
        {
            craft_addr_p = craft_addr_ar[i];
            if(!craft_addr_p || craft_addr_p->ifindex ==0)
                continue;

            if (rif->addr.preflen < craft_addr_p->addr.preflen)
                overlapped = IP_LIB_IsIpBelongToSubnet(rif->addr.addr, rif->addr.preflen, craft_addr_p->addr.addr);
            else
                overlapped = IP_LIB_IsIpBelongToSubnet(craft_addr_p->addr.addr, craft_addr_p->addr.preflen, rif->addr.addr);
            if(overlapped)
            {
                /* char tmp_buf[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

                L_INET_InaddrToString((L_INET_Addr_T *)&craft_addr_p->addr, tmp_buf, sizeof(tmp_buf));
                printf("overlape with %s\n", tmp_buf);
                */
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED;
            }
        } /* for */
    }
#endif

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_AttachInetRifToInterface
 * PURPOSE:
 *      Attach a new created rif to its related interface
 *
 * INPUT:
 *      intf    -- interface
 *      rif     -- rif to be attached
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1. The rif is stored in rif_list;
 *      2. Attach the rif pointer to the rif list in interface;
 *      3. There's no storage memory for rif in rif list of interface;
 */
static UI32_T NETCFG_OM_IP_AttachInetRifToInterface(NETCFG_OM_L3_Interface_T *intf_p, NETCFG_OM_IP_InetRifConfig_T *rif_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    int ret = 0;
    NETCFG_OM_IP_InetRifConfig_T *rif_pre_node, *rif_cur_node;

    /* BODY
     */

    rif_cur_node = rif_pre_node = NULL;

    if ((NULL == intf_p) || (NULL == rif_p))
        return NETCFG_TYPE_INVALID_ARG;

    rif_cur_node = intf_p->inet_rif_p;
    for (; rif_cur_node; rif_cur_node = rif_cur_node->next)
    {
        ret = NETCFG_OM_IP_CompareRif((void *)rif_p, (void *)rif_cur_node);
        if (ret < 0)
            break;

        rif_pre_node = rif_cur_node;
    }

    if (NULL == rif_pre_node)
        intf_p->inet_rif_p = rif_p;
    else
        rif_pre_node->next = rif_p;

    rif_p->next = rif_cur_node;

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_DetachInetRifFromInterface
 * PURPOSE:
 *      Detach a created rif from its related interface
 *
 * INPUT:
 *      intf    -- interface
 *      rif     -- rif to be detached
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1. The rif is stored in rif_list;
 *      2. Detach the rif pointer from the rif list in interface;
 *      3. There's no storage memory for rif in rif list of interface;
 */
static UI32_T NETCFG_OM_IP_DetachInetRifFromInterface(NETCFG_OM_L3_Interface_T *intf_p, NETCFG_OM_IP_InetRifConfig_T *rif_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T *rif_pre_node, *rif_cur_node;

    /* BODY
     */
    if ((NULL == intf_p) || (NULL == rif_p))
        return NETCFG_TYPE_INVALID_ARG;

    rif_pre_node = NULL;
    rif_cur_node = intf_p->inet_rif_p;
    for (; rif_cur_node;
        rif_pre_node = rif_cur_node, rif_cur_node = rif_cur_node->next)
    {
        if (rif_cur_node == rif_p)
            break;
    }

    /* No rif attached or can not find the node?
     * this should never happen */
    if (NULL == rif_cur_node)
        return NETCFG_TYPE_FAIL;

    /* It it's the first node? */
    if (NULL == rif_pre_node)
        intf_p->inet_rif_p = rif_cur_node->next;
    else
        rif_pre_node->next = rif_cur_node->next;

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_IP_GetInterfaceInetRif
 * PURPOSE:
 *      Get/GetNext the rif from its related interface
 *
 * INPUT:
 *      intf
 *      rif->addr
 *      get_next
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None
 */
static UI32_T NETCFG_OM_IP_GetInterfaceInetRif(NETCFG_OM_L3_Interface_T *intf,
                                    NETCFG_OM_IP_InetRifConfig_T *rif, BOOL_T get_next)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T *rif_cur_node, *result_node;
    int ret = 0;

    /* BODY
     */
    if ((NULL == intf) || (NULL == rif))
        return NETCFG_TYPE_INVALID_ARG;

    result_node = NULL;

    /* for vrf case */
    rif->ifindex = intf->ifindex;

    for (rif_cur_node = intf->inet_rif_p; rif_cur_node ; rif_cur_node = rif_cur_node->next)
    {
        ret = NETCFG_OM_IP_CompareRif(rif, rif_cur_node);

        if (ret == 0)
        {
            if (get_next == TRUE)
                result_node = rif_cur_node->next;
            else
                result_node = rif_cur_node;

            /* found and break */
            break;
        }
        else if (ret > 0)
        {
            continue;
        }
        else /* ret <0 */
        {
            if (get_next == TRUE)
            {
                /* found and break */
                result_node = rif_cur_node;
            }
            break;
        }
    }

    if (NULL == result_node)
        return NETCFG_TYPE_FAIL;

    memcpy(rif, result_node, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_GetRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
     /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T rif_om, *rif_om_p1;
    NETCFG_OM_L3_Interface_T intf, *intf_p1;
    UI32_T orig_priority;
    UI32_T found = FALSE;

     /* BODY
     */
    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    /* if (IP_LIB_IsValidNetworkMask(rif_config_p->mask) == FALSE)
        return NETCFG_TYPE_INVALID_ARG;
     */

    /* check l3 interface */
    memset(&intf, 0, sizeof(intf));
    intf.ifindex = rif_config_p->ifindex;
    intf_p1 = L_SORT_LST_GetPtr(&intf_list, &intf);
    /* interface not exist */
    if (NULL == intf_p1)
        return NETCFG_TYPE_FAIL;
    /* no any rif */
    if (NULL == intf_p1->inet_rif_p)
        return NETCFG_TYPE_FAIL;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* check addr type */
    switch(rif_config_p->addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
            break;
        case L_INET_ADDR_TYPE_UNKNOWN :
        case L_INET_ADDR_TYPE_IPV6 :
        case L_INET_ADDR_TYPE_IPV6Z :
        default:
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            return NETCFG_TYPE_INVALID_ARG;
    }

    /* Does not specified the ip address, get the first rif
     * of the interface.
     */
    if((rif_config_p->addr.type == L_INET_ADDR_TYPE_IPV4)
        && (!memcmp(rif_config_p->addr.addr, ipv4_zero_addr, SYS_ADPT_IPV4_ADDR_LEN)) /* ipv4 addr is zero */
        )
    {
        if((intf_p1->inet_rif_p->addr.type == L_INET_ADDR_TYPE_IPV4)
            ||  (intf_p1->inet_rif_p->addr.type == L_INET_ADDR_TYPE_IPV4Z))
        {
            found = TRUE;
            rif_om_p1 = intf_p1->inet_rif_p;
        }
    }
    else if (NETCFG_TYPE_OK == NETCFG_OM_IP_GetInterfaceInetRif(intf_p1, &rif_om, FALSE))
    {
        found = TRUE;
        rif_om_p1 = &rif_om;

    }

    if(found)
    {
        memcpy(&rif_config_p->addr, &rif_om_p1->addr, sizeof(L_INET_AddrIp_T));

        rif_config_p->ipv4_role = rif_om_p1->ipv4_role;
        rif_config_p->row_status = rif_om_p1->row_status;
        rif_config_p->flags = rif_om_p1->flags;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_FAIL;
}


/* FUNCTION NAME: NETCFG_OM_IP_GetPrimaryRifFromInterface
 * PURPOSE:
 *      Get the primary rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
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
 *      None
 */
static UI32_T NETCFG_OM_IP_GetPrimaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T orig_priority;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (NETCFG_OM_IP_InternalLookupPrimaryRif(&rif_om, rif_config_p->ifindex) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    rif_config_p->ipv4_role = rif_om.ipv4_role;
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME: NETCFG_OM_IP_GetNextSecondaryRifFromInterface
 * PURPOSE:
 *      Get the next secondary rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
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
 *      None
 */
static UI32_T NETCFG_OM_IP_GetNextSecondaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T orig_priority;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (NETCFG_OM_IP_InternalLookupNextSecondaryRif(&rif_om, rif_config_p->ifindex) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    rif_config_p->ipv4_role = rif_om.ipv4_role;
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetNextRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    NETCFG_OM_L3_Interface_T intf;
    BOOL_T entry_got = FALSE;
    UI32_T rc;
    UI32_T orig_priority;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    /* check addr type */
    switch(rif_config_p->addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
            break;
        case L_INET_ADDR_TYPE_UNKNOWN :
        case L_INET_ADDR_TYPE_IPV6 :
        case L_INET_ADDR_TYPE_IPV6Z :
        default:
            return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    memset(&intf, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf.ifindex = rif_config_p->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* If ifindex is not specified, iterate from the first interface */
    if (0 == rif_config_p->ifindex)
        rif_config_p->ifindex = SYS_ADPT_LOOPBACK_IF_INDEX_BASE;

    /* check in the specified interface */
    rc = NETCFG_OM_IP_GetNextInetRifEntryByIfindex(&rif_om, rif_config_p->ifindex);
    if(NETCFG_TYPE_OK == rc)
    {
        /* check addr type */
        if((L_INET_ADDR_TYPE_IPV4 == rif_om.addr.type)
            ||(L_INET_ADDR_TYPE_IPV4Z == rif_om.addr.type))
        {
            entry_got = TRUE;
        }
    }
    /* No more rif in the given interface, try next interface */
    if(NETCFG_TYPE_NO_MORE_ENTRY == rc)
    {
        entry_got = FALSE;
        /* No more rif in the given interface, try next interface */
        while(TRUE == (L_SORT_LST_Get_Next(&intf_list, &intf)))
        {
            /* Then get the first rif of the next interface */
            memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifEntryByIfindex(&rif_om, intf.ifindex))
            {
                /* check addr type */
                if((L_INET_ADDR_TYPE_IPV4 == rif_om.addr.type)
                    ||(L_INET_ADDR_TYPE_IPV4Z == rif_om.addr.type))
                {

                    entry_got = TRUE;
                    break;
                }
            }
        }
    }

    /* No more rif, iteration terminated */
    if (FALSE == entry_got)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->ipv4_role = rif_om.ipv4_role;
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_GetNextInetRifOfInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T orig_priority;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (NETCFG_OM_IP_GetNextInetRifEntryByIfindex(&rif_om, rif_config_p->ifindex) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->ipv4_role = rif_om.ipv4_role;
#if (SYS_CPNT_IPV6 == TRUE)
    rif_config_p->ipv6_addr_type = rif_om.ipv6_addr_type;
    rif_config_p->ipv6_addr_config_type = rif_om.ipv6_addr_config_type;
#endif
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_GetRifFromSubnet(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    return NETCFG_OM_IP_GetRifFromIp(rif_config_p);
}

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
UI32_T NETCFG_OM_IP_GetRifFromIp(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    NETCFG_OM_RifSearchInfo_T search_info;
    BOOL_T subnet_match = FALSE;
    UI32_T orig_priority;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memcpy(&search_info.ipaddr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));
    search_info.found = FALSE;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_SEARCH_BREAK == L_SORT_LST_ShMem_Search(&rif_list, NULL,
            NETCFG_OM_IP_MAX_RIF_NBR, NETCFG_OM_IP_CompareSubnet, &search_info))
    {
        if (search_info.found)
        {
            memcpy(&rif_config_p->addr, &search_info.result.addr, sizeof(L_INET_AddrIp_T));
            rif_config_p->ifindex = search_info.result.ifindex;
            rif_config_p->ipv4_role = search_info.result.ipv4_role;
            rif_config_p->flags = search_info.result.flags;
            rif_config_p->row_status = search_info.result.row_status;

            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            return NETCFG_TYPE_OK;
        }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

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
UI32_T NETCFG_OM_IP_GetRifFromIpAndIfindex(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    NETCFG_OM_IP_InetRifConfig_T    rif_om;
    NETCFG_OM_RifSearchInfo_T       search_info;
    BOOL_T                          subnet_match = FALSE;
    UI32_T                          orig_priority;

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memcpy(&search_info.ipaddr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));
    search_info.ifindex = rif_config_p->ifindex;
    search_info.found = FALSE;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_SEARCH_BREAK == L_SORT_LST_ShMem_Search(&rif_list, NULL,
            NETCFG_OM_IP_MAX_RIF_NBR, NETCFG_OM_IP_CompareSubnetAndIfindex, &search_info))
    {
        if (search_info.found)
        {
            memcpy(&rif_config_p->addr, &search_info.result.addr, sizeof(L_INET_AddrIp_T));
            rif_config_p->ifindex = search_info.result.ifindex;
            rif_config_p->ipv4_role = search_info.result.ipv4_role;
            rif_config_p->flags = search_info.result.flags;
            rif_config_p->row_status = search_info.result.row_status;

            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            return NETCFG_TYPE_OK;
        }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

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
    BOOL_T                      chk_ifidx)
{
    UI32_T                          orig_priority;
    NETCFG_OM_RifSearchInfo_T       search_info;
    BOOL_T                          rif_found = FALSE; /* rifconfig is found */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    NETCFG_TYPE_CraftInetAddress_T  craft_addr;
    BOOL_T                          craft_found = FALSE;
#endif

    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memcpy(&search_info.ipaddr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));
    search_info.found     = FALSE;
    search_info.ifindex   = rif_config_p->ifindex;
    search_info.chk_ifidx = chk_ifidx;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);
    if (L_SORT_LST_SEARCH_BREAK == L_SORT_LST_ShMem_Search(&rif_list, NULL,
            NETCFG_OM_IP_MAX_RIF_NBR, NETCFG_OM_IP_CompareIpAndIfindex, &search_info))
    {
        if (search_info.found)
        {
            rif_found = TRUE;
        }
    }

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    memset(&craft_addr, 0, sizeof(craft_addr));
    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(craft_addr), (void*) &craft_addr))
    {
        craft_found = TRUE;
    }
    DBGprintf("rif_found:%d, craft_found:%d\n", rif_found, craft_found);
    if(DEBUG_FLAG & DEBUG_FLAG_BIT_DEBUG)
    {
        DUMP_INET_ADDR(rif_config_p->addr);
        DUMP_INET_ADDR(search_info.result.addr);
        DUMP_INET_ADDR(craft_addr.addr);
    }
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    if (rif_found == FALSE)
    {
        if(craft_found == FALSE)
            goto no_entry;
        else if(!memcmp(rif_config_p->addr.addr, craft_addr.addr.addr, 4))
            goto copy_craft;
        else
            goto no_entry;
    }
    else
        goto copy_rifconfig;
#else
    if (rif_found == FALSE)
    {
        goto no_entry;
    }
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
copy_craft:
    DBGprintf("label copy_craft\n");

    memcpy(&rif_config_p->addr, &craft_addr.addr, sizeof(L_INET_AddrIp_T));
    rif_config_p->ifindex = craft_addr.ifindex;
    rif_config_p->ipv4_role= NETCFG_TYPE_MODE_PRIMARY;
    rif_config_p->flags = craft_addr.flags;
    rif_config_p->row_status = craft_addr.row_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
#endif

copy_rifconfig:
    DBGprintf("label copy_rifconfig\n");

    rif_config_p->addr      = search_info.result.addr;
    rif_config_p->ifindex   = search_info.result.ifindex;
    rif_config_p->ipv4_role = search_info.result.ipv4_role;
    rif_config_p->flags     = search_info.result.flags;
    rif_config_p->row_status= search_info.result.row_status;

#if (SYS_CPNT_IPV6 == TRUE)
    rif_config_p->ipv6_addr_type = search_info.result.ipv6_addr_type;
    rif_config_p->ipv6_addr_config_type = search_info.result.ipv6_addr_config_type;
#endif /*(SYS_CPNT_IPV6 == TRUE)*/

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;

no_entry:
    DBGprintf("label no_entry\n");

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

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
 *      2. Also search in the ipv4 rif of craft interface.
 */
UI32_T NETCFG_OM_IP_GetRifFromExactIp (NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    return NETCFG_OM_IP_GetRifFromExactIpAndIfindex(rif_config_p, FALSE);
}

/* FUNCTION NAME: NETCFG_OM_IP_GetIpAddrEntry
 * PURPOSE:
 *      Get IpAddrEntry.
 * INPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * OUTPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
static UI32_T NETCFG_OM_IP_GetIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p)
{
    return NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME: NETCFG_OM_IP_GetNextIpAddrEntry
 * PURPOSE:
 *      Get next IpAddrEntry.
 * INPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * OUTPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */

static UI32_T NETCFG_OM_IP_GetNextIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p)
{
    return NETCFG_TYPE_FAIL;
}


/* FUNCTION NAME : NETCFG_OM_IP_InternalLookupPrimaryRif
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
static UI32_T NETCFG_OM_IP_InternalLookupPrimaryRif(NETCFG_OM_IP_InetRifConfig_T *primary_rif, UI32_T ifindex)
{
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    NETCFG_OM_IP_InetRifConfig_T *rif_config = NULL;

    if (NULL == primary_rif)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf.ifindex = ifindex;
    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf);
    if (NULL == mem_intf)
        return NETCFG_TYPE_NO_SUCH_INTERFACE;

    if (NULL == mem_intf->inet_rif_p)
        return NETCFG_TYPE_FAIL;

    for (rif_config = mem_intf->inet_rif_p; rif_config; rif_config = rif_config->next)
    {
        if (rif_config->ipv4_role == NETCFG_TYPE_MODE_PRIMARY)
        {
            memcpy(primary_rif, rif_config, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            return NETCFG_TYPE_OK;
        }
    }

    return NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME : NETCFG_OM_IP_InternalLookupNextSecondaryRif
 * PURPOSE:
 *      Get the next secondary rif of given interface.
 *
 * INPUT:
 *      secondary_rif->addr
 *      ifindex
 *
 * OUTPUT:
 *      secondary_rif
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
static UI32_T NETCFG_OM_IP_InternalLookupNextSecondaryRif(NETCFG_OM_IP_InetRifConfig_T *secondary_rif, UI32_T ifindex)
{
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    NETCFG_OM_IP_InetRifConfig_T *rif_config = NULL;

    if (NULL == secondary_rif)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf.ifindex = ifindex;
    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf);
    if (NULL == mem_intf)
        return NETCFG_TYPE_NO_SUCH_INTERFACE;

    if (NULL == mem_intf->inet_rif_p)
        return NETCFG_TYPE_FAIL;

    for (rif_config = mem_intf->inet_rif_p; rif_config; rif_config = rif_config->next)
    {
        if (rif_config->ipv4_role == NETCFG_TYPE_MODE_SECONDARY)
        {
            if(memcmp(&(secondary_rif->addr), &(rif_config->addr), sizeof(L_INET_AddrIp_T))>= 0)
            {
                continue;
            }
            else
            {
                memcpy(secondary_rif, rif_config, sizeof(NETCFG_OM_IP_InetRifConfig_T));
                return NETCFG_TYPE_OK;
            }
        }
    }

    return NETCFG_TYPE_FAIL;
}

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
UI32_T NETCFG_OM_IP_GetIpNetToMediaProxyStatus(UI32_T ifindex,BOOL_T *status)
{
    UI32_T      result = NETCFG_TYPE_FAIL;
    UI32_T      original_priority;
    NETCFG_OM_L3_Interface_T local_interface;

    if (ifindex == 0)
        return NETCFG_TYPE_INVALID_ARG;

    if(NULL == status)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    memset(&local_interface, 0, sizeof(NETCFG_OM_L3_Interface_T));
    local_interface.ifindex = ifindex;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &local_interface) == FALSE)
        result = NETCFG_TYPE_CAN_NOT_GET;
    else
    {
        if(local_interface.iftype == VLAN_L3_IP_IFTYPE)
        {
            *status = local_interface.u.physical_intf.proxy_arp_enable;
            result = NETCFG_TYPE_OK;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, original_priority);
    return  result;
}

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
UI32_T NETCFG_OM_IP_GetNextIpNetToMediaProxyStatus(UI32_T *ifindex,BOOL_T *status)
{
    UI32_T      result = NETCFG_TYPE_FAIL;
    UI32_T      original_priority;
    NETCFG_OM_L3_Interface_T local_interface;

    if(NULL == status)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    memset(&local_interface, 0, sizeof(NETCFG_OM_L3_Interface_T));
    local_interface.ifindex = *ifindex;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if(local_interface.ifindex == 0)
    {
        if(L_SORT_LST_Get_1st(&intf_list, &local_interface) == FALSE)
            result = NETCFG_TYPE_CAN_NOT_GET;
        else
        {
            if(local_interface.iftype == VLAN_L3_IP_IFTYPE)
            {
                *status = local_interface.u.physical_intf.proxy_arp_enable;
                *ifindex = local_interface.ifindex;
                result = NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        if(L_SORT_LST_Get_Next(&intf_list, &local_interface) == FALSE)
            result = NETCFG_TYPE_CAN_NOT_GET;
        else
        {
            if(local_interface.iftype == VLAN_L3_IP_IFTYPE)
            {
                *status = local_interface.u.physical_intf.proxy_arp_enable;
                *ifindex = local_interface.ifindex;
                result = NETCFG_TYPE_OK;
            }
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, original_priority);
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_IP_SetIpNetToMediaProxyStatus
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
UI32_T NETCFG_OM_IP_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status)
{
    UI32_T      result = NETCFG_TYPE_FAIL;
    UI32_T      original_priority;
    NETCFG_OM_L3_Interface_T local_interface;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;

    if (ifindex == 0)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&local_interface, 0, sizeof(NETCFG_OM_L3_Interface_T));
    local_interface.ifindex = ifindex;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &local_interface);
    if (NULL == mem_intf)
    {
        result = NETCFG_TYPE_CAN_NOT_SET;
    }
    else
    {
        if( VLAN_L3_IP_IFTYPE == mem_intf->iftype)
        {
            mem_intf->u.physical_intf.proxy_arp_enable= status;
            result = NETCFG_TYPE_OK;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, original_priority);
    return  result;
}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

UI32_T NETCFG_OM_IP_SetDebugFlag(UI8_T debug_flag)
{
    DEBUG_FLAG = debug_flag;
    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_SetRifFlags(NETCFG_OM_IP_InetRifConfig_T *rif_p, UI32_T flags)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T *mem_rif = NULL;
    UI32_T orig_priority;


    /* BODY
     */
    if (NULL == rif_p)
        return (NETCFG_TYPE_INVALID_ARG);

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_rif = L_SORT_LST_ShMem_GetPtr(&rif_list, rif_p);
    if (NULL != mem_rif)
    {
        mem_rif->flags |= flags;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

UI32_T NETCFG_OM_IP_UnSetRifFlags(NETCFG_OM_IP_InetRifConfig_T *rif_p, UI32_T flags)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T *mem_rif = NULL;
    UI32_T orig_priority;


    /* BODY
     */
    if (NULL == rif_p)
        return (NETCFG_TYPE_INVALID_ARG);

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_rif = L_SORT_LST_ShMem_GetPtr(&rif_list, rif_p);
    if (NULL != mem_rif)
    {
        mem_rif->flags &= ~flags;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

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
UI32_T NETCFG_OM_IP_GetSystemRifNbr(UI32_T *rif_num)
{
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);
    *rif_num = rif_list.nbr_of_element;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

#if (SYS_CPNT_IPV6 == TRUE)

/* FUNCTION NAME : NETCFG_OM_IP_GetInetRifConfig
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
 *      The KEY is only ip address.
 */
UI32_T NETCFG_OM_IP_GetIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T* rif_config_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T orig_priority;
    NETCFG_OM_IP_InetRifConfig_T rif_om;

    /* BODY
     */
    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    /* are those fields enough ? */
    rif_om.ifindex = rif_config_p->ifindex;
    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_ShMem_Get(&rif_list, &rif_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->row_status = rif_om.row_status;
    rif_config_p->ipv6_addr_type = rif_om.ipv6_addr_type;
    rif_config_p->ipv6_addr_config_type = rif_om.ipv6_addr_config_type;
    rif_config_p->flags = rif_om.flags;
    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_GetNextIPv6RifConfig
 * PURPOSE:
 *      Get Next routing interface entry sorting by the ip address and interface index.
 *
 * INPUT:
 *      rif_config_p->addr
 *      rif_config_p->ifindex
 *
 * OUTPUT:
 *      rif_config_p
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      The KEY is ip address and interface index.
 */
UI32_T NETCFG_OM_IP_GetNextIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T orig_priority;

    /* BODY
     */
    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));
    rif_om.ifindex = rif_config_p->ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if ((L_SORT_LST_ShMem_Get_Next(&rif_list, &rif_om) == FALSE)
        || ((L_INET_ADDR_TYPE_IPV6Z != rif_om.addr.type)
            && (L_INET_ADDR_TYPE_IPV6 != rif_om.addr.type)))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_MORE_ENTRY;
    }
    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->ipv6_addr_type = rif_om.ipv6_addr_type;
    rif_config_p->ipv6_addr_config_type = rif_om.ipv6_addr_config_type;
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T rif_om, *rif_om_p1;
    NETCFG_OM_L3_Interface_T intf, *intf_p1;
    UI32_T orig_priority;
    UI32_T found = FALSE;

    /* BODY
     */
   if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    /* check l3 interface */
    memset(&intf, 0, sizeof(intf));
    intf.ifindex = rif_config_p->ifindex;
    intf_p1 = L_SORT_LST_GetPtr(&intf_list, &intf);
    /* interface not exist */
    if (NULL == intf_p1)
        return NETCFG_TYPE_FAIL;
    /* no any rif */
    if (NULL == intf_p1->inet_rif_p)
        return NETCFG_TYPE_FAIL;


    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));
    NOTEprintf("Search key vlan %ld, ip=%lx:%lx:%lx:%lx", (long)intf.ifindex,L_INET_EXPAND_IPV6(rif_om.addr.addr));
    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* check addr type */
    switch(rif_config_p->addr.type)
    {
        case L_INET_ADDR_TYPE_IPV6 :
        case L_INET_ADDR_TYPE_IPV6Z :
            break;
        case L_INET_ADDR_TYPE_UNKNOWN :
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
        default:
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            DBGprintf("invalid addr type");
            return NETCFG_TYPE_INVALID_ARG;

    }

    if(!memcmp(rif_config_p->addr.addr, ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN)) /* ipv6 addr is zero */
    {
        if((intf_p1->inet_rif_p->addr.type == L_INET_ADDR_TYPE_IPV6)
            ||  (intf_p1->inet_rif_p->addr.type == L_INET_ADDR_TYPE_IPV6Z))
        {
            found = TRUE;
            rif_om_p1 = intf_p1->inet_rif_p;
        }
    }
    else if (NETCFG_TYPE_OK == NETCFG_OM_IP_GetInterfaceInetRif(intf_p1, &rif_om, FALSE))
    {
        found = TRUE;
        rif_om_p1 = &rif_om;
    }

    if(found)
    {
        memcpy(&rif_config_p->addr, &rif_om_p1->addr, sizeof(L_INET_AddrIp_T));

        rif_config_p->ipv6_addr_type = rif_om_p1->ipv6_addr_type;
        rif_config_p->ipv6_addr_config_type = rif_om_p1->ipv6_addr_config_type;
        rif_config_p->flags = rif_om_p1->flags;
        rif_config_p->row_status = rif_om_p1->row_status;

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_FAIL;
}

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
UI32_T NETCFG_OM_IP_GetNextIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    NETCFG_OM_L3_Interface_T intf;
    BOOL_T entry_got = FALSE;
    UI32_T orig_priority;

    /* BODY
     */
    if (NULL == rif_config_p)
        return NETCFG_TYPE_INVALID_ARG;

    /* check addr type */
    switch(rif_config_p->addr.type)
    {
        case L_INET_ADDR_TYPE_UNKNOWN :
        case L_INET_ADDR_TYPE_IPV6 :
            break;
        case L_INET_ADDR_TYPE_IPV6Z :
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
        default:
            return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    memcpy(&rif_om.addr, &rif_config_p->addr, sizeof(L_INET_AddrIp_T));

    memset(&intf, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf.ifindex = rif_config_p->ifindex;
    NOTEprintf("Search key vlan %ld, ip=%lx:%lx:%lx:%lx", (long)intf.ifindex, L_INET_EXPAND_IPV6(rif_om.addr.addr));
    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* If ifindex is not specified, iterate from the first interface */
    if (0 == rif_config_p->ifindex)
        rif_config_p->ifindex = SYS_ADPT_VLAN_1_IF_INDEX_NUMBER;

    /* check in the specified interface */
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifEntryByIfindex(&rif_om, rif_config_p->ifindex))
    {
        /* check addr type */
        if((L_INET_ADDR_TYPE_IPV6 == rif_om.addr.type)
            ||(L_INET_ADDR_TYPE_IPV6Z == rif_om.addr.type))
        {
            entry_got = TRUE;
            break;
        }
    }

    /* No more rif in the given interface, try next interface */
    if(FALSE == entry_got)
    {
        //while (TRUE == NETCFG_OM_IP_GetNextInterfaceEntry(&intf))
        while(TRUE == (L_SORT_LST_Get_Next(&intf_list, &intf)))
        {
            /* Then get the first rif of the next interface */
            memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifEntryByIfindex(&rif_om, intf.ifindex))
            {
                /* check addr type */
                if((L_INET_ADDR_TYPE_IPV6 == rif_om.addr.type)
                    ||(L_INET_ADDR_TYPE_IPV6Z == rif_om.addr.type))
                {
                    entry_got = TRUE;
                    break;
                }
            }
            if(TRUE == entry_got)
                break;
        }
    }

    /* No more rif, iteration terminated */
    if (FALSE == entry_got)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    memcpy(&rif_config_p->addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

    rif_config_p->ifindex = rif_om.ifindex;
    rif_config_p->ipv6_addr_type= rif_om.ipv6_addr_type;
    rif_config_p->ipv6_addr_config_type= rif_om.ipv6_addr_config_type;
    rif_config_p->flags = rif_om.flags;
    rif_config_p->row_status = rif_om.row_status;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6EnableStatus
 * PURPOSE:
 *      CMD: "ipv6 enable", to enable/disable IPv6 processing on an interface
 *      that has not been configured with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex -- the L3 interface.
 *      status  -- status
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only set it in OM.
 */
UI32_T NETCFG_OM_IP_SetIPv6EnableStatus(UI32_T ifindex, BOOL_T status)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }
    intf.ipv6_enable = status;

    if (L_SORT_LST_Set(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }


    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *                     VAL_ipv6IpForwarding_forwarding
 *                     VAL_ipv6IpForwarding_notForwarding
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only get it in OM.
 */
UI32_T NETCFG_OM_IP_GetIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    *status_p = intf.ipv6_enable;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      To enable/disable automatic configuration of IPv6 addresses using stateless
 *      autoconfiguration on an interface and enable IPv6 processing on the interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *      status      -- enable/disable
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. If router advertisements (RAs) received on this interface have the
 *      "other configuration" flag set, then the interface will also attempt
 *      to acquire other configuration (i.e., non-address) using DHCP for IPv6.
 */
UI32_T NETCFG_OM_IP_SetIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T status)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }
    intf.ipv6_autoconf_enable = status;

    if (L_SORT_LST_Set(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }
    *status_p = intf.ipv6_autoconf_enable;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

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
 *      NETCFG_TYPE_NO_MORE_ENTRY
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetNextIPv6AddrAutoconfigEnableStatus(UI32_T *ifindex_p, BOOL_T *status_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;
    UI32_T rc;
    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = *ifindex_p;

    if (intf.ifindex == 0)
    {
        rc = L_SORT_LST_Get_1st(&intf_list, &intf);
    }
    else
    {
        rc = L_SORT_LST_Get_Next(&intf_list, &intf);
    }

    if (rc == FALSE)
    {
        /* DEBUG */
        //printf("%s, %d, return NETCFG_TYPE_NO_MORE_ENTRY\n", __FUNCTION__, __LINE__);

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_NO_MORE_ENTRY;
    }

    *ifindex_p = intf.ifindex;
    *status_p = intf.ipv6_autoconf_enable;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_GetLinkLocalRifFromInterface
 * PURPOSE:
 *      Get RIF with link local address for IPv4 or IPv6 in
 *      the specified L3 interface.
 *
 * INPUT:
 *      rif_p       -- pointer to rif.
 *
 * OUTPUT:
 *      rif_p       -- pointer to rif.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_IMPLEMENT
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      1. rif_p->addr.type should be L_INET_ADDR_TYPE_IPV4Z or L_INET_ADDR_TYPE_IPV6Z
 */
UI32_T NETCFG_OM_IP_GetLinkLocalRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_InetRifConfig_T local;

    /* BODY
     */


    // peter remove for deadlock with NETCFG_OM_IP_GetNextInetRifOfInterface, orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* copy addr.type */
    memcpy(&local, rif_p, sizeof(NETCFG_TYPE_InetRifConfig_T));

    switch(local.addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            printf("not implement yet!\r\n");
            return NETCFG_TYPE_NOT_IMPLEMENT;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        default:
            /* set addr to FE80:: , then get next and check if link-local */
            local.addr.addr[0] = 0xFE;
            local.addr.addr[1] = 0x80;

            if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&local))
            {
                /* check if it is a link-local */
                if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(local.addr.addr))
                {
                    memcpy(rif_p, &local, sizeof(NETCFG_TYPE_InetRifConfig_T));
                    return NETCFG_TYPE_OK;
                }
            }
            return NETCFG_TYPE_ENTRY_NOT_EXIST;
            break;
    }
}

UI32_T NETCFG_OM_IP_GetL3IntfIndex(UI32_T ifindex, BOOL_T is_tunnel,  UI32_T* l3_intf_index_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf_om;
    UI32_T orig_priority;

    /* BODY
     */
    if (0 == ifindex)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&intf_om, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf_om.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &intf_om) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    *l3_intf_index_p = intf_om.drv_l3_intf_index;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6InterfaceMTU
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
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetIPv6InterfaceMTU(UI32_T ifindex, UI32_T mtu)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }
    /* set config flag */
    SET_FLAG(intf.config, NETCFG_OM_IP_CONFIG_MTU6);

    if((intf.iftype == VLAN_L3_IP_IFTYPE)||
       (intf.iftype == VLAN_L3_IP_TUNNELTYPE) )
    {
        intf.u.physical_intf.mtu6 = mtu;
    }
    else
        return NETCFG_TYPE_FAIL;

    if (L_SORT_LST_Set(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }


    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_IP_UnsetIPv6InterfaceMTU
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
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_UnsetIPv6InterfaceMTU(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }
    /* unset config flag */
    UNSET_FLAG(intf.config, NETCFG_OM_IP_CONFIG_MTU6);

    if(intf.iftype == VLAN_L3_IP_IFTYPE)
    {
        intf.u.physical_intf.mtu6 = SYS_DFLT_IPV6_INTERFACE_MTU;
    }
    else
        return NETCFG_TYPE_FAIL;

    if (L_SORT_LST_Set(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }


    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU from OM.
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
UI32_T NETCFG_OM_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf;

    /* BODY
     */

    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    intf.ifindex = ifindex;

    if (L_SORT_LST_Get(&intf_list, &intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }
    if((intf.iftype == VLAN_L3_IP_IFTYPE)||
       (intf.iftype == VLAN_L3_IP_TUNNELTYPE))
    {
        *mtu_p = intf.u.physical_intf.mtu6;
    }
    else
        return NETCFG_TYPE_FAIL;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}

UI32_T NETCFG_OM_IP_Debug_ShowInetRifOfInterface(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T intf_om;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;

    NETCFG_OM_IP_InetRifConfig_T *rif_cur_node;
    NETCFG_OM_IP_InetRifConfig_T *result_node;
    UI32_T orig_priority;
    char addr_buf[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    //int ret = 0;

    result_node = NULL;


    /* BODY
     */
    if (0 == ifindex)
        return NETCFG_TYPE_INVALID_ARG;


    /* DEBUG */
    printf("%s, %d, ifindex: %ld\n", __FUNCTION__, __LINE__, (long)ifindex);

    memset(&intf_om, 0, sizeof(NETCFG_OM_L3_Interface_T));
    intf_om.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &intf_om);

    /* DEBUG */
    printf("%s, %d, mem_intf: %p\n", __FUNCTION__, __LINE__, mem_intf);

    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    /* DEBUG */
    printf("%s, %d, mem_intf->inet_rif_p: %p\n", __FUNCTION__, __LINE__, mem_intf->inet_rif_p);

    if (NULL == mem_intf->inet_rif_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_FAIL;
    }

    for (rif_cur_node = mem_intf->inet_rif_p; rif_cur_node ; rif_cur_node = rif_cur_node->next)
    {
        memset(addr_buf, 0, sizeof(addr_buf));
        L_INET_InaddrToString((L_INET_Addr_T *)&rif_cur_node->addr, addr_buf, sizeof(addr_buf));
        printf("%s\n", addr_buf);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetIPv6StatBaseCntr(IPAL_Ipv6Statistics_T *ip6stat_base_p)
{
    UI32_T  ret = NETCFG_TYPE_FAIL;

    if (NULL != ip6stat_base_p)
    {
        memcpy (ip6stat_base_p, &ip6stat_base, sizeof (ip6stat_base));
        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

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
UI32_T NETCFG_OM_IP_SetIPv6StatBaseCntr(IPAL_Ipv6Statistics_T *ip6stat_base_p)
{
    UI32_T  ret = NETCFG_TYPE_FAIL;

    if (NULL != ip6stat_base_p)
    {
        memcpy (&ip6stat_base, ip6stat_base_p, sizeof (ip6stat_base));
        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

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
UI32_T NETCFG_OM_IP_GetICMPv6StatBaseCntr(IPAL_Icmpv6Statistics_T *icmp6stat_base_p)
{
    UI32_T  ret = NETCFG_TYPE_FAIL;

    if (NULL != icmp6stat_base_p)
    {
        memcpy (icmp6stat_base_p, &icmp6stat_base, sizeof (icmp6stat_base));
        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

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
UI32_T NETCFG_OM_IP_SetICMPv6StatBaseCntr(IPAL_Icmpv6Statistics_T *icmp6stat_base_p)
{
    UI32_T  ret = NETCFG_TYPE_FAIL;

    if (NULL != icmp6stat_base_p)
    {
        memcpy (&icmp6stat_base, icmp6stat_base_p, sizeof (icmp6stat_base));
        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

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
UI32_T NETCFG_OM_IP_GetUDPv6StatBaseCntr(IPAL_Udpv6Statistics_T *udp6stat_base_p)
{
    UI32_T  ret = NETCFG_TYPE_FAIL;

    if (NULL != udp6stat_base_p)
    {
        memcpy (udp6stat_base_p, &udp6stat_base, sizeof (udp6stat_base));
        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

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
UI32_T NETCFG_OM_IP_SetUDPv6StatBaseCntr(IPAL_Udpv6Statistics_T *udp6stat_base_p)
{
    UI32_T  ret = NETCFG_TYPE_FAIL;

    if (NULL != udp6stat_base_p)
    {
        memcpy (&udp6stat_base, udp6stat_base_p, sizeof (udp6stat_base));
        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

#endif /* SYS_CPNT_IPV6 */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
UI32_T NETCFG_OM_IP_CheckAddressOverlap_Craft(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p)
{
    UI32_T orig_priority;
    NETCFG_OM_IP_InetRifConfig_T exist_rif;
    int overlapped = 0;

    /* check craft overlape */
    memset(&exist_rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    /* Traverse all rif entries */
    while(L_SORT_LST_ShMem_Get_Next(&rif_list, &exist_rif) == TRUE)
    {
        if (craft_addr_p->addr.preflen < exist_rif.addr.preflen)
            overlapped = IP_LIB_IsIpBelongToSubnet(craft_addr_p->addr.addr, craft_addr_p->addr.preflen, exist_rif.addr.addr);
        else
            overlapped = IP_LIB_IsIpBelongToSubnet(exist_rif.addr.addr, exist_rif.addr.preflen, craft_addr_p->addr.addr);

        if (overlapped)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
            return NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

UI32_T NETCFG_OM_IP_SetCraftInterfaceValue(UI32_T field, UI32_T len, void *value_p)
{

    switch(field)
    {
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR:
            memcpy(&craft_interface.craft_if_ipv4_address, value_p, len);
            return NETCFG_TYPE_OK;
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE:
            memcpy(&craft_interface.craft_if_ipv6_enable, value_p, len);
            return NETCFG_TYPE_OK;
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK:
            memcpy(&craft_interface.craft_if_ipv6_address_link, value_p, len);
            return NETCFG_TYPE_OK;
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL:
            memcpy(&craft_interface.craft_if_ipv6_address_global, value_p, len);
            return NETCFG_TYPE_OK;
        default:
            break;
    }

    return NETCFG_TYPE_FAIL;

}

UI32_T NETCFG_OM_IP_GetCraftInterfaceValue(UI32_T field, UI32_T len, void *value_p)
{

    switch(field)
    {
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR:
            if(craft_interface.craft_if_ipv4_address.ifindex)
            {
                memcpy(value_p, &craft_interface.craft_if_ipv4_address, len);
                return NETCFG_TYPE_OK;
            }
            else
                return NETCFG_TYPE_FAIL;
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE:
            memcpy(value_p, &craft_interface.craft_if_ipv6_enable, len);
            return NETCFG_TYPE_OK;
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK:
            if(craft_interface.craft_if_ipv6_address_link.ifindex)
            {
                memcpy(value_p, &craft_interface.craft_if_ipv6_address_link, len);
                return NETCFG_TYPE_OK;
            }
            else
                return NETCFG_TYPE_FAIL;
        case NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL:
            if(craft_interface.craft_if_ipv6_address_global.ifindex)
            {
                memcpy(value_p, &craft_interface.craft_if_ipv6_address_global, len);
                return NETCFG_TYPE_OK;
            }
            else
                return NETCFG_TYPE_FAIL;
        default:
            break;
    }

    return NETCFG_TYPE_FAIL;
}

#if 0
UI32_T NETCFG_OM_IP_SetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p)
{
    switch(craft_addr_p->row_status)
    {
        case VAL_netConfigStatus_2_createAndGo:
            memcpy(&craft_if_ipv4_address, craft_addr_p, sizeof(craft_if_ipv4_address));
            craft_if_ipv4_address.row_status = VAL_netConfigStatus_2_active;
            return NETCFG_TYPE_OK;
        case VAL_netConfigStatus_2_destroy:
            if((craft_if_ipv4_address.ifindex == craft_addr_p->ifindex)
            && !memcmp(&(craft_if_ipv4_address.addr), &(craft_addr_p->addr), sizeof(craft_if_ipv4_address.addr)))
            {
                memset(&craft_if_ipv4_address, 0, sizeof(craft_if_ipv4_address));
                return NETCFG_TYPE_OK;
            }
            else
                return NETCFG_TYPE_FAIL;


        default: /* not support */
            return NETCFG_TYPE_FAIL;
    }
}
#endif

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
UI32_T NETCFG_OM_IP_GetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p)
{
    UI32_T orig_priority;

    if (NULL == craft_addr_p)
        return NETCFG_TYPE_INVALID_ARG;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    switch(craft_addr_p->addr.type)
    {
        case L_INET_ADDR_TYPE_IPV6:
            if(craft_interface.craft_if_ipv6_address_global.ifindex !=0)
                memcpy(craft_addr_p, &(craft_interface.craft_if_ipv6_address_global), sizeof(*craft_addr_p));
            else
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_ENTRY_NOT_EXIST;
            }
            break;
        case L_INET_ADDR_TYPE_IPV6Z:
            if(craft_interface.craft_if_ipv6_address_link.ifindex !=0)
                memcpy(craft_addr_p, &(craft_interface.craft_if_ipv6_address_link), sizeof(*craft_addr_p));
            else
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_ENTRY_NOT_EXIST;
            }
            break;
        default:
            if(craft_interface.craft_if_ipv4_address.ifindex !=0)
                memcpy(craft_addr_p, &(craft_interface.craft_if_ipv4_address), sizeof(*craft_addr_p));
            else
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
                return NETCFG_TYPE_ENTRY_NOT_EXIST;
            }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}


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
UI32_T NETCFG_OM_IP_GetIPv6EnableStatus_Craft(UI32_T ifindex, BOOL_T *status_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */

    /* BODY
     */
    UI32_T orig_priority;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    *status_p = craft_interface.craft_if_ipv6_enable;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);

    return NETCFG_TYPE_OK;
}
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
UI32_T NETCFG_OM_IP_SetDhcpInform(UI32_T ifindex, BOOL_T do_enable)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T entry;
    NETCFG_OM_L3_Interface_T *mem_intf = NULL;
    UI32_T orig_priority;

    /* BODY
     */
    memset(&entry, 0, sizeof(NETCFG_OM_L3_Interface_T));
    entry.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    mem_intf = L_SORT_LST_GetPtr(&intf_list, &entry);
    if (NULL == mem_intf)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    /* Update address mode of related interface */
    mem_intf->dhcp_inform = do_enable;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

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
UI32_T NETCFG_OM_IP_GetDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_L3_Interface_T entry;
    UI32_T orig_priority;

    /* BODY
     */
    if(NULL == do_enable_p)
    {
        return (NETCFG_TYPE_INVALID_ARG);
    }

    memset(&entry, 0, sizeof(NETCFG_OM_L3_Interface_T));
    entry.ifindex = ifindex;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(ip_om_sem_id);

    if (L_SORT_LST_Get(&intf_list, &entry) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    *do_enable_p = entry.dhcp_inform;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ip_om_sem_id, orig_priority);
    return NETCFG_TYPE_OK;
}

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
SYS_TYPE_Get_Running_Cfg_T NETCFG_OM_IP_GetRunningDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */

    /* BODY
     */
    if(NULL == do_enable_p)
    {
        return (SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetDhcpInform(ifindex, do_enable_p))
    {
        if(SYS_DFLT_DHCP_INFORM != (*do_enable_p))
        {
            return (SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        }
        else
        {
            return (SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        }
    }
    else
    {
        return (SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

}
#endif /* SYS_CPNT_DHCP_INFORM*/

#if (SYS_CPNT_VIRTUAL_IP == TRUE)
UI32_T NETCFG_OM_IP_GetVirtualRifByIpaddr(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetRifFromExactIp(rif_config_p))
    {
        if(NETCFG_TYPE_MODE_VIRTUAL == rif_config_p->ipv4_role)
        {
            return NETCFG_TYPE_OK;
        }
    }
    return NETCFG_TYPE_FAIL;
}

UI32_T NETCFG_OM_IP_GetNextVirtualRifByIfindex(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(rif_config_p))
    {
        if(  (rif_config_p->addr.type != L_INET_ADDR_TYPE_IPV4)
           &&(rif_config_p->addr.type != L_INET_ADDR_TYPE_IPV4Z))
            continue;

        if(NETCFG_TYPE_MODE_VIRTUAL == rif_config_p->ipv4_role)
        {
            return NETCFG_TYPE_OK;
        }
    }
    return NETCFG_TYPE_FAIL;
}


#endif
