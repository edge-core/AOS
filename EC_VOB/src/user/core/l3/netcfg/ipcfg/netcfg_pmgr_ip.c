/* MODULE NAME:  netcfg_pmgr_ip.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_MGR_IP and NETCFG_OM_IP service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    01/22/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_bld.h"
#include "sys_cpnt.h"
#include "netcfg_mgr_ip.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_type.h"
#include "sys_module.h"
#include "l_mm.h"

/* NAMING CONSTANT DECLARATIONS
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


#define NETCFG_PMGR_IP_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    NETCFG_MGR_IP_IPCMsg_T *msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf; \

#define NETCFG_PMGR_IP_SEND_WAIT_MSG_P() \
do{\
    msgbuf_p->cmd = SYS_MODULE_IPCFG;\
    msgbuf_p->msg_size = msg_size;\
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p, \
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, \
            msg_size, msgbuf_p) != SYSFUN_OK) \
    { \
        return NETCFG_TYPE_FAIL; \
    } \
}while(0)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T netcfg_ip_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_PMGR_IP_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_PMGR_IP, it should initiate 
 *    the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_PMGR_IP_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_ip_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME : NETCFG_PMGR_IP_CreateL3If
 * PURPOSE:
 *    Create a L3 interface.
 * INPUT:
 *    vid -- vlan id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_CreateL3If(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_CREATEL3IF;
    msg_p->data.ui32_v = vid;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_CreateLoopbackInterface
 * PURPOSE:
 *    Create a loopback interface.
 * INPUT:
 *    lo_id -- loopback id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_CreateLoopbackInterface(UI32_T lo_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_CREATELOOPBACKIF;
    msg_p->data.ui32_v = lo_id;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_DeleteL3If
 * PURPOSE:
 *    Delete a L3 interface.
 * INPUT:
 *    vid -- vlan id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_DeleteL3If(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_DELETEL3IF;
    msg_p->data.ui32_v = vid;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_DeleteLoopbackInterface
 * PURPOSE:
 *    Delete a Loopback interface.
 * INPUT:
 *    lo_id -- loopback id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_DeleteLoopbackInterface(UI32_T lo_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_DELETELOOPBACKIF;
    msg_p->data.ui32_v = lo_id;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_SetInetRif
 * PURPOSE:
 *      To add or delete rif. also remember the incoming type
 * INPUT:
 *      rif_p               -- pointer to rif
 *      incoming_type       -- CLI/Web      1
 *                             SNMP         2
 *                             Dynamic      3
 *      rif_p->rowStatus    -- the action on this entry, valid actions :
 *          VAL_netConfigStatus_2_active          1L
 *              make a RIF to be active.
 *          VAL_netConfigStatus_2_notInService    2L
 *          VAL_netConfigStatus_2_notReady        3L
 *              disable the circuit, to change some configurations.
 *          VAL_netConfigStatus_2_createAndGo     4L
 *              not allowed in IPCFG, by field config, could not active immediately.
 *          VAL_netConfigStatus_2_createAndWait   5L
 *              create entry and wait other fields config.
 *          VAL_netConfigStatus_2_destroy         6L
 *              disable and destroy an entry.
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
 *      None.
 */
UI32_T NETCFG_PMGR_IP_SetInetRif(NETCFG_TYPE_InetRifConfig_T *rif_p, UI32_T incoming_type)
{
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(inet_rif_config_u32a1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_SETINETRIF;

    memcpy(&(msg_p->data.inet_rif_config_u32a1.inet_rif_config), rif_p, sizeof(msg_p->data.inet_rif_config_u32a1.inet_rif_config));
    msg_p->data.inet_rif_config_u32a1.u32_a1 = incoming_type;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_PMGR_IP_SetIpAddressMode
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
 */
UI32_T  NETCFG_PMGR_IP_SetIpAddressMode(UI32_T ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_T ip_addr_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_ip_addr_mode);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_SETIPADDRESSMODE;
    msg_p->data.u32a1_ip_addr_mode.u32_a1 = ifindex;
    msg_p->data.u32a1_ip_addr_mode.ip_addr_mode = ip_addr_mode;
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus
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
UI32_T NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status)
{
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(arp_proxy_status);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_SETARPPROXYSTATUS;
    msg_p->data.arp_proxy_status.ifindex = ifindex;
    msg_p->data.arp_proxy_status.status = status;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIpNetToMediaProxyStatus
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
NETCFG_PMGR_IP_GetRunningIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T *status)
{
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(arp_proxy_status);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETRUNNINGARPPROXYSTATUS;
    msg_p->data.arp_proxy_status.ifindex = ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *status = msg_p->data.arp_proxy_status.status;
    return msg_p->type.result_running_cfg;
}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6Enable
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
UI32_T NETCFG_PMGR_IP_IPv6Enable(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6ENABLE;
    msg_p->data.ui32_v = ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6Disable
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
UI32_T NETCFG_PMGR_IP_IPv6Disable(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6DISABLE;
    msg_p->data.ui32_v = ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6AddrAutoconfigEnable
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
UI32_T NETCFG_PMGR_IP_IPv6AddrAutoconfigEnable(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6ADDRAUTOCONFIGENABLE;
    msg_p->data.ui32_v = ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_DisableIPv6AddrAutoconfig
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
UI32_T NETCFG_PMGR_IP_IPv6AddrAutoconfigDisable(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6ADDRAUTOCONFIGDISABLE;
    msg_p->data.ui32_v = ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_SetIPv6InterfaceMTU
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
UI32_T  NETCFG_PMGR_IP_SetIPv6InterfaceMTU(UI32_T ifindex, UI32_T mtu)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_a1_a2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6SETINTERFACEMTU;
    msg_p->data.ui32_a1_a2.a1 = ifindex;
    msg_p->data.ui32_a1_a2.a2 = mtu;
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_UnsetIPv6InterfaceMTU
 * PURPOSE:
 *      Set IPv6 interface MTU.
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
UI32_T  NETCFG_PMGR_IP_UnsetIPv6InterfaceMTU(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6UNSETINTERFACEMTU;
    msg_p->data.ui32_v = ifindex;
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetIPv6InterfaceMTU
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
UI32_T NETCFG_PMGR_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(ui32_a1_a2)

    msg_p->data.ui32_a1_a2.a1 = ifindex;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6GETINTERFACEMTU;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    *mtu_p = msg_p->data.ui32_a1_a2.a2;

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetNextIfJoinIpv6McastAddr
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
UI32_T NETCFG_PMGR_IP_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(u32a1_addra2)

    msg_p->data.u32a1_addra2.u32_a1 = ifindex;
    memcpy(&(msg_p->data.u32a1_addra2.addr_a2), mcaddr_p, sizeof(L_INET_AddrIp_T));
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETNEXTIFJOINIPV6MCASTADDR;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    memcpy(mcaddr_p, &(msg_p->data.u32a1_addra2.addr_a2), sizeof(L_INET_AddrIp_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetNextPMTUEntry
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
UI32_T NETCFG_PMGR_IP_GetNextPMTUEntry(NETCFG_TYPE_PMTU_Entry_T *entry_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(pmtu_entry_v)

    memcpy(&(msg_p->data.pmtu_entry_v), entry_p, sizeof(NETCFG_TYPE_PMTU_Entry_T));
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETNEXTPMTUENTRY;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    memcpy(entry_p, &(msg_p->data.pmtu_entry_v), sizeof(NETCFG_TYPE_PMTU_Entry_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetIfIpv6AddrInfo
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
UI32_T NETCFG_PMGR_IP_GetIfIpv6AddrInfo(NETCFG_TYPE_IpAddressInfoEntry_T *addr_info_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(ip_addr_info_v)

    memcpy(&(msg_p->data.ip_addr_info_v), addr_info_p, sizeof(NETCFG_TYPE_IpAddressInfoEntry_T));
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETIFIPV6ADDRINFO;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    memcpy(addr_info_p, &(msg_p->data.ip_addr_info_v), sizeof(NETCFG_TYPE_IpAddressInfoEntry_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_DhcpReleaseComplete
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
UI32_T NETCFG_PMGR_IP_DhcpReleaseComplete(UI32_T ifindex, UI32_T protocols)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(ui32_a1_a2)

    msg_p->data.ui32_a1_a2.a1 = ifindex;
    msg_p->data.ui32_a1_a2.a2 = protocols;
    
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_DHCPRELEASECOMPLETE;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIPv6EnableStatus
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
NETCFG_PMGR_IP_GetRunningIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;

    msg_p->data.u32a1_bla2.u32_a1 = ifindex;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6ENABLESTATUS;
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *status_p = msg_p->data.u32a1_bla2.bl_a2;
    
    return msg_p->type.result_running_cfg;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus
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
NETCFG_PMGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p)
{

    NETCFG_PMGR_IP_DECLARE_MSG_P(u32a1_bla2)

    msg_p->data.u32a1_bla2.u32_a1 = ifindex;    
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6ADDRAUTOCONFIGENABLESTATUS;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    *status_p = msg_p->data.u32a1_bla2.bl_a2;
    
    return msg_p->type.result_running_cfg;

}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIPv6InterfaceMTU
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
NETCFG_PMGR_IP_GetRunningIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_a1_a2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;

    msg_p->data.ui32_a1_a2.a1 = ifindex;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6INTERFACEMTU;
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *mtu_p = msg_p->data.ui32_a1_a2.a2;
    
    return msg_p->type.result_running_cfg;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetIPv6Statistics
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
UI32_T NETCFG_PMGR_IP_GetIPv6Statistics(IPAL_Ipv6Statistics_T *ip6stat_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(ip6_stat)

    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETIPV6STATISTICS;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    memcpy(ip6stat_p, &(msg_p->data.ip6_stat), sizeof(IPAL_Ipv6Statistics_T));

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetICMPv6Statistics
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
UI32_T NETCFG_PMGR_IP_GetICMPv6Statistics(IPAL_Icmpv6Statistics_T *icmp6stat_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(icmp6_stat)

    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETICMPV6STATISTICS;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    memcpy(icmp6stat_p, &(msg_p->data.icmp6_stat), sizeof(IPAL_Icmpv6Statistics_T));

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_GetUDPv6Statistics
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
UI32_T NETCFG_PMGR_IP_GetUDPv6Statistics(IPAL_Udpv6Statistics_T *udp6stat_p)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(udp6_stat)

    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_GETUDPV6STATISTICS;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    memcpy(udp6stat_p, &(msg_p->data.udp6_stat), sizeof(IPAL_Udpv6Statistics_T));

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_IP_ClearIPv6StatisticsByType
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
UI32_T NETCFG_PMGR_IP_ClearIPv6StatisticsByType(UI32_T clear_type)
{
    NETCFG_PMGR_IP_DECLARE_MSG_P(ui32_v)

    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_CLEARIPV6STATISTICSBYTYPE;
    msg_p->data.ui32_v = clear_type;

    NETCFG_PMGR_IP_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

#endif /* SYS_CPNT_IPV6 */

#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME: NETCFG_MGR_IP_ClusterVlanSetRifRowStatus
 * PURPOSE  : Set and delete a IP address on specific Cluster VLAN
 * INPUT    : ipAddress -- for cluster VLAN internal IP
 *            rowStatus -- only allow VAL_netConfigStatus_2_createAndGo and VAL_netConfigStatus_2_destroy
 *            incoming_type -- from which UI (CLI/WEB/dynamic)
 * OUTPUT   : none
 * RETURN   : NETCFG_TYPE_OK
 *            NETCFG_TYPE_FAIL
 * NOTES    :
 *      1. Because we can set only 1 IP, if there is existing one, delete it first. Then we process rowStatus request by each
 *         case, and only validate IP in createAndGo case. If no IP existed, destroy case will return OK.
 *      1. This function is derived from NETCFG_SetRifRowStatus() and for cluster vlan only.
 *      2. Allow only 1 IP address on this VLAN as far.
 *      3. rowStatus only provides VAL_netConfigStatus_2_createAndGo and VAL_netConfigStatus_2_destroy
 *      4. Call NETCFG_MGR_SetIpAddressMode() here.
 *      hawk, 2006.3.1
 */
UI32_T NETCFG_PMGR_IP_ClusterVlanSetRifRowStatus(UI8_T *ipAddress, UI8_T *ipMask, UI32_T rowStatus, UI32_T incoming_type)
{

    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(cluster_vlan_rif);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_CLUSTERVLANSETRIFROWSTATUS;

    memcpy(msg_p->data.cluster_vlan_rif.ipAddress, ipAddress, SYS_ADPT_IPV4_ADDR_LEN);
    memcpy(msg_p->data.cluster_vlan_rif.ipMask, ipMask, SYS_ADPT_IPV4_ADDR_LEN);
    msg_p->data.cluster_vlan_rif.rowStatus = rowStatus;
    msg_p->data.cluster_vlan_rif.incoming_type = incoming_type;

    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
#endif /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_SetCraftInterfaceInetAddress
 * PURPOSE:
 *      To add or delete ipv4/v6 address on craft interface
 * INPUT:
 *      rif_p               -- pointer to rif
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p)
{

    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(craft_addr);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_SETCRAFTINTERFACEINETADDRESS;

    memset(&(msg_p->data.craft_addr), 0, sizeof(msg_p->data.craft_addr));
    memcpy(&(msg_p->data.craft_addr), craft_addr_p, sizeof(msg_p->data.craft_addr));


    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6Enable_Craft
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
UI32_T NETCFG_PMGR_IP_IPv6Enable_Craft(UI32_T ifindex, BOOL_T do_enable)
{

    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_IPV6_IPV6ENABLE_CRAFT;
    msg_p->data.u32a1_bla2.u32_a1 = ifindex;
    msg_p->data.u32a1_bla2.bl_a2 = do_enable;
    
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
#endif /* SYS_CPNT_CRAFT_PORT */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_SetDhcpInform
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
UI32_T NETCFG_PMGR_IP_SetDhcpInform(UI32_T ifindex, BOOL_T do_enable)
{
    const UI32_T msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_IP_IPCCMD_SET_DHCP_INFORM;
    msg_p->data.u32a1_bla2.u32_a1 = ifindex;
    msg_p->data.u32a1_bla2.bl_a2 = do_enable;
    
    if (SYSFUN_SendRequestMsg(netcfg_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
#endif


