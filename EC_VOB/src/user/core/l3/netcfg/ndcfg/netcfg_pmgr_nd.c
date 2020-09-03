/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_ND.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_MGR_ND and NETCFG_OM_ARP service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_PMGR_XXX for APIs NETCFG_MGR_XXX provided by NETCFG
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "netcfg_mgr_nd.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_nd.h"/*for convet fun.*/







/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
/***************************************************************/
static UI32_T DEBUG_FLAG = 0; //DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) if((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)!=0){printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args);}
#define INFOprintf(format,args...) if((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)!=0){printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args);}
#define NOTEprintf(format,args...) if((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)!=0){printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args);}

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







static SYSFUN_MsgQ_T ipcmsgq_handle;


#define NETCFG_PMGR_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    NETCFG_MGR_ND_IPCMsg_T *msg_p = (NETCFG_MGR_ND_IPCMsg_T*)msgbuf_p->msg_buf;

#define NETCFG_PMGR_SEND_WAIT_MSG_P(result) \
do{\
    msgbuf_p->cmd = SYS_MODULE_NDCFG;\
    msgbuf_p->msg_size = msg_size;\
    result = SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,msg_size, msgbuf_p);\
}while(0)

 #define SN_LAN                          1


#define NETCFG_PMGR_COMPOSEINETADDR(type,addrp,prefixLen) NETCFG_PMGR_ComposeInetAddr(type, addrp,prefixLen) /*temp comp fun, maybe replaced by library func??*/
static L_INET_AddrIp_T NETCFG_PMGR_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen)
{
    L_INET_AddrIp_T addr;
    memset(&addr,0,sizeof(addr));
    addr.type = type;
    if(L_INET_ADDR_TYPE_IPV4 == type || L_INET_ADDR_TYPE_IPV4Z==type)
        addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    else if(L_INET_ADDR_TYPE_IPV6 == type || L_INET_ADDR_TYPE_IPV6 == type)
        addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    else{printf ("Wooop! something wring!\r\n");}
    memcpy(addr.addr,addrp,addr.addrlen);
    addr.preflen = prefixLen;
    return addr;
}



/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_ND in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_InitiateProcessResource(void)/*init in netcfg_main*/
{
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next ARP entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_entry)
    UI32_T result;
    NETCFG_TYPE_IpNetToPhysicalEntry_T* entryp ;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTIPV4NDENTRY;
    entryp = &msg_p->data.arg_nd_entry;
    memset(entryp, 0, sizeof(NETCFG_TYPE_IpNetToPhysicalEntry_T));

    NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical(entryp, entry);
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia( entry,entryp);


    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set ARP timeout.
 *
 * INPUT   : age_time.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetIpNetToMediaTimeout(UI32_T age_time)
{
    const UI32_T msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ND_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETTIMEOUT;
    msg_p->data.ui32_v = age_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_PMGR_ND_AddStaticIpNetToMediaEntry(UI32_T vid_ifindex,
                                                 UI32_T ip_addr,
                                                 UI32_T phy_addr_len,
                                                 UI8_T *phy_addr)
{
    L_INET_AddrIp_T inet_addr = NETCFG_PMGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr, 0);
    NETCFG_TYPE_PhysAddress_T netcfg_phy_entry;

    netcfg_phy_entry.phy_address_type = SN_LAN;
    netcfg_phy_entry.phy_address_len = phy_addr_len;
    memcpy(netcfg_phy_entry.phy_address_cctet_string,phy_addr,phy_addr_len);
    return NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(vid_ifindex, &inet_addr,&netcfg_phy_entry);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifindex, UI32_T ip_addr)
{

    L_INET_AddrIp_T inet_addr = NETCFG_PMGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr, 0);

    inet_addr.type = L_INET_ADDR_TYPE_IPV4;
    memcpy(inet_addr.addr,&ip_addr,SYS_ADPT_IPV4_ADDR_LEN);

    return NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(vid_ifindex, &inet_addr);

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToMediaEntry(UI32_T vid_ifindex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr)
{
    L_INET_AddrIp_T inet_addr = NETCFG_PMGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr, 0);
    NETCFG_TYPE_PhysAddress_T netcfg_phy_entry;

    netcfg_phy_entry.phy_address_type = SN_LAN;
    netcfg_phy_entry.phy_address_len = phy_addr_len;
    memcpy(netcfg_phy_entry.phy_address_cctet_string,phy_addr,phy_addr_len);

    return NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(vid_ifindex,
                                                    &inet_addr,&netcfg_phy_entry);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Remove a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry(UI32_T vid_ifindex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr)
{
    L_INET_AddrIp_T inet_addr = NETCFG_PMGR_COMPOSEINETADDR(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ip_addr, 0);
    NETCFG_TYPE_PhysAddress_T netcfg_phy_entry;

    netcfg_phy_entry.phy_address_type = SN_LAN;
    netcfg_phy_entry.phy_address_len = phy_addr_len;
    memcpy(netcfg_phy_entry.phy_address_cctet_string,phy_addr,phy_addr_len);

    return NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(vid_ifindex,
                                                    &inet_addr,&netcfg_phy_entry);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetStatistics
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ARP packet statistics.
 *
 * INPUT   : None.
 *
 * OUTPUT  : stat.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat)
{
    const UI32_T msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_stat);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ND_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETSTATISTICS;

    memset(&(msg_p->data.arg_nd_stat), 0, sizeof(msg_p->data.arg_nd_stat));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(stat, &(msg_p->data.arg_nd_stat),sizeof(msg_p->data.arg_nd_stat));

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ARP entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_ND_GET_MSG_SIZE(arg_nd_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ND_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NDCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ND_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNDENTRY;

    memset(&msg_p->data.arg_nd_entry, 0, sizeof(msg_p->data.arg_nd_entry));

    msg_p->data.arg_nd_entry.ip_net_to_physical_if_index = entry->ip_net_to_media_if_index;
    msg_p->data.arg_nd_entry.ip_net_to_physical_phys_address = entry->ip_net_to_media_phys_address;
    msg_p->data.arg_nd_entry.ip_net_to_physical_net_address.type = L_INET_ADDR_TYPE_IPV4;
    memcpy(msg_p->data.arg_nd_entry.ip_net_to_physical_net_address.addr,&entry->ip_net_to_media_net_address,SYS_ADPT_IPV4_ADDR_LEN);



    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    entry->ip_net_to_media_if_index = msg_p->data.arg_nd_entry.ip_net_to_physical_if_index;
    entry->ip_net_to_media_phys_address = msg_p->data.arg_nd_entry.ip_net_to_physical_phys_address;
    memcpy(&entry->ip_net_to_media_net_address,msg_p->data.arg_nd_entry.ip_net_to_physical_net_address.addr,SYS_ADPT_IPV4_ADDR_LEN);
    entry->ip_net_to_media_type = msg_p->data.arg_nd_entry.ip_net_to_physical_type;


    return msg_p->type.result_bool;

}


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static ARP entry invalid or valid.
 *
 * INPUT   : entry, type.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry, int type)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_static_entry_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETSTATICND;

    NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical(&msg_p->data.arg_static_entry_and_ui32.static_entry.ip_net_to_physical_entry,&entry->ip_net_to_media_entry);
    msg_p->data.arg_static_entry_and_ui32.static_entry.status = entry->status;
    msg_p->data.arg_static_entry_and_ui32.ui32_v = type;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ARP timeout.
 *
 * INPUT   : None.
 *
 * OUTPUT  : age_time.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningIpNetToMediaTimeout(UI32_T *age_time)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v);
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGTIMEOUT;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *age_time = msg_p->data.ui32_v;
    return msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static ARP entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_static_entry)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV4NDENTRY;
    NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical(&msg_p->data.arg_nd_static_entry.ip_net_to_physical_entry, &entry->ip_net_to_media_entry);
    msg_p->data.arg_nd_static_entry.status = entry->status;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);

    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia(&entry->ip_net_to_media_entry, &msg_p->data.arg_nd_static_entry.ip_net_to_physical_entry);
    entry->status = msg_p->data.arg_nd_static_entry.status;

    return msg_p->type.result_ui32;
}


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static NIEGHBOR entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_config)
    UI32_T result;


    msg_p->type.cmd = NETCFG_MGR_ND_IPC_CREATESTATIC;
    msg_p->data.arg_nd_config.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_config.ip_addr = * ip_addr;
    msg_p->data.arg_nd_config.phy_addr = *phy_addr;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static NIEGHBOR entry.
 *
 * INPUT   : vid_ifindex, L_INET_AddrIp_T.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(UI32_T vid_ifindex, L_INET_AddrIp_T* ip_addr)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_config);
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_DELETESTATIC;
    msg_p->data.arg_nd_config.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_config.ip_addr = * ip_addr;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_config)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_ADDROUTERREDUNDANCYPROTOCOLENTRY;
    msg_p->data.arg_nd_config.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_config.ip_addr = * ip_addr;
    msg_p->data.arg_nd_config.phy_addr = *phy_addr;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Remove a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_config);
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_DELETEROUTERREDUNDANCYPROTOCOLENTRY;
    msg_p->data.arg_nd_config.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_config.ip_addr = * ip_addr;
    msg_p->data.arg_nd_config.phy_addr = *phy_addr;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an NIEGHBOR entry information.
 *
 * INPUT   : entry. key is  ip_net_to_media_phys_address
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_GetIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_entry);
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNDENTRY;
    msg_p->data.arg_nd_entry = *entry;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return FALSE;
    }
    *entry = msg_p->data.arg_nd_entry;
    return msg_p->type.result_bool;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static NIEGHBOR entry invalid or valid.
 *
 * INPUT   : entry,
 *                  type should be one of follow type:.
 *                            VAL_ipNetToMediaType_invalid
 *                            VAL_ipNetToMediaType_static
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_PMGR_ND_SetStaticIpNetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry, int type)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_static_entry_and_ui32);
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETSTATICND;
    msg_p->data.arg_static_entry_and_ui32.static_entry = *entry;
    msg_p->data.arg_static_entry_and_ui32.ui32_v = type;


    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static NIEGHBOR entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextStaticIpNetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_static_entry)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTSTATICNDENTRY;
    msg_p->data.arg_nd_static_entry = *entry;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *entry = msg_p->data.arg_nd_static_entry;

    return msg_p->type.result_ui32;
}
UI32_T NETCFG_PMGR_ND_GetNextStaticIpv4NetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_static_entry)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV4NDENTRY;
    msg_p->data.arg_nd_static_entry = *entry;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *entry = msg_p->data.arg_nd_static_entry;

    return msg_p->type.result_ui32;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next NIEGHBOR entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_PMGR_ND_GetNextIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_entry)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTNDENTRY;
    msg_p->data.arg_nd_entry = *entry;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *entry = msg_p->data.arg_nd_entry;
    return msg_p->type.result_ui32;

}
UI32_T NETCFG_PMGR_ND_GetNextIpv4NetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_entry)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTIPV4NDENTRY;
    msg_p->data.arg_nd_entry = *entry;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *entry = msg_p->data.arg_nd_entry;
    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
 UI32_T NETCFG_PMGR_ND_DeleteAllDynamicIpNetToMediaEntry(void)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_DELETEALLDYNAMIC;
    msg_p->data.ui32_v = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
UI32_T NETCFG_PMGR_ND_DeleteAllDynamicIpv4NetToMediaEntry(void)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_DELETEALLDYNAMICIPV4;
    msg_p->data.ui32_v = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

#if (SYS_CPNT_IPV6 == TRUE)

 UI32_T NETCFG_PMGR_ND_GetNextStaticIpv6NetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_static_entry)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTSTATICIPV6NDENTRY;
    msg_p->data.arg_nd_static_entry = *entry;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *entry = msg_p->data.arg_nd_static_entry;

    return msg_p->type.result_ui32;
}
UI32_T NETCFG_PMGR_ND_GetNextIpv6NetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_entry)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETNEXTIPV6NDENTRY;
    msg_p->data.arg_nd_entry = *entry;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *entry = msg_p->data.arg_nd_entry;
    return msg_p->type.result_ui32;

}


UI32_T NETCFG_PMGR_ND_DeleteAllDynamicIpv6NetToMediaEntry(void)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_DELETEALLDYNAMICIPV6;
    msg_p->data.ui32_v = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdDADAttempts
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the number of consecutive neighbor solicitation messages
 *                      that are sent on an interface while duplicate address detection is performed
 *                      on the unicast IPv6 addresses of the interface
 *
 * INPUT   : UI32_T attempts :  The number of neighbor solicitation messages
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdDADAttempts(UI32_T vid_ifindex, UI32_T attempts)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32);
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETDADATTEMPTS;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = attempts;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningDADAttempts
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND DAD attempts .
 *
 * INPUT   : none .
 *
 * OUTPUT  : attempts.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningDADAttempts(UI32_T vid_ifindex, UI32_T* attempts)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGDADATTEMPTS;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *attempts;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *attempts = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_running_cfg;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdNsInterval
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the interval between IPv6 neighbor solicitation retransmissions on an interface
 *
 * INPUT   :    UI32_T vid_ifindex: interface id
 *                  UI32_T msec :  The interval between IPv6 neighbor solicit transmissions in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdNsInterval(UI32_T vid_ifindex, UI32_T msec)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDNSINTERVAL;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = msec;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
//UI32_T NETCFG_PMGR_ND_UnsetNdNsInterval(UI32_T vid_ifindex);

UI32_T NETCFG_PMGR_ND_UnsetNdNsInterval(UI32_T vid_ifindex)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_UNSETNDNSINTERVAL;
    msg_p->data.ui32_v = vid_ifindex;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdNsInterval
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND NS interval .
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : msec.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdNsInterval(UI32_T vid_ifindex, UI32_T* msec)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDNSINTERVAL;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = *msec;


    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *msec = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the amount of time that a remote IPv6 node is considered reachable
 *                  after some reachability confirmation event has occurred
 *
 * INPUT   :UI32_T vid_ifindex: interface id
 *              UI32_T msec:  amount of time to considered reachable in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdReachableTime(UI32_T vid_ifindex,UI32_T msec)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDREACHABLETIME;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = msec;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
UI32_T NETCFG_PMGR_ND_UnsetNdReachableTime(UI32_T vid_ifindex)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_UNSETNDREACHABLETIME;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND reachalbe time by vlan .
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : msec.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdReachableTime(UI32_T vid_ifindex, UI32_T* msec)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDREACHABLETIME;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = * msec;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *msec = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_running_cfg;
}

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdHoplimit
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the maximum number of hops used in router advertisements
 *
 * INPUT   : hoplimit :
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdHoplimit(UI32_T hoplimit)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDHOPLIMIT;
    msg_p->data.ui32_v = hoplimit;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result) ;
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    return msg_p->type.result_ui32;
}
UI32_T NETCFG_PMGR_ND_UnsetNdHoplimit()
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_UNSETNDHOPLIMIT;
    msg_p->data.ui32_v = 0;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result) ;
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    return msg_p->type.result_ui32;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdHoplimit
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND hoplimit .
 *
 * INPUT   : none .
 *
 * OUTPUT  : hoplimit.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdHoplimit(UI32_T* hoplimit)
{
    NETCFG_PMGR_DECLARE_MSG_P(ui32_v)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDHOPLIMIT;
    msg_p->data.ui32_v = *hoplimit;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *hoplimit = msg_p->data.ui32_v;
    return msg_p->type.result_running_cfg;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :configure which IPv6 prefixes are included in IPv6 router advertisements
 *
 * INPUT   :UI32_T vid_ifindex : interface id
 *              L_INET_AddrIp_T prefix :   Configure IPv6 prefixes in router advertisements
 *              UI32_T validLifetime : The amount of time (in seconds)
 *                                                  that the specified IPv6 prefix is advertised as being valid
 *
 *              UI32_T preferredLifetime: The amount of time (in seconds)
 *                                                      that the specified IPv6 prefix is advertised as being preferred.
 *              BOOL_T enable_onlink : When FALSE, means do not use this prefix for onlink determination
 *              BOOL_T enable_autoconf:When FALSE, means do not use this prefix for autoconfiguration

 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T *prefix, UI32_T validLifetime, UI32_T preferredLifetime,BOOL_T enable_onlink,BOOL_T enable_autoconf)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_prefix);
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDPREFIX;
    msg_p->data.arg_nd_prefix.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_prefix.prefix = *prefix;
    msg_p->data.arg_nd_prefix.valid_lifetime = validLifetime;
    msg_p->data.arg_nd_prefix.preferred_lifetime = preferredLifetime;
    msg_p->data.arg_nd_prefix.enable_onlink = enable_onlink;
    msg_p->data.arg_nd_prefix.enable_autoconf = enable_autoconf;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
UI32_T NETCFG_PMGR_ND_UnsetNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix )
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_prefix);
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_UNSETNDPREFIX;
    msg_p->data.arg_nd_prefix.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_prefix.prefix = *prefix;
    msg_p->data.arg_nd_prefix.valid_lifetime = 0;
    msg_p->data.arg_nd_prefix.preferred_lifetime = 0;
    msg_p->data.arg_nd_prefix.enable_onlink = 0;
    msg_p->data.arg_nd_prefix.enable_autoconf = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND prefix by vlan
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : prefix + validLifetime +preferredLifetime + offLink +noAutoconfig
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetNextRunningNdPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T*enable_onlink,BOOL_T* enable_autoconf)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_nd_prefix)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNEXTNDPREFIX;
    msg_p->data.arg_nd_prefix.vid_ifindex = vid_ifindex;
    msg_p->data.arg_nd_prefix.prefix = *prefix;
    msg_p->data.arg_nd_prefix.valid_lifetime = *validLifetime;
    msg_p->data.arg_nd_prefix.preferred_lifetime = *preferredLifetime;
    msg_p->data.arg_nd_prefix.enable_onlink = *enable_onlink;
    msg_p->data.arg_nd_prefix.enable_autoconf = *enable_autoconf;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *prefix = msg_p->data.arg_nd_prefix.prefix ;
    *validLifetime= msg_p->data.arg_nd_prefix.valid_lifetime ;
    *preferredLifetime = msg_p->data.arg_nd_prefix.preferred_lifetime ;
    *enable_onlink = msg_p->data.arg_nd_prefix.enable_onlink;
    * enable_autoconf = msg_p->data.arg_nd_prefix.enable_autoconf;

    return  msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdManagedConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :To set the "managed address configuration" flag in IPv6 router advertisements
 *
 * INPUT   :UI32_T vid_ifindex: interface id
 *              BOOL_T enableFlag:  when TRUE means  flag on, when FLASE means flag off.
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T enableFlag)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDMANAGEDCONFIG;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = enableFlag;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdManagedConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND managed flag by vlan .
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : enableFlag.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdManagedConfigFlag(UI32_T vid_ifindex, BOOL_T* enableFlag)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDMANAGEDCONFIG;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = * enableFlag;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result!= SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *enableFlag = msg_p->data.arg_ifindex_and_bool.bool_v;
    return msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdOtherConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :set the "other stateful configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: interface id
 *                  BOOL_T enableFlag:  when TRUE means  flag on, when FLASE means flag off.
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T enableFlag)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDOTHERCONFIG;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = enableFlag;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result) ;
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdOtherConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND other flag by vlan .
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : enableFlag.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdOtherConfigFlag(UI32_T vid_ifindex, BOOL_T* enableFlag)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDOTHERCONFIG;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = * enableFlag;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *enableFlag = msg_p->data.arg_ifindex_and_bool.bool_v;
    return msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaSuppress
 *-----------------------------------------------------------------------------
 * PURPOSE : suppress IPv6 router advertisement transmissions
 *
 * INPUT   :UI32_T vid_ifindex: interface
 *              BOOL_T enableSuppress:  when TRUE means  supress , when FLASE means not suppress.
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRaSuppress(UI32_T vid_ifindex, BOOL_T enableSuppress)
{

    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDRASUPPRESS;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v= enableSuppress;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result) ;
    if (result!= SYSFUN_OK)
    {
        DBGprintf("Fail to send IPC, return %d",result);
        return NETCFG_TYPE_FAIL;
    }
    return msg_p->type.result_ui32;


}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaSuppress
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND RA supress by vlan .
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : msec.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRaSuppress(UI32_T vid_ifindex, BOOL_T* enableSuppress)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDRASUPPRESS;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_bool.bool_v = * enableSuppress;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *enableSuppress = msg_p->data.arg_ifindex_and_bool.bool_v;
    return msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the router lifetime value in IPv6 router advertisements
 *
 * INPUT   :UI32_T vid_ifindex: interface
 *              UI32_T seconds:  router lifetime in seconds
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRaLifetime(UI32_T vid_ifindex, UI32_T seconds)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    NOTEprintf("vid=%d, seconds=%d", vid_ifindex, seconds);
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDRALIFETIME;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = seconds;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
UI32_T NETCFG_PMGR_ND_UnsetNdRaLifetime(UI32_T vid_ifindex)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    NOTEprintf("vid=%d", vid_ifindex);
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_UNSETNDRALIFETIME;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND RA lifetime
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : seconds.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRaLifetime(UI32_T vid_ifindex, UI32_T* seconds)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDRALIFETIME;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = * seconds;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *seconds = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_running_cfg;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between IPv6 router advertisement  transmissions
 *
 * INPUT   :vid_ifindex: interface ifindex
 *          max:  max router advertisement interval in seconds
 *          min:  min router advertisement interval in seconds
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK
 *           NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRaInterval(UI32_T vid_ifindex, UI32_T max, UI32_T min)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32x2)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDRAINTERVAL;
    msg_p->data.arg_ifindex_and_ui32x2.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32x2.ui32_1_v = max;
    msg_p->data.arg_ifindex_and_ui32x2.ui32_2_v = min;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
UI32_T NETCFG_PMGR_ND_UnsetNdRaInterval(UI32_T vid_ifindex)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_UNSETNDRAINTERVAL;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = 0;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND RA interval
 *
 * INPUT   : vid_ifindex -- vlan ifindex
 *
 * OUTPUT  : max_p -- max ra interval
 *           min_p -- min ra interval
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRaInterval(UI32_T vid_ifindex, UI32_T *max_p, UI32_T* min_p)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32x2)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDRAINTERVAL;
    msg_p->data.arg_ifindex_and_ui32x2.vid_ifindex = vid_ifindex;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *max_p = msg_p->data.arg_ifindex_and_ui32x2.ui32_1_v;
    *min_p = msg_p->data.arg_ifindex_and_ui32x2.ui32_2_v;

    return msg_p->type.result_running_cfg;
}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between IPv6 router advertisement  transmissions
 *
 * INPUT   :UI32_T vid_ifindex: interface
 *          NETCFG_RA_ROUTER_PREFERENCE_T perference: default route preference, possible value are one of:
 *                  {NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH,
 *                   NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM,
 *                   NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW}
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK
 *           NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRouterPreference(UI32_T vid_ifindex, UI32_T  prefer)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;
    NOTEprintf("vid=%d, prefer=%d",vid_ifindex,prefer );
    msg_p->type.cmd = NETCFG_MGR_ND_IPC_SETNDROUTERPREFERENCE;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v= prefer;
    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND route preference
 *
 * INPUT   : vid_ifindex .
 *
 * OUTPUT  : NETCFG_RA_ROUTER_PREFERENCE_T prefer
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRouterPreference(UI32_T vid_ifindex, UI32_T*  prefer)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_ui32)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_GETRUNNINGNDROUTERPREFERENCE;
    msg_p->data.arg_ifindex_and_ui32.vid_ifindex = vid_ifindex;
    msg_p->data.arg_ifindex_and_ui32.ui32_v = * prefer;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result  != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *prefer = msg_p->data.arg_ifindex_and_ui32.ui32_v;
    return msg_p->type.result_running_cfg;
}

#endif /* #if (SYS_CPNT_IPV6_ROUTING == TRUE) */
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_GetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard port status for specified lport.
 * INPUT  : lport       - which lport to get
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_RAGUARD_GetPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_RAGUARD_GETPORTSTATUS;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = lport;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.result_bool)
    {
        *is_enable_p = msg_p->data.arg_ifindex_and_bool.bool_v;
    }

    return msg_p->type.result_bool;
}
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_GetNextPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard next port status for specified lport.
 * INPUT  : lport_p     - which lport to get next
 * OUTPUT : lport_p     - next lport
 *          is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_RAGUARD_GetNextPortStatus(
    UI32_T  *lport_p,
    BOOL_T  *is_enable_p)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_RAGUARD_GETNEXTPORTSTATUS;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = *lport_p;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.result_bool)
    {
        *lport_p     = msg_p->data.arg_ifindex_and_bool.vid_ifindex;
        *is_enable_p = msg_p->data.arg_ifindex_and_bool.bool_v;
    }

    return msg_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_GetRunningPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard running port status for specified lport.
 * INPUT  : lport       - which lport to get
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_ND_RAGUARD_GetRunningPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_RAGUARD_GETRUNNINGPORTSTATUS;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = lport;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == msg_p->type.result_running_cfg)
    {
        *is_enable_p = msg_p->data.arg_ifindex_and_bool.bool_v;
    }

    return msg_p->type.result_running_cfg;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_SetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To set RA Guard port status for specified lport.
 * INPUT  : lport     - which lport to set
 *          is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_RAGUARD_SetPortStatus(
    UI32_T  lport,
    BOOL_T  is_enable)
{
    NETCFG_PMGR_DECLARE_MSG_P(arg_ifindex_and_bool)
    UI32_T result;

    msg_p->type.cmd = NETCFG_MGR_ND_IPC_RAGUARD_SETPORTSTATUS;
    msg_p->data.arg_ifindex_and_bool.vid_ifindex = lport;
    msg_p->data.arg_ifindex_and_bool.bool_v      = is_enable;

    NETCFG_PMGR_SEND_WAIT_MSG_P(result);
    if (result != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

