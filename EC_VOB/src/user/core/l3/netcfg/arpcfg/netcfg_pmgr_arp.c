/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_ARP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_MGR_ARP and NETCFG_OM_ARP service.
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
#include "netcfg_mgr_arp.h"
#include "netcfg_pmgr_arp.h"

static SYSFUN_MsgQ_T ipcmsgq_handle;

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_ARP in the calling process.
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
BOOL_T NETCFG_PMGR_ARP_InitiateProcessResource(void)/*init in netcfg_main*/
{
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetNextIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_GETNEXTARPENTRY;
    memset(&(msg_p->data.arg_arp_entry), 0, sizeof(msg_p->data.arg_arp_entry));
	msg_p->data.arg_arp_entry.ip_net_to_media_if_index = entry->ip_net_to_media_if_index;
	msg_p->data.arg_arp_entry.ip_net_to_media_net_address = entry->ip_net_to_media_net_address;
	
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.arg_arp_entry),sizeof(msg_p->data.arg_arp_entry));

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_DeleteAllDynamicIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_DeleteAllDynamicIpNetToMediaEntry(void)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_DELETEALLDYNAMIC;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_SetIpNetToMediaTimeout
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
UI32_T NETCFG_PMGR_ARP_SetIpNetToMediaTimeout(UI32_T age_time)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_SETTIMEOUT;
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
 * ROUTINE NAME : NETCFG_PMGR_ARP_AddStaticIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_CREATESTATICARP;
    
    msg_p->data.arg_grp1.arg1 = vid_ifIndex;
    msg_p->data.arg_grp1.arg2 = ip_addr;
    msg_p->data.arg_grp1.arg3 = phy_addr_len;
    memcpy((msg_p->data.arg_grp1.arg4), phy_addr, phy_addr_len);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_DeleteStaticIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_DELETESTATICARP;
    
    msg_p->data.arg_grp1.arg1 = vid_ifIndex;
    msg_p->data.arg_grp1.arg2 = ip_addr;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetStatistics
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
UI32_T NETCFG_PMGR_ARP_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_stat);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_GETSTATISTICS;
    
    memset(&(msg_p->data.arg_arp_stat), 0, sizeof(msg_p->data.arg_arp_stat));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(stat, &(msg_p->data.arg_arp_stat),sizeof(msg_p->data.arg_arp_stat));

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetIpNetToMediaEntry
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
BOOL_T NETCFG_PMGR_ARP_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_GETARPENTRY;
    
    memset(&(msg_p->data.arg_arp_entry), 0, sizeof(msg_p->data.arg_arp_entry));
	msg_p->data.arg_arp_entry.ip_net_to_media_if_index = entry->ip_net_to_media_if_index;
	msg_p->data.arg_arp_entry.ip_net_to_media_net_address = entry->ip_net_to_media_net_address;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry, &(msg_p->data.arg_arp_entry),sizeof(msg_p->data.arg_arp_entry));
    return msg_p->type.result_bool;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_SetStaticIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_SetStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry, int type)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_SETSTATICARP;
    
    memcpy(&(msg_p->data.arg_grp3.arg1), entry, sizeof(msg_p->data.arg_grp3.arg1));
    msg_p->data.arg_grp3.arg2 = type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetRunningIpNetToMediaTimeout
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
NETCFG_PMGR_ARP_GetRunningIpNetToMediaTimeout(UI32_T *age_time)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_GETRUNNINGTIMEOUT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *age_time = msg_p->data.ui32_v;
    return msg_p->type.result_running_cfg;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetNextStaticIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_ARP_GET_MSG_SIZE(arg_arp_static_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ARP_IPC_GETNEXTRUNNINGSTATICARPENTRY;
    
    memcpy(&(msg_p->data.arg_arp_static_entry), entry, sizeof(msg_p->data.arg_arp_static_entry));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.arg_arp_static_entry),sizeof(msg_p->data.arg_arp_static_entry));

    return msg_p->type.result_ui32;
}

