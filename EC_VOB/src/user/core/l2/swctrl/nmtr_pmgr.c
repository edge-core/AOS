/* MODULE NAME:  nmtr_pmgr.c
 * PURPOSE:
 *    This is a sample code for implementation of PMGR.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    8/3/2007 - kh shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "nmtr_mgr.h"
#include "nmtr_pmgr.h"
#include "nmtr_type.h"
#include "sys_module.h"
#include "l_cvrt.h"
#include "l_mm.h"

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_InitiateProcessResource                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize kernel resources               
 * INPUT   : None                                                         
 * OUTPUT  : TRUE  -- success
 *           FALSE -- fail
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for AMTR MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_NETACCESS_NMTR_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}


BOOL_T NMTR_PMGR_GetIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_IFTABLE_STATS;
    data_p->data.GetIfStats_data.ifindex = ifindex;
    data_p->data.GetIfStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(if_table_stats_p, &(data_p->data.GetIfStats_data.if_stats), sizeof(SWDRV_IfTableStats_T));

    return data_p->type.ret_bool;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T   *if_table_stats - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_IFTABLE_STATS;
    data_p->data.GetIfStats_data.ifindex = *ifindex;
    data_p->data.GetIfStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetIfStats_data.ifindex;
    memcpy(if_table_stats_p, &(data_p->data.GetIfStats_data.if_stats), sizeof(SWDRV_IfTableStats_T));

    return data_p->type.ret_bool;
}


/*--------------------------
 * Extended MIB2 Statistics                                 
 *--------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfXTableStats                                 
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended iftable a interface           
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T  *if_xtable_stats - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfXStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;    

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_IFXTABLE_STATS;
    data_p->data.GetIfXStats_data.ifindex = ifindex;
    data_p->data.GetIfXStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(if_xtable_stats_p, &(data_p->data.GetIfXStats_data.ifx_stats), sizeof(SWDRV_IfXTableStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfXTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T  *if_xtable_stats - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfXStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_IFXTABLE_STATS;
    data_p->data.GetIfXStats_data.ifindex = *ifindex;
    data_p->data.GetIfXStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetIfXStats_data.ifindex;
    memcpy(if_xtable_stats_p, &(data_p->data.GetIfXStats_data.ifx_stats), sizeof(SWDRV_IfXTableStats_T));

    return data_p->type.ret_bool;
}


/*---------------------------
 * Ether-like MIB Statistics 
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T  *ether_like_stats - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;
    
    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_ETHERLIKE_STATS;
    data_p->data.GetEtherlikeStats_data.ifindex = ifindex;
    data_p->data.GetEtherlikeStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(ether_like_stats_p, &(data_p->data.GetEtherlikeStats_data.etherlike_stats), sizeof(SWDRV_EtherlikeStats_T));

    return data_p->type.ret_bool;
}
        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T  *ether_like_stats - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_ETHERLIKE_STATS;
    data_p->data.GetEtherlikeStats_data.ifindex = *ifindex;
    data_p->data.GetEtherlikeStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetEtherlikeStats_data.ifindex;
    memcpy(ether_like_stats_p, &(data_p->data.GetEtherlikeStats_data.etherlike_stats), sizeof(SWDRV_EtherlikeStats_T));

    return data_p->type.ret_bool;
}


/*---------------------------
 * Ether-like MIB Pause Stats
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_SetEtherLikePauseAdminMode
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will set the ether-like pause function
 * INPUT   : UI32_T ifindex                            - interface index
 *           mode                                      - VAL_dot3PauseAdminMode_disabled
 *                                                       VAL_dot3PauseAdminMode_enabledXmit
 *                                                       VAL_dot3PauseAdminMode_enabledRcv
 *                                                       VAL_dot3PauseAdminMode_enabledXmitAndRcv
 * OUTPUT  : None
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetEtherLikePauseAdminMode(UI32_T ifindex, UI32_T mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_SetEtherlikePauseAdminMode_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;
    
    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_SET_ETHERLIKE_PAUSE_ADMIN_MODE;
    data_p->data.SetEtherlikePauseAdminMode_data.ifindex = ifindex;
    data_p->data.SetEtherlikePauseAdminMode_data.mode    = mode;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like pause statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;
    
    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_ETHERLIKE_PAUSE_STATS;
    data_p->data.GetEtherlikePause_data.ifindex = ifindex;
    data_p->data.GetEtherlikePause_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(ether_like_pause_p, &(data_p->data.GetEtherlikePause_data.etherlike_pause), sizeof(SWDRV_EtherlikePause_T));

    return data_p->type.ret_bool;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetNextEtherLikePause                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_ETHERLIKE_PAUSE_STATS;
    data_p->data.GetEtherlikePause_data.ifindex = *ifindex;
    data_p->data.GetEtherlikePause_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetEtherlikePause_data.ifindex;
    memcpy(ether_like_pause_p, &(data_p->data.GetEtherlikePause_data.etherlike_pause), sizeof(SWDRV_EtherlikePause_T));

    return data_p->type.ret_bool;
}


/*---------------------
 * RMON MIB Statistics 
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON statistics of a interface                
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T  *rome_stats - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetRmonStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_RMON_STATS;
    data_p->data.GetRmonStats_data.ifindex = ifindex;
    data_p->data.GetRmonStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(rmon_stats_p, &(data_p->data.GetRmonStats_data.rmon_stats), sizeof(SWDRV_RmonStats_T));

    return data_p->type.ret_bool;
}
        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index 
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T  *rome_stats - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetRmonStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_RMON_STATS;
    data_p->data.GetRmonStats_data.ifindex = *ifindex;
    data_p->data.GetRmonStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetRmonStats_data.ifindex;
    memcpy(rmon_stats_p, &(data_p->data.GetRmonStats_data.rmon_stats), sizeof(SWDRV_RmonStats_T));

    return data_p->type.ret_bool;
}


#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_IFPERQ_STATS;
    data_p->data.GetIfPerQStats_data.ifindex = ifindex;
    data_p->data.GetIfPerQStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(stats, &(data_p->data.GetIfPerQStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_IFPERQ_STATS;
    data_p->data.GetIfPerQStats_data.ifindex = *ifindex;
    data_p->data.GetIfPerQStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ifindex = data_p->data.GetIfPerQStats_data.ifindex;
        memcpy(stats, &(data_p->data.GetIfPerQStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */


#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPfcStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the PFC statistics of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetPfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPfcStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_PFC_STATS;
    data_p->data.GetPfcStats_data.ifindex = ifindex;
    data_p->data.GetPfcStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(stats, &(data_p->data.GetPfcStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPfcStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next PFC statistics of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextPfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPfcStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_PFC_STATS;
    data_p->data.GetPfcStats_data.ifindex = *ifindex;
    data_p->data.GetPfcStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ifindex = data_p->data.GetPfcStats_data.ifindex;
        memcpy(stats, &(data_p->data.GetPfcStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_PFC == TRUE) */


#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetQcnStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_QCN_STATS;
    data_p->data.GetQcnStats_data.ifindex = ifindex;
    data_p->data.GetQcnStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(stats, &(data_p->data.GetQcnStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetQcnStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_QCN_STATS;
    data_p->data.GetQcnStats_data.ifindex = *ifindex;
    data_p->data.GetQcnStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ifindex = data_p->data.GetQcnStats_data.ifindex;
        memcpy(stats, &(data_p->data.GetQcnStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_CN == TRUE) */


/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 4 
 *         (Port Table for Transparent Bridges)                          */
/*
 *          dot1dTpPortTable OBJECT-TYPE
 *              SYNTAX  SEQUENCE OF Dot1dTpPortEntry
 *              ::= { dot1dTp 4 }
 *          dot1dTpPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortTable 1 }
 *          Dot1dTpPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPort             INTEGER,
 *                  dot1dTpPortMaxInfo      INTEGER,
 *                  dot1dTpPortInFrames     Counter,
 *                  dot1dTpPortOutFrames    Counter,
 *                  dot1dTpPortInDiscards   Counter
 *              }
 */              
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpPortEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_DOT1D_TP_PORT_ENTRY;
    memcpy(&(data_p->data.GetDot1dTpPortEntry_data.tp_port_entry),tp_port_entry,sizeof(NMTR_MGR_Dot1dTpPortEntry_T));
    data_p->data.GetDot1dTpPortEntry_data.next = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(tp_port_entry, &(data_p->data.GetDot1dTpPortEntry_data.tp_port_entry), sizeof(NMTR_MGR_Dot1dTpPortEntry_T));

    return data_p->type.ret_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- the next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetNextDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry) 
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpPortEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_DOT1D_TP_PORT_ENTRY;
    memcpy(&(data_p->data.GetDot1dTpPortEntry_data.tp_port_entry),tp_port_entry,sizeof(NMTR_MGR_Dot1dTpPortEntry_T));
    data_p->data.GetDot1dTpPortEntry_data.next = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(tp_port_entry, &(data_p->data.GetDot1dTpPortEntry_data.tp_port_entry), sizeof(NMTR_MGR_Dot1dTpPortEntry_T));

    return data_p->type.ret_bool;
}


/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 5 ) -- dot1dTp 5
 *          dot1dTpHCPortTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpHCPortEntry
 *              ::= { dot1dTp 5 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpHCPortTable 1 }
 *          Dot1dTpHCPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpHCPortInFrames   Counter64,
 *                  dot1dTpHCPortOutFrames  Counter64,
 *                  dot1dTpHCPortInDiscards Counter64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpHCPortEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_DOT1D_TP_HC_PORT_ENTRY;
    memcpy(&(data_p->data.GetDot1dTpHCPortEntry_data.tp_hc_port_entry),tp_hc_port_entry,sizeof(NMTR_MGR_Dot1dTpHCPortEntry_T));
    data_p->data.GetDot1dTpHCPortEntry_data.next = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(tp_hc_port_entry, &(data_p->data.GetDot1dTpHCPortEntry_data.tp_hc_port_entry), sizeof(NMTR_MGR_Dot1dTpHCPortEntry_T));

    return data_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetNextDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpHCPortEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_DOT1D_TP_HC_PORT_ENTRY;
    memcpy(&(data_p->data.GetDot1dTpHCPortEntry_data.tp_hc_port_entry),tp_hc_port_entry,sizeof(NMTR_MGR_Dot1dTpHCPortEntry_T));
    data_p->data.GetDot1dTpHCPortEntry_data.next = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(tp_hc_port_entry, &(data_p->data.GetDot1dTpHCPortEntry_data.tp_hc_port_entry), sizeof(NMTR_MGR_Dot1dTpHCPortEntry_T));

    return data_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 6 ) -- dot1dTp 6
 *          dot1dTpPortOverflowTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpPortOverflowEntry
 *              ::= { dot1dTp 6 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortOverflowTable 1 }
 *          Dot1dTpPortOverflowEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPortInOverflowFrames     Counter32,
 *                  dot1dTpPortOutOverflowFrames    Counter32,
 *                  dot1dTpPortInOverflowDiscards   Counter32
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpPortOverflowEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_DOT1D_TP_PORT_OVERFLOW_ENTRY;
    memcpy(&(data_p->data.GetDot1dTpPortOverflowEntry_data.tp_port_overflow_entry),tp_port_overflow_entry,sizeof(NMTR_MGR_Dot1dTpPortOverflowEntry_T));
    data_p->data.GetDot1dTpPortOverflowEntry_data.next = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(tp_port_overflow_entry, &(data_p->data.GetDot1dTpPortOverflowEntry_data.tp_port_overflow_entry), sizeof(NMTR_MGR_Dot1dTpPortOverflowEntry_T));

    return data_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetNextDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpPortOverflowEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_DOT1D_TP_PORT_OVERFLOW_ENTRY;
    memcpy(&(data_p->data.GetDot1dTpPortOverflowEntry_data.tp_port_overflow_entry),tp_port_overflow_entry,sizeof(NMTR_MGR_Dot1dTpPortOverflowEntry_T));
    data_p->data.GetDot1dTpPortOverflowEntry_data.next = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(tp_port_overflow_entry, &(data_p->data.GetDot1dTpPortOverflowEntry_data.tp_port_overflow_entry), sizeof(NMTR_MGR_Dot1dTpPortOverflowEntry_T));

    return data_p->type.ret_bool;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value   
 * RETURN  : None                                                           
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetPortUtilization(UI32_T ifindex, NMTR_MGR_Utilization_T *utilization_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPortUtilization_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_PORT_UTILIZATION;
    data_p->data.GetPortUtilization_data.ifindex = ifindex;
    data_p->data.GetPortUtilization_data.next = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }
    
    memcpy(utilization_entry, &(data_p->data.GetPortUtilization_data.utilization_entry), sizeof(NMTR_MGR_Utilization_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get next utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextPortUtilization(UI32_T *ifindex, NMTR_MGR_Utilization_T *utilization_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPortUtilization_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_PORT_UTILIZATION;
    data_p->data.GetPortUtilization_data.ifindex = *ifindex;
    data_p->data.GetPortUtilization_data.next = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetPortUtilization_data.ifindex;
    memcpy(utilization_entry, &(data_p->data.GetPortUtilization_data.utilization_entry), sizeof(NMTR_MGR_Utilization_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwideStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide counters of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
void NMTR_PMGR_ClearSystemwideStats(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_ClearSystemwideStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_CLEAR_SYSTEMWIDE_STATS;
    data_p->data.ClearSystemwideStats_data.ifindex = ifindex;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, NMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return;
    }
    
    return;
}

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide PFC counters of a interface
 * INPUT   : ifindex - interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTR_PMGR_ClearSystemwidePfcStats(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_ClearSystemwideStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_CLEAR_SYSTEMWIDE_PFC_STATS;
    data_p->data.ClearSystemwideStats_data.ifindex = ifindex;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, NMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return;
    }
    
    return;
}
#endif /* (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide iftable of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_IFTABLE_STATS;
    data_p->data.GetIfStats_data.ifindex = ifindex;
    data_p->data.GetIfStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(if_table_stats_p, &(data_p->data.GetIfStats_data.if_stats), sizeof(SWDRV_IfTableStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_IFTABLE_STATS;
    data_p->data.GetIfStats_data.ifindex = *ifindex;
    data_p->data.GetIfStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetIfStats_data.ifindex;
    memcpy(if_table_stats_p, &(data_p->data.GetIfStats_data.if_stats), sizeof(SWDRV_IfTableStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended systemwide iftable a interface
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfXStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_IFXTABLE_STATS;
    data_p->data.GetIfXStats_data.ifindex = ifindex;
    data_p->data.GetIfXStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(if_xtable_stats_p, &(data_p->data.GetIfXStats_data.ifx_stats), sizeof(SWDRV_IfXTableStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfXStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_IFXTABLE_STATS;
    data_p->data.GetIfXStats_data.ifindex = *ifindex;
    data_p->data.GetIfXStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetIfXStats_data.ifindex;
    memcpy(if_xtable_stats_p, &(data_p->data.GetIfXStats_data.ifx_stats), sizeof(SWDRV_IfXTableStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_STATS;
    data_p->data.GetEtherlikeStats_data.ifindex = ifindex;
    data_p->data.GetEtherlikeStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(ether_like_stats_p, &(data_p->data.GetEtherlikeStats_data.etherlike_stats), sizeof(SWDRV_EtherlikeStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_STATS;
    data_p->data.GetEtherlikeStats_data.ifindex = *ifindex;
    data_p->data.GetEtherlikeStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetEtherlikeStats_data.ifindex;
    memcpy(ether_like_stats_p, &(data_p->data.GetEtherlikeStats_data.etherlike_stats), sizeof(SWDRV_EtherlikeStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide pause statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;
    
    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_PAUSE_STATS;
    data_p->data.GetEtherlikePause_data.ifindex = ifindex;
    data_p->data.GetEtherlikePause_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(ether_like_pause_p, &(data_p->data.GetEtherlikePause_data.etherlike_pause), sizeof(SWDRV_EtherlikePause_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_PMGR_GetNextSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 * OUTPUT  : UI32_T *ifindex                           - the next interface index
 *           SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_PAUSE_STATS;
    data_p->data.GetEtherlikePause_data.ifindex = *ifindex;
    data_p->data.GetEtherlikePause_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetEtherlikePause_data.ifindex;
    memcpy(ether_like_pause_p, &(data_p->data.GetEtherlikePause_data.etherlike_pause), sizeof(SWDRV_EtherlikePause_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetRmonStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_RMON_STATS;
    data_p->data.GetRmonStats_data.ifindex = ifindex;
    data_p->data.GetRmonStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(rmon_stats_p, &(data_p->data.GetRmonStats_data.rmon_stats), sizeof(SWDRV_RmonStats_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetRmonStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_RMON_STATS;
    data_p->data.GetRmonStats_data.ifindex = *ifindex;
    data_p->data.GetRmonStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetRmonStats_data.ifindex;
    memcpy(rmon_stats_p, &(data_p->data.GetRmonStats_data.rmon_stats), sizeof(SWDRV_RmonStats_T));

    return data_p->type.ret_bool;
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_IFPERQ_STATS;
    data_p->data.GetIfPerQStats_data.ifindex = ifindex;
    data_p->data.GetIfPerQStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(stats, &(data_p->data.GetIfPerQStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_IFPERQ_STATS;
    data_p->data.GetIfPerQStats_data.ifindex = *ifindex;
    data_p->data.GetIfPerQStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ifindex = data_p->data.GetIfPerQStats_data.ifindex;
        memcpy(stats, &(data_p->data.GetIfPerQStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwidePfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPfcStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_PFC_STATS;
    data_p->data.GetPfcStats_data.ifindex = ifindex;
    data_p->data.GetPfcStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(stats, &(data_p->data.GetPfcStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwidePfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPfcStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_PFC_STATS;
    data_p->data.GetPfcStats_data.ifindex = *ifindex;
    data_p->data.GetPfcStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ifindex = data_p->data.GetPfcStats_data.ifindex;
        memcpy(stats, &(data_p->data.GetPfcStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetSystemwideQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetQcnStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_QCN_STATS;
    data_p->data.GetQcnStats_data.ifindex = ifindex;
    data_p->data.GetQcnStats_data.next    = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(stats, &(data_p->data.GetQcnStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextSystemwideQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetQcnStats_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_SYSTEMWIDE_QCN_STATS;
    data_p->data.GetQcnStats_data.ifindex = *ifindex;
    data_p->data.GetQcnStats_data.next    = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ifindex = data_p->data.GetQcnStats_data.ifindex;
        memcpy(stats, &(data_p->data.GetQcnStats_data.stats), sizeof(*stats));
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_CN == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization300secs
 * -------------------------------------------------------------------------
 * FUNCTION: get the utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetPortUtilization300secs(UI32_T ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPortUtilization300secs_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;
    
    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_PORT_UTILIZATION_300_SECS;
    data_p->data.GetPortUtilization300secs_data.ifindex = ifindex;
    data_p->data.GetPortUtilization300secs_data.next = FALSE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }
   
    memcpy(utilization_entry, &(data_p->data.GetPortUtilization300secs_data.utilization_entry), sizeof(NMTR_MGR_Utilization_300_SECS_T));

    return data_p->type.ret_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_PMGR_GetNextPortUtilization300secs
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 * OUTPUT  : ifindex            -- interface index
 * OUTPUT  : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T NMTR_PMGR_GetNextPortUtilization300secs(UI32_T *ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPortUtilization300secs_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;
    
    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_PORT_UTILIZATION_300_SECS;
    data_p->data.GetPortUtilization300secs_data.ifindex = *ifindex;
    data_p->data.GetPortUtilization300secs_data.next = TRUE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *ifindex = data_p->data.GetPortUtilization300secs_data.ifindex;
    memcpy(utilization_entry, &(data_p->data.GetPortUtilization300secs_data.utilization_entry), sizeof(NMTR_MGR_Utilization_300_SECS_T));

    return data_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetEtherStatsHighCapacityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified ether statistics
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   es_hc_entry->ether_stats_index          -- (key)
 * OUTPUT   :   es_hc_entry         -- Ether stats high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherStatsHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_PMGR_GetEtherStatsHighCapacityEntry(NMTR_MGR_EtherStatsHighCapacityEntry_T *es_hc_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherStatsHighCapacityEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_ETHER_STATS_HIGH_CAPACITY_ENTRY;
    memcpy(&(data_p->data.GetEtherStatsHighCapacityEntry_data.es_hc_entry),es_hc_entry,sizeof(NMTR_MGR_EtherStatsHighCapacityEntry_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(es_hc_entry, &(data_p->data.GetEtherStatsHighCapacityEntry_data.es_hc_entry), sizeof(NMTR_MGR_EtherStatsHighCapacityEntry_T));

    return data_p->type.ret_bool;
}

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetDefaultHistoryCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will create default ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetDefaultHistoryCtrlEntry(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_SET_DEFAULT_HISTORY_CTRL_ENTRY;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    tmp_entry_p->data_source = ifindex;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a field of ctrl entry
 * INPUT   : ctrl_idx
 *           field_idx
 *           data_p    - value of field
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetHistoryCtrlEntryField(UI32_T ctrl_idx, UI32_T field_idx, void *data)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntryField_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_SET_HISTORY_CTRL_ENTRY_FIELD;
    data_p->data.HistoryCtrlEntryField_data.ctrl_idx = ctrl_idx;
    data_p->data.HistoryCtrlEntryField_data.field_idx = field_idx;

    if (field_idx == NMTR_TYPE_HIST_CTRL_FIELD_NAME)
    {
        strncpy(
            data_p->data.HistoryCtrlEntryField_data.data.s,
            data,
            sizeof(data_p->data.HistoryCtrlEntryField_data.data.s));
        data_p->data.HistoryCtrlEntryField_data.data.s[sizeof(data_p->data.HistoryCtrlEntryField_data.data.s)-1] = 0;
    }
    else
    {
        data_p->data.HistoryCtrlEntryField_data.data.i = L_CVRT_PTR_TO_UINT(data);
    }

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : ctrl_p->ctrl_idx
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_SetHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_SET_HISTORY_CTRL_ENTRY_BY_NAME_AND_DATASRC;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    memcpy(tmp_entry_p, ctrl_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        ctrl_p->ctrl_idx = tmp_entry_p->ctrl_idx;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_DestroyHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_DESTROY_HISTORY_CTRL_ENTRY_BY_NAME_AND_DATASRC;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    memcpy(tmp_entry_p, ctrl_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy ctrl entries related to 
 *           specified interface
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_DestroyHistoryCtrlEntryByIfindex(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_DESTROY_HISTORY_CTRL_ENTRY_BY_IFINDEX;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    tmp_entry_p->data_source = ifindex;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           get_next
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_HISTORY_CTRL_ENTRY;
    data_p->data.HistoryCtrlEntry_data.next = get_next;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    memcpy(tmp_entry_p, ctrl_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(ctrl_p, tmp_entry_p, sizeof(*ctrl_p));
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get a ctrl entry with specified data source
 * INPUT   : entry_p->ctrl_idx   - for get_next = TRUE
 *           ctrl_p->name        - for get_next = FALSE
 *           ctrl_p->data_source
 *           get_next
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_HISTORY_CTRL_ENTRY_BY_DATASRC;
    data_p->data.HistoryCtrlEntry_data.next = get_next;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    memcpy(tmp_entry_p, ctrl_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ctrl_p = *tmp_entry_p;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryCurrentEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get current entry
 * INPUT   : entry_p->ctrl_idx
 *           get_next
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryCurrentEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistSampleEntry_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_HISTORY_CURRENT_ENTRY;
    data_p->data.HistorySampleEntry_data.next = get_next;
    tmp_entry_p = &data_p->data.HistorySampleEntry_data.entry;
    memcpy(tmp_entry_p, entry_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(entry_p, tmp_entry_p, sizeof(*entry_p));
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetHistoryPreviousEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get previous entry
 * INPUT   : entry_p->ctrl_idx
 *           entry_p->sample_idx
 *           get_next
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetHistoryPreviousEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistSampleEntry_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_HISTORY_PREVIOUS_ENTRY;
    data_p->data.HistorySampleEntry_data.next = get_next;
    tmp_entry_p = &data_p->data.HistorySampleEntry_data.entry;
    memcpy(tmp_entry_p, entry_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(entry_p, tmp_entry_p, sizeof(*entry_p));
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetNextHistoryPreviousEntryByCtrlIdx
 *-----------------------------------------------------------------
 * FUNCTION: This function will get next previous entry
 * INPUT   : entry_p->ctrl_idx
 *           entry_p->sample_idx
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextHistoryPreviousEntryByCtrlIdx(NMTR_TYPE_HistSampleEntry_T *entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistSampleEntry_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_NEXT_HISTORY_PREVIOUS_ENTRY_BY_CTRL_IDX;
    tmp_entry_p = &data_p->data.HistorySampleEntry_data.entry;
    memcpy(tmp_entry_p, entry_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        memcpy(entry_p, tmp_entry_p, sizeof(*entry_p));
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetNextRemovedDefaultHistoryCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get removed default ctrl entry
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextRemovedDefaultHistoryCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_NEXT_REMOVED_DEFAULT_HISTORY_CTRL_ENTRY_BY_DATASRC;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    memcpy(tmp_entry_p, ctrl_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ctrl_p = *tmp_entry_p;
    }

    return data_p->type.ret_bool;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetNextUserCfgHistoryCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get user configured ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           ctrl_p->data_source
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_PMGR_GetNextUserCfgHistoryCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NMTR_MGR_IPCMsg_T *data_p;
    NMTR_TYPE_HistCtrlInfo_T *tmp_entry_p;

    msgbuf_p->cmd = SYS_MODULE_NMTR;
    msgbuf_p->msg_size = msg_size;

    data_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = NMTR_MGR_IPC_GET_NEXT_USER_CFG_HISTORY_CTRL_ENTRY_BY_DATASRC;
    tmp_entry_p = &data_p->data.HistoryCtrlEntry_data.entry;
    memcpy(tmp_entry_p, ctrl_p, sizeof(*tmp_entry_p));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (data_p->type.ret_bool)
    {
        *ctrl_p = *tmp_entry_p;
    }

    return data_p->type.ret_bool;
}
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

