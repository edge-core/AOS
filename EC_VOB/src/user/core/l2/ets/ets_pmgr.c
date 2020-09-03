/* MODULE NAME: ets_pmgr.c
 * PURPOSE:
 *   Definitions of MGR IPC APIs for ETS
 *   (802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "ets_pmgr.h"





/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define ETS_PMGR_FUNC_BEGIN(req_sz, rep_sz, cmd_id)      \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG((req_sz>rep_size)?req_sz:rep_size)]; \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
    ETS_MGR_IpcMsg_T *msg_p;                             \
                                                            \
    msgbuf_p->cmd = SYS_MODULE_ETS;                      \
    msgbuf_p->msg_size = req_size;                          \
    msg_p = (ETS_MGR_IpcMsg_T *)msgbuf_p->msg_buf;       \
    msg_p->type.cmd = cmd_id;


#if (ETS_PMGR_DEBUG_ENABLE==TRUE)

#define ETS_PMGR_DEBUG_LINE() \
{  \
  printf("\r\n%s(%d)",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define ETS_PMGR_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}
#else
#define ETS_PMGR_DEBUG_LINE()
#define ETS_PMGR_DEBUG_MSG(a,b...)
#endif
/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - ETS_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for ETS_PMGR in the calling process.
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
BOOL_T ETS_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for ETS MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS mode on the interface.
 * INPUT   : lport -- which port to configure
 * OUTPUT  : mode  -- current mode
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_IPCCMD_GET_MODE)

    /*assign input*/
    msg_p->data.ets_lport_mode.lport=lport;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, rep_size, msgbuf_p)!=SYSFUN_OK)
    {
        ETS_PMGR_DEBUG_LINE();
        return ETS_TYPE_RETURN_OTHERS;
    }
    /*assign output*/
    *mode = msg_p->data.ets_lport_mode.mode;

    return msg_p->type.result_ui32;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS mode on the interface.
 * INPUT   : lport -- which port to configure.
 *           mode  -- which mode to set.
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetMode(UI32_T lport, ETS_TYPE_MODE_T mode)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_MSGBUF_TYPE_SIZE,
        ETS_MGR_IPCCMD_SET_MODE);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;

        msg_p->data.ets_lport_mode.lport = lport;
        msg_p->data.ets_lport_mode.mode  = mode;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport -- which port to get
 * OUTPUT  : mode  -- current operation mode
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetOperMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_IPCCMD_GET_OPER_MODE)

    /*assign input*/
    msg_p->data.ets_lport_mode.lport=lport;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, rep_size, msgbuf_p)!=SYSFUN_OK)
    {
        ETS_PMGR_DEBUG_LINE();
        return ETS_TYPE_RETURN_OTHERS;
    }
    /*assign output*/
    *mode = msg_p->data.ets_lport_mode.mode;

    return msg_p->type.result_ui32;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Get TSA of a traffic class on the interface.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           oper_cfg -- operation or configuration
 * OUTPUT  : tsa      -- outputed TSA
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetTSA(UI32_T lport, UI32_T tc, UI32_T* tsa, ETS_TYPE_DB_T oper_cfg)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_tc_tsa_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_tc_tsa_oper_cfg),
        ETS_MGR_IPCCMD_GET_TC_TSA);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.lport     =lport;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.tc        =tc;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.oper_cfg  =oper_cfg;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            *tsa = msg_p->data.ets_lport_tc_tsa_oper_cfg.tsa;
            ret = msg_p->type.result_ui32;
        }
        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetTSAByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Set TSA of a traffic class on the interface from UI.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           tsa      -- what kind of  TSA
 * OUTPUT  : NONE
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are updated.
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetTSAByUser(UI32_T lport, UI32_T tc, UI32_T tsa)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_tc_tsa_oper_cfg),
        ETS_MGR_MSGBUF_TYPE_SIZE,
        ETS_MGR_IPCCMD_SET_TC_TSA_BY_USER);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.lport=lport;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.tc= tc;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.tsa= tsa;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetPortPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set the priotiy to TC mapping tc in a port
 * INPUT   : lport  - which port
 *           prio   - which priority
 * OUTPUT  : tc     - the TC of the prio belongs
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetPortPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc, ETS_TYPE_DB_T oper_cfg)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_prio_tc_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_prio_tc_oper_cfg),
        ETS_MGR_IPCCMD_GET_PRIO_ASSIGNMENT);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_prio_tc_oper_cfg.lport   =lport;
        msg_p->data.ets_lport_prio_tc_oper_cfg.prio    =prio;
        msg_p->data.ets_lport_prio_tc_oper_cfg.oper_cfg=oper_cfg;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            *tc = msg_p->data.ets_lport_prio_tc_oper_cfg.tc;
            ret = msg_p->type.result_ui32;
        }
        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetPortPrioAssignByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Change mapping of priority to tc in a port
 * INPUT   : lport - which port
 *           prio  - which Cos priority
 *           tc    - which traffic class of this Cos priority belong
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are copied.
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetPortPrioAssignByUser(UI32_T lport, UI32_T prio, UI32_T tc)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_prio_tc_oper_cfg),
        ETS_MGR_MSGBUF_TYPE_SIZE,
        ETS_MGR_IPCCMD_SET_PRIO_ASSIGNMENT_BY_USER);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_prio_tc_oper_cfg.lport=lport;
        msg_p->data.ets_lport_prio_tc_oper_cfg.prio= prio;
        msg_p->data.ets_lport_prio_tc_oper_cfg.tc= tc;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS],  ETS_TYPE_DB_T oper_cfg)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_weight_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_weight_oper_cfg),
        ETS_MGR_IPCCMD_GET_TC_WEIGHT);

    {

        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_weight_oper_cfg.lport  =lport;
        msg_p->data.ets_lport_weight_oper_cfg.oper_cfg=oper_cfg;
        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            memcpy(weight, msg_p->data.ets_lport_weight_oper_cfg.weight,  sizeof(msg_p->data.ets_lport_weight_oper_cfg.weight));
            ret = msg_p->type.result_ui32;
        }
        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetWeightByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Set BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are updated.
 *           Weight is always 0 for non-ETS TC.
 *           Weight is 0~100 for ETS TC.
 *           If all weights are 0xffffffff, weights will be reset to default.
 *           For chip, weight=0 means Strict Priority, so we work around to be 1 by
 *              update OM and get again by ETS_MGR_GetAllTCWeight().
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetWeightByUser(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_weight_oper_cfg),
        ETS_MGR_MSGBUF_TYPE_SIZE,
        ETS_MGR_IPCCMD_SET_TC_WEIGHT_BY_USER);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_weight_oper_cfg.lport  =lport;
        memcpy(msg_p->data.ets_lport_weight_oper_cfg.weight, weight, sizeof(msg_p->data.ets_lport_weight_oper_cfg.weight));

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           oper_cfg - operation or configuration
 * OUTPUT  : entry - input OM entry
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetPortEntry(UI32_T lport, ETS_TYPE_PortEntry_T  *entry, ETS_TYPE_DB_T oper_cfg)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_entry_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_entry_oper_cfg),
        ETS_MGR_IPCCMD_GET_PORT_ENTRY);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_entry_oper_cfg.lport      =lport;
        msg_p->data.ets_lport_entry_oper_cfg.oper_cfg   =oper_cfg;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            memcpy(entry, &msg_p->data.ets_lport_entry_oper_cfg.entry,  sizeof(msg_p->data.ets_lport_entry_oper_cfg.entry));
            ret = msg_p->type.result_ui32;
        }
        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetNextPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport_p  - which port to be overwritten
 *           oper_cfg - operation or configuration
 * OUTPUT  : entry_p  - input OM entry
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetNextPortEntry(
    UI32_T *lport_p, ETS_TYPE_PortEntry_T  *entry_p, ETS_TYPE_DB_T oper_cfg)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_entry_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_entry_oper_cfg),
        ETS_MGR_IPCCMD_GET_NEXT_PORT_ENTRY);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;
        msg_p->data.ets_lport_entry_oper_cfg.lport      =*lport_p;
        msg_p->data.ets_lport_entry_oper_cfg.oper_cfg   =oper_cfg;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            memcpy(entry_p, &msg_p->data.ets_lport_entry_oper_cfg.entry,  sizeof(msg_p->data.ets_lport_entry_oper_cfg.entry));
            *lport_p = msg_p->data.ets_lport_entry_oper_cfg.lport;
            ret = msg_p->type.result_ui32;
        }
        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetMaxNumberOfTC
 * -------------------------------------------------------------------------
 * FUNCTION: Get max number of ETS  on this machine
 * INPUT   : none
 * OUTPUT  : max_nbr_of_tc - max number of Traffic class (machine depends)
 * RETURN  : ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetMaxNumberOfTC(UI8_T *max_nbr_of_tc)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ui8_data),
        ETS_MGR_GET_MSG_SIZE(ui8_data),
        ETS_MGR_IPCCMD_GET_MAX_NUMBER_OF_TC);

    {
        UI32_T  ret = ETS_TYPE_RETURN_OTHERS;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            *max_nbr_of_tc = msg_p->data.ui8_data;
            ret = msg_p->type.result_ui32;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningPortMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport    -- which port to configure.
 * OUTPUT  : mode     -- which mode to set.
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningPortMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_GET_MSG_SIZE(ets_lport_mode),
        ETS_MGR_IPCCMD_GETRUNNING_PORT_MODE)

    /*assign input*/
    msg_p->data.ets_lport_mode.lport=lport;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, rep_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    /*assign output*/
    *mode = msg_p->data.ets_lport_mode.mode;
    return msg_p->type.ret_running_cfg;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningTCTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Get TSA of a traffic class on the interface.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 * OUTPUT  : tsa      -- outputed TSA
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningTCTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T* tsa)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_tc_tsa_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_tc_tsa_oper_cfg),
        ETS_MGR_IPCCMD_GETRUNNING_TC_TSA);

    {
        SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.lport   =lport;
        msg_p->data.ets_lport_tc_tsa_oper_cfg.tc      =tc;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            *tsa = msg_p->data.ets_lport_tc_tsa_oper_cfg.tsa;
            ret = msg_p->type.ret_running_cfg;
        }
        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_weight_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_weight_oper_cfg),
        ETS_MGR_IPCCMD_GETRUNNING_TC_WEIGHT);

    {
        SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
        msg_p->data.ets_lport_weight_oper_cfg.lport  =lport;
        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            memcpy(weight,msg_p->data.ets_lport_weight_oper_cfg.weight,  sizeof(msg_p->data.ets_lport_weight_oper_cfg.weight));
            ret = msg_p->type.ret_running_cfg;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set the priotiy to TC mapping tc in a port
 * INPUT   : lport  - which port
 *           prio   - which priority
 * OUTPUT  : tc     - the TC of the prio belongs
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc)
{
    ETS_PMGR_FUNC_BEGIN(
        ETS_MGR_GET_MSG_SIZE(ets_lport_prio_tc_oper_cfg),
        ETS_MGR_GET_MSG_SIZE(ets_lport_prio_tc_oper_cfg),
        ETS_MGR_IPCCMD_GETRUNNING_PRIO_ASSIGNMENT);
    {
        SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
        msg_p->data.ets_lport_prio_tc_oper_cfg.lport   =lport;
        msg_p->data.ets_lport_prio_tc_oper_cfg.prio    =prio;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            *tc = msg_p->data.ets_lport_prio_tc_oper_cfg.tc;
            ret = msg_p->type.ret_running_cfg;
        }
        return ret;
    }
}


