#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sys_module.h"
#include "sysfun.h"
#include "swctrl.h"
#include "swctrl_init.h"
#include "swctrl_task.h"
#include "swctrl_pmgr.h"
#include "stktplg_om.h"

#define SWCTRL_PMGR_DEBUG_ENABLE FALSE

#if (SWCTRL_PMGR_DEBUG_ENABLE==TRUE)

#define SWCTRL_PMGR_DEBUG_LINE() \
{  \
  printf("\r\n%s(%d)",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define SWCTRL_PMGR_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}
#else
#define SWCTRL_PMGR_DEBUG_LINE()
#define SWCTRL_PMGR_DEBUG_MSG(a,b...)
#endif

#define SWCTRL_PMGR_FUNC_BEGIN(req_sz, rep_sz, cmd_id)      \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG((req_sz>rep_size)?req_sz:rep_size)]; \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
    SWCTRL_MGR_IPCMsg_T *msg_p;                             \
                                                            \
    msgbuf_p->cmd = SYS_MODULE_SWCTRL;                      \
    msgbuf_p->msg_size = req_size;                          \
    msg_p = (SWCTRL_MGR_IPCMsg_T *)msgbuf_p->msg_buf;       \
    msg_p->type.cmd = cmd_id;


/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for SWCTRL_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void SWCTRL_PMGR_Init(void)
{
    /* Given that L4 PMGR requests are handled in L4GROUP of L2_L4_PROC
     */
    SWCTRL_PMGR_DEBUG_LINE();
    if(SYSFUN_GetMsgQ(SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port administration status
 * INPUT   : ifindex        -- which port to set
 *           admin_status   -- VAL_ifAdminStatus_up/VAL_ifAdminStatus_down
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAdminStatus(UI32_T ifindex, UI32_T admin_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTADMINSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=admin_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port administration status
 * INPUT   : ifindex        -- which port to set
 *           status         -- TRUE to be up; FALSE to be down
 *           reason         -- indicates role of caller
 *                             bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. when set admin up with reason = SWCTRL_PORT_STATUS_SET_BY_CFG,
 *              it works like set admin up for all.
 *           2. user config will be affected only if set with SWCTRL_PORT_STATUS_SET_BY_CFG.
 *           3. Only CMGR_SetPortStatus may call SWCTRL_SetPortStatus/SWCTRL_PMGR_SetPortStatus; 
 *              CSCs that is in the layer higher than CMGR may call CMGR_SetPortStatus
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortStatus(UI32_T ifindex, BOOL_T status, UI32_T reason)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=status;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=reason;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPort1000BaseTForceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set force[master/slave] mode configuration of a port
 * INPUT   : ifindex      -- which port to set
 *           forced_mode  -- VAL_portMasterSlaveModeCfg_master / VAL_portMasterSlaveModeCfg_slave
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPort1000BaseTForceMode(UI32_T ifindex, UI32_T forced_mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORT1000BASETFORCEMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=forced_mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCfgSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set speed/duplex configuration of a port
 * INPUT   : ifindex      -- which port to set
 *           speed_duplex -- speed/duplex to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCfgSpeedDuplex(UI32_T ifindex, UI32_T speed_duplex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTCFGSPEEDDUPLEX;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=speed_duplex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDefaultSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port speed/duplex to default value
 * INPUT   : l_port      -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : for CLI use
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDefaultSpeedDuplex(UI32_T l_port)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTDEFAULTSPEEDDUPLEX;

    /*assign input*/
    msg_data_p->data.ui32_v=l_port;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAutoNegEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto-negotiation enable state
 * INPUT   : ifindex        -- which port to set
 *           autoneg_state  -- VAL_portAutonegotiation_enabled /
 *                             VAL_portAutonegotiation_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. EA3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAutoNegEnable(UI32_T ifindex, UI32_T autoneg_state)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTAUTONEGENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=autoneg_state;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCfgFlowCtrlEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable flow control of a port or not
 * INPUT   : ifindex -- which port to set
 *           flow_contrl_cfg    -- VAL_portFlowCtrlCfg_enabled /
 *                                 VAL_portFlowCtrlCfg_disabled /
 *                                 VAL_portFlowCtrlCfg_tx /
 *                                 VAL_portFlowCtrlCfg_rx
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCfgFlowCtrlEnable(UI32_T ifindex, UI32_T flow_control_cfg)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTCFGFLOWCTRLENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=flow_control_cfg;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAutoNegCapability
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the capability of auto-negotiation of a
 *           port
 * INPUT   : ifindex    -- which port to get
 *           capability -- auto-negotiation capability
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : Flow control capability bit always depends on flow control mode.
 *           ie. If flow control is enabled, when enabing auto negotiation,
 *               flow control capability bit needs to be set on as well.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAutoNegCapability(UI32_T ifindex, UI32_T capability)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTAUTONEGCAPABILITY;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=capability;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDefaultAutoNegCapability
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the capability of auto-negotiation of a
 *           port to default value
 * INPUT   : ifindex
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : for CLI use
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDefaultAutoNegCapability(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTDEFAULTAUTONEGCAPABILITY;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#if (SYS_CPNT_SWCTRL_FEC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortFec
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable FEC
 * INPUT   : ifindex
 *           fec_mode - VAL_portFecMode_XXX
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortFec(UI32_T ifindex, UI32_T fec_mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTFEC;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=fec_mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif /* (SYS_CPNT_SWCTRL_FEC == TRUE) */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLinkChangeTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable to send trap when port link state
 *           changes.
 * INPUT   : ifindex            -- which port to set
 *           link_change_trap   -- VAL_ifLinkUpDownTrapEnable_enabled/
                                   VAL_ifLinkUpDownTrapEnable_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLinkChangeTrapEnable(UI32_T ifindex, UI32_T link_change_trap)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTLINKCHANGETRAPENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=link_change_trap;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDot1xEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is called by dot1x. In this way SWCTRL could know
 *           the dot1x enable/disable status of this port and then SWCTRL can
 *           make the state machine work without dot1x intervention.
 * INPUT   : ifindex            -- which port to set
 *           dot1x_port_status  -- 1) SWCTRL_DOT1X_PORT_DISABLE
 *                                 2) SWCTRL_DOT1X_PORT_ENABLE
 * OUTPUT  : None
 * RETURN  : TRUE:  Successfully.
 *           FALSE: 1) ifindex is not belong to user port.
 *                  2) This port is not present.
 *                  3) Parameter error.
 * NOTE    : For dot1x.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDot1xEnable(UI32_T ifindex, UI32_T dot1x_port_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTDOT1XENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=dot1x_port_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDot1xAuthState
 * -------------------------------------------------------------------------
 * FUNCTION: This function is called by dot1x. SWCTRL use this event to drive
 *           the state machine of the oper state.
 *           the dot1x enable/disable status of this port and then SWCTRL can
 *           make the state machine work without dot1x intervention.
 * INPUT   : ifindex          -- which port to set
 *           dot1x_auth_status -- 1) SWCTRL_DOT1X_PORT_AUTHORIZED
 *                                2) SWCTRL_DOT1X_PORT_UNAUTHORIZED
 * OUTPUT  : None.
 * RETURN  : TRUE:  Successfully.
 *           FALSE: 1) ifindex is not belong to user port.
 *                  2) This port is not present.
 *                  3) Parameter error.
 * NOTE    : For dot1x.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDot1xAuthState(UI32_T ifindex, UI32_T dot1x_auth_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTDOT1XAUTHSTATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=dot1x_auth_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpOperEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set LACP oper status.
 * INPUT   : ifindex --- Which user port.
 *           lacp_oper_state --- VAL_lacpPortStatus_enabled/
 *                               VAL_lacpPortStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. For LACP only.
 *           2. User port only.
 *           3. SWCTRL use this oper status to run ifOperStatus state machine.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpOperEnable(UI32_T ifindex, UI32_T lacp_oper_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTLACPOPERENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=lacp_oper_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpAdminEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set LACP admin status.
 * INPUT   : ifindex --- Which user port.
 *           lacp_admin_status --- VAL_lacpPortStatus_enabled/
 *                                 VAL_lacpPortStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. For LACP only.
 *           2. User port only.
 *           3. SWCTRL use this admin status to do mutex check with dot1x.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpAdminEnable(UI32_T ifindex, UI32_T lacp_admin_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTLACPADMINENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=lacp_admin_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#else
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the LACP function of a port
 * INPUT   : ifindex        -- which port to set
 *           lacp_state     -- VAL_lacpPortStatus_enabled/
                               VAL_lacpPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpEnable(UI32_T ifindex, UI32_T lacp_state)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTLACPENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=lacp_state;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetLacpEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the LACP function of whole system
 * INPUT   : lacp_state     -- VAL_lacpPortStatus_enabled/
 *                             VAL_lacpPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetLacpEnable(UI32_T lacp_state)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETLACPENABLE;

    /*assign input*/
    msg_data_p->data.ui32_v=lacp_state;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpCollecting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the LACP state to collecting
 * INPUT   : ifindex            -- which port to set
 *           lacp_collecting    -- VAL_LacpCollecting_collecting/
 *                                 VAL_LacpCollecting_not_collecting
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpCollecting(UI32_T ifindex, UI32_T lacp_collecting)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTLACPCOLLECTING;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=lacp_collecting;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortOperDormantStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To set status of ifOperStatus 'dormant'
 * INPUT   : ifindex - specified port to change status
 *           level   - see SWCTRL_OperDormantLevel_T
 *           enable  - TRUE/FALSE
 *           stealth - relevant if enable is TRUE.
 *                     TRUE to avoid oper status change from upper (e.g. up)
 *                     to this level of dormant.
 *                     FALSE to allow oper status change from upper (e.g. up)
 *                     to this level of dormant.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortOperDormantStatus(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    BOOL_T enable,
    BOOL_T stealth)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3_u32a4),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTOPERDORMANTSTATUS);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = level;
        msg_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = enable;
        msg_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = stealth;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TriggerPortOperDormantEvent
 * -------------------------------------------------------------------------
 * FUNCTION: To set status of ifOperStatus 'dormant'
 * INPUT   : ifindex - specified port to change status
 *           level   - see SWCTRL_OperDormantLevel_T
 *           event   - see SWCTRL_OperDormantEvent_T
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TriggerPortOperDormantEvent(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    SWCTRL_OperDormantEvent_T event)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_TRIGGERPORTOPERDORMANTEVENT);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2_u32a3.u32_a2 = level;
        msg_p->data.u32a1_u32a2_u32a3.u32_a3 = event;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSTAState
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set STA state of a port
 * INPUT   : vid     -- which VLAN to set
 *           ifindex -- which port to set
 *           state   -- spanning tree state to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : This function only for Allayer chip using when include 802.1x feature.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSTAState(UI32_T vid,
                              UI32_T ifindex,
                              UI32_T state)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSTASTATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=vid;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=state;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMstEnableStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will is used to set the spanning tree protocal
 * INPUT   : mst_enable  -- SWCTRL_MST_DISABLE
 *                          SWCTRL_MST_ENABLE
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMstEnableStatus(UI32_T mst_enable_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMSTENABLESTATUS;

    /*assign input*/
    msg_data_p->data.ui32_v=mst_enable_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security status
 * INPUT    : ifindex : the logical port
 *            port_security_status : VAL_portSecPortStatus_enabled
 *                                   VAL_portSecPortStatus_disabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSecurityStatus( UI32_T ifindex, UI32_T  port_security_status,UI32_T port_security_called_by_who /*kevin*/)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSECURITYSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=port_security_status;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=port_security_called_by_who;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security action status
 * INPUT    : ifindex : the logical port
 *            action_status: VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSecurityActionStatus( UI32_T ifindex, UI32_T  action_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSECURITYACTIONSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=action_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security action trap operation status
 * INPUT    : ifindex : the logical port
 *            action_trap_status : VAL_portSecActionTrap_enabled
 *                                 VAL_portSecActionTrap_disabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus( UI32_T ifindex, UI32_T  action_trap_oper_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSECURITYACTIONTRAPOPERSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=action_trap_oper_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_AddSecurityActionTrapTimeStamp
 * ------------------------------------------------------------------------|
 * FUNCTION : increase the timer stamp to the timer of this port?
 * INPUT    : ifindex : interface index.
 *            increase_time_stamp_ticks : increase ticks of the timer stamp.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddSecurityActionTrapTimeStamp( UI32_T ifindex, UI32_T increase_time_stamp_ticks)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADDSECURITYACTIONTRAPTIMESTAMP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=increase_time_stamp_ticks;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_ResetSecurityActionTrapTimeStamp
 * ------------------------------------------------------------------------|
 * FUNCTION : reset the timer stamp to the timer of this port?
 * INPUT    : ifindex : interface index.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ResetSecurityActionTrapTimeStamp( UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_RESETSECURITYACTIONTRAPTIMESTAMP;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPVID
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set default VLAN ID of a port
 * INPUT   : ifindex -- which port to set
 *           pvid    -- permanent VID to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPVID(UI32_T ifindex, UI32_T pvid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTPVID;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=pvid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_CreateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a specified VLAN
 * INPUT   : vid -- which VLAN to create
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_CreateVlan(UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_CREATEVLAN;

    /*assign input*/
    msg_data_p->data.ui32_v=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DestroyVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a specified VLAN
 * INPUT   : vid -- which VLAN to delete
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DestroyVlan(UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DESTROYVLAN;

    /*assign input*/
    msg_data_p->data.ui32_v=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetGlobalDefaultVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function changes the global default VLAN
 * INPUT   : vid                -- the vid of the new default VLAN
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If the specified VLAN is not available.
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetGlobalDefaultVlan(UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETGLOBALDEFAULTVLAN;

    /*assign input*/
    msg_data_p->data.ui32_v=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddPortToVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a port to the member set of a specified
 *           VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddPortToVlanMemberSet(UI32_T ifindex, UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADDPORTTOVLANMEMBERSET;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DeletePortFromVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the member set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to delete
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeletePortFromVlanMemberSet(UI32_T ifindex, UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELETEPORTFROMVLANMEMBERSET;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddPortToVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set a port to output untagged frames over
 *           the specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddPortToVlanUntaggedSet(UI32_T ifindex, UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADDPORTTOVLANUNTAGGEDSET;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DeletePortFromVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the untagged set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : Delete a port from untagged set means to recover this port to be
 *           a tagged member set of specified vlan.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeletePortFromVlanUntaggedSet(UI32_T ifindex, UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELETEPORTFROMVLANUNTAGGEDSET;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



BOOL_T SWCTRL_PMGR_GetSystemMTU (UI32_T *jumbo,UI32_T *mtu)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();


    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETSYSTEMMTU;



    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign input*/
    *jumbo = msg_data_p->data.u32a1_u32a2.u32_a1;
    *mtu = msg_data_p->data.u32a1_u32a2.u32_a2;
    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}



BOOL_T SWCTRL_PMGR_SetSystemMTU(UI32_T jumbo, UI32_T mtu)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();


    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETSYSTEMMTU;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=jumbo;
    msg_data_p->data.u32a1_u32a2.u32_a2=mtu;


    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}




BOOL_T SWCTRL_PMGR_SetPortMTU(UI32_T ifindex, UI32_T mtu)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();


    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTMTU;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mtu;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable ingress filter of a port
 * INPUT   : ifindex -- which port to enable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableIngressFilter(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEINGRESSFILTER;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable ingress filter of a port
 * INPUT   : ifindex -- which port to disable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableIngressFilter(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEINGRESSFILTER;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AdmitVLANTaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow tagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AdmitVLANTaggedFramesOnly(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADMITVLANTAGGEDFRAMESONLY;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AdmitVLANUntaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow untagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AdmitVLANUntaggedFramesOnly(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADMITVLANUNTAGGEDFRAMESONLY;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AdmitAllFrames

 * -------------------------------------------------------------------------
 * FUNCTION: This function will allow all kinds of frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AdmitAllFrames(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADMITALLFRAMES;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AllowToBeTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return TRUE if the port is allowed to add
 *           into trunk
 * INPUT   : unit -- unit number
 *           port -- which port to check
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AllowToBeTrunkMember(UI32_T unit, UI32_T port)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ALLOWTOBETRUNKMEMBER;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=unit;
    msg_data_p->data.u32a1_u32a2.u32_a2=port;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetTrunkBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the balance mode of trunking
 * INPUT   : balance_mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : balance_mode:
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA      Determinded by source mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_DA      Determinded by destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA   Determinded by source and destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA       Determinded by source IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_DA       Determinded by destination IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA    Determinded by source and destination IP address
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetTrunkBalanceMode(UI32_T balance_mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETTRUNKBALANCEMODE;

    /*assign input*/
    msg_data_p->data.ui32_v = balance_mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 *           max_num_of_active_ports
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T max_num_of_active_ports)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETTRUNKMAXNUMOFACTIVEPORTS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = trunk_id;
    msg_data_p->data.u32a1_u32a2.u32_a2 = max_num_of_active_ports;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetTrunkMemberActiveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a members to a trunk
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to add
 *           is_active  -- active or inactive member
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetTrunkMemberActiveStatus(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port, BOOL_T is_active)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.trunkid_unitport)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETTRUNKMEMBERACTIVESTATUS;

    /*assign input*/
    msg_data_p->data.trunkid_unitport.trunk_id = trunk_id;
    msg_data_p->data.trunkid_unitport.unit_port = unit_port;
    msg_data_p->data.trunkid_unitport.u32_a1 = is_active;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the IGMP function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : RFC2933/igmpInterfaceStatus
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableIgmpTrap(SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEIGMPTRAP;

    /*assign input*/
    msg_data_p->data.ui32_v=owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the IGMP function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableIgmpTrap(SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEIGMPTRAP;

    /*assign input*/
    msg_data_p->data.ui32_v=owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnknownIPMcastFwdPortList
 * -------------------------------------------------------------------------
 * FUNCTION: Set the unknown multicast packet forwarding-to port list.
 * INPUT   : port_list  - on which the multicast packets allow to forward-to
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : To determine which ports are allow to forward the unknow IPMC
 *           packets.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnknownIPMcastFwdPortList(UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.port_list)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKNOWNIPMCASTFWDPORTLIST;

    /*assign input*/
    memcpy(msg_data_p->data.port_list,port_list,sizeof(msg_data_p->data.port_list));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBStormControlRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the rate limit of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which mode of rate
 *           nRate   -- rate of broadcast amount
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBStormControlRateLimit(UI32_T ifindex,
                                        UI32_T mode,
                                        UI32_T nRate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBSTORMCONTROLRATELIMIT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=mode;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=nRate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMStormControlRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the rate limit of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which mode of rate
 *           nRate   -- rate of multicast amount
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMStormControlRateLimit(UI32_T ifindex,
                                        UI32_T mode,
                                        UI32_T nRate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMSTORMCONTROLRATELIMIT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=mode;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=nRate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBroadcastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the broadcast storm control function
 * INPUT   : ifindex -- which port to set
 *           broadcast_storm_status -- VAL_bcastStormStatus_enabled
 *                                     VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBroadcastStormStatus(UI32_T ifindex, UI32_T broadcast_storm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBROADCASTSTORMSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=broadcast_storm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMulticastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the multicast storm control function
 * INPUT   : ifindex -- which port to set
 *           multicast_storm_status -- VAL_bcastStormStatus_enabled
 *                                     VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMulticastStormStatus(UI32_T ifindex, UI32_T multicast_storm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMULTICASTSTORMSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=multicast_storm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnknownUStormControlRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the rate limit of unknown unicast(DLF)
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which mode of rate
 *           nRate   -- rate of multicast amount
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnknownUStormControlRateLimit(UI32_T ifindex,
                                               UI32_T mode,
                                               UI32_T nRate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKNOWNUSTORMCONTROLRATELIMIT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=mode;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=nRate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnknownUnicastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the unknown unicast(DLF) storm control function
 * INPUT   : ifindex -- which port to set
 *           multicast_storm_status -- VAL_unknownUcastStormStatus_enabled
 *                                     VAL_unknownUcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnknownUnicastStormStatus(UI32_T ifindex, UI32_T unknown_unicast_storm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKNOWNUNICASTSTORMSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=unknown_unicast_storm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

#if (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetGlobalStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will set global storm sample type
 * INPUT   : ifindex                        - interface index
 *           global_storm_sample_type       - VAL_stormSampleType_pkt_rate
 *                                            VAL_stormSampleType_octet_rate
 *                                            VAL_stormSampleType_percent
 *                                            0 for default value
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetGlobalStormSampleType(UI32_T global_storm_sample_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETGLOBALSTORMSAMPLETYPE;

    /*assign input*/
    msg_data_p->data.ui32_v = global_storm_sample_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif /* (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE) */

/****************************************************************************/
/* Auto Traffic Control Broadcast/Multicast Storm Control                   */
/****************************************************************************/
#if (SYS_CPNT_ATC_BSTORM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 1.
 * FUNCTION: This function will set the auto traffic control on status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_on_status    -- which status of auto traffic control on
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlOnStatus
(UI32_T ifindex,
UI32_T  auto_traffic_control_on_status
)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCAST;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=auto_traffic_control_on_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();

    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 3.
 * FUNCTION: This function will set the auto traffic control release status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_release_status    -- which status of auto traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T auto_traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTAUTOCONTROLRELEASE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=auto_traffic_control_release_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 7.
 * FUNCTION: This function will set the traffic control release status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_release_status    -- which status of traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTCONTROLRELEASE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=traffic_control_release_status;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormTrafficControlOnTimer
 * -------------------------------------------------------------------------
 * 9.
 * FUNCTION: This function will set the auto traffic control on status of broadcast
 *           storm control function
 * INPUT   : traffic_control_on_timer    -- which status of traffic control on timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrafficControlOnTimer(UI32_T traffic_control_on_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTAPPLYTIME;

    /*assign input*/
    msg_data_p->data.ui32_v=traffic_control_on_timer;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrafficControlOnTimer(UI32_T *traffic_control_on_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETBCASTAPPLYTIME;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
        *traffic_control_on_timer = msg_data_p->data.ui32_v;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseTimer
 * -------------------------------------------------------------------------
 * 11.
 * FUNCTION: This function will set the traffic control release timer of broadcast
 *           storm control function
 * INPUT   : traffic_control_release_timer    -- which status of traffic control release timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseTimer(UI32_T traffic_control_release_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTRELEASETIME;

    /*assign input*/
    msg_data_p->data.ui32_v=traffic_control_release_timer;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrafficControlReleaseTimer(UI32_T *traffic_control_release_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETBCASTRELEASETIME;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
        *traffic_control_release_timer = msg_data_p->data.ui32_v;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThreshold
 * -------------------------------------------------------------------------
 * 13.
 * FUNCTION: This function will set the storm alarm fire threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThreshold(UI32_T ifindex, UI32_T storm_alarm_threshold)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTALARMFIRETHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=storm_alarm_threshold;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormStormClearThreshold
 * -------------------------------------------------------------------------
 * 15.
 * FUNCTION: This function will set the storm alarm clear threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_clear_threshold    -- which status of storm alarm clear threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormStormClearThreshold(UI32_T ifindex, UI32_T storm_clear_threshold)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //req_size=msg_buf_size;
    //resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTALARMCLEARTHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=storm_clear_threshold;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThresholdEx
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the storm alarm fire/clear threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 *                                       0 indicates unchanged.
 *           storm_clear_threshold    -- which status of storm alarm clear threshold
 *                                       0 indicates unchanged.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThresholdEx(UI32_T ifindex, UI32_T storm_alarm_threshold, UI32_T storm_clear_threshold)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //req_size=msg_buf_size;
    //resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTALARMTHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=storm_alarm_threshold;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=storm_clear_threshold;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCBroadcastStormAction
 * -------------------------------------------------------------------------
 * 25.
 * FUNCTION: This function will set the action method of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           action    -- which status of action
 *                            --- 1. rate-control , 2. shutdown
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormAction(UI32_T ifindex, UI32_T action)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_BCASTACTION;
    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=action;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetATCBroadcastStormEntry
 * -------------------------------------------------------------------------
 * 27.
 * FUNCTION: This function will get the entry of broadcast
 *           storm control function
 * INPUT   : atc_broadcast_storm_entry    -- which status of entry
 * OUTPUT  : atc_broadcast_storm_entry    -- which status of entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormEntry(SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry)
{
    UI32_T rv = 0;
    const UI32_T msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE +
                            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.get_atc_bcast_entry);
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)space_msg;
    UI32_T req_size,resp_size;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;

    //req_size=sizeof(msg_data_p->data.get_atc_bcast_entry.port)
    //    +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //resp_size=msg_size;
    req_size= msg_size;
    resp_size=msg_size;

    memset(space_msg, 0, sizeof(space_msg));
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    /* assign req cmd*/
    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETBCASTSTORMENTRY;

    /* assgin req data */

    msg_data_p->data.get_atc_bcast_entry.port = atc_broadcast_storm_entry->atc_broadcast_storm_ifindex;
    /* Send IPC */
    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, rv);
        return FALSE;
    }

    /* prepare resp msg */
    *atc_broadcast_storm_entry = msg_data_p->data.get_atc_bcast_entry.bcast_entry;
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetATCBroadcastStormTimer
 * -------------------------------------------------------------------------
 * 29.
 * FUNCTION: This function will set the tc apply timer and tc release timer on status of broadcast
 *           storm control function
 * INPUT   : a_broadcast_storm_timer    -- which status of tc apply timer and tc release apply
 * OUTPUT  : a_broadcast_storm_timer    -- which status of tc apply timer and tc release apply
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTimer(SWCTRL_ATCBroadcastStormTimer_T *a_broadcast_storm_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.atc_storm_timer)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETBCASTTIMEER;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
      *a_broadcast_storm_timer = msg_data_p->data.atc_storm_timer.bcast_timer;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCBroadcastStormEntry(SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry)
{
    UI32_T rv = 0;
    const UI32_T msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE +
                            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.get_atc_bcast_entry);
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)space_msg;
    UI32_T req_size,resp_size;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;

   // req_size=sizeof(msg_data_p->data.get_atc_bcast_entry.port)
   //     +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //resp_size=msg_size;

    req_size=msg_size;
    resp_size=msg_size;

    memset(space_msg, 0, sizeof(space_msg));
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    /* assign req cmd*/
    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGBCASTSTORMENTRY;

    /* assgin req data */

    msg_data_p->data.get_atc_bcast_entry.port = atc_broadcast_storm_entry->atc_broadcast_storm_ifindex;
    /* Send IPC */
    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, rv);
        return FALSE;
    }

    /* prepare resp msg */
    *atc_broadcast_storm_entry = msg_data_p->data.get_atc_bcast_entry.bcast_entry;
    return msg_data_p->type.result_ui32;

}

SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCBroadcastStormTimer(SWCTRL_ATCBroadcastStormTimer_T *a_broadcast_storm_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.atc_storm_timer)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGBCASTTIMEER;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
      *a_broadcast_storm_timer = msg_data_p->data.atc_storm_timer.bcast_timer;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;

}


BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T trap_storm_alarm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapBStormAlarmStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_storm_alarm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T *trap_storm_alarm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapBStormAlarmStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_storm_alarm_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapStormClearStatus(UI32_T ifindex, UI32_T trap_storm_clear_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapBStormClearStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_storm_clear_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapStormClearStatus(UI32_T ifindex, UI32_T *trap_storm_clear_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapBStormClearStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_storm_clear_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T trap_traffic_control_on_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapBStormTCOnStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_traffic_control_on_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T *trap_traffic_control_on_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapBStormTCOnStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_traffic_control_on_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T trap_traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapBStormTCReleaseStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_traffic_control_release_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *trap_traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapBStormTCReleaseStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_traffic_control_release_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


BOOL_T SWCTRL_PMGR_SetATCBroadcastStormSampleType(UI32_T ifindex, UI32_T atc_multicast_storm_sample_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetBStormSampleType;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=atc_multicast_storm_sample_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_GetNextATCBroadcastStormEntry(SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry)
{
    UI32_T rv = 0;
    const UI32_T msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE +
                            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.get_atc_bcast_entry);
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)space_msg;
    UI32_T req_size,resp_size;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;

   // req_size=sizeof(msg_data_p->data.get_atc_mcast_entry.port)
    //    +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
   // resp_size=msg_size;

    req_size=msg_size;
    resp_size=msg_size;

    memset(space_msg, 0, sizeof(space_msg));
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    /* assign req cmd*/
    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetNextATCBStormEntry;

    /* assgin req data */

    msg_data_p->data.get_atc_bcast_entry.port = atc_broadcast_storm_entry->atc_broadcast_storm_ifindex;
    /* Send IPC */
    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, rv);
        return FALSE;
    }

    /* prepare resp msg */
    *atc_broadcast_storm_entry = msg_data_p->data.get_atc_bcast_entry.bcast_entry;
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_DisableBStormAfterClearThreshold(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_DisableBStormAfterSetClearThreshold;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif /* End of SYS_CPNT_ATC_BSTORM */

#if (SYS_CPNT_ATC_MSTORM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 1.
 * FUNCTION: This function will set the auto traffic control on status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_on_status    -- which status of auto traffic control on
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlOnStatus(UI32_T ifindex, UI32_T auto_traffic_control_on_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCAST;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=auto_traffic_control_on_status;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 3.
 * FUNCTION: This function will set the auto traffic control release status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_release_status    -- which status of auto traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T auto_traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTAUTOCONTROLRELEASE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=auto_traffic_control_release_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 7.
 * FUNCTION: This function will set the traffic control release status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_release_status    -- which status of traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTCONTROLRELEASE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=traffic_control_release_status;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormTrafficControlOnTimer
 * -------------------------------------------------------------------------
 * 9.
 * FUNCTION: This function will set the auto traffic control on status of multicast
 *           storm control function
 * INPUT   : traffic_control_on_timer    -- which status of traffic control on timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrafficControlOnTimer(UI32_T traffic_control_on_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTAPPLYTIME;

    /*assign input*/
    msg_data_p->data.ui32_v=traffic_control_on_timer;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrafficControlOnTimer(UI32_T *traffic_control_on_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETMCASTAPPLYTIME;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
        *traffic_control_on_timer = msg_data_p->data.ui32_v;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseTimer
 * -------------------------------------------------------------------------
 * 11.
 * FUNCTION: This function will set the traffic control release timer of multicast
 *           storm control function
 * INPUT   : traffic_control_release_timer    -- which status of traffic control release timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseTimer(UI32_T traffic_control_release_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTRELEASETIME;

    /*assign input*/
    msg_data_p->data.ui32_v=traffic_control_release_timer;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrafficControlReleaseTimer(UI32_T *traffic_control_release_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETMCASTRELEASETIME;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
        *traffic_control_release_timer = msg_data_p->data.ui32_v;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormStormAlarmThreshold
 * -------------------------------------------------------------------------
 * 13.
 * FUNCTION: This function will set the storm alarm fire threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormStormAlarmThreshold(UI32_T ifindex, UI32_T storm_alarm_threshold)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTALARMFIRETHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=storm_alarm_threshold;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormStormClearThreshold
 * -------------------------------------------------------------------------
 * 15.
 * FUNCTION: This function will set the storm alarm clear threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_clear_threshold    -- which status of storm alarm clear threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormStormClearThreshold(UI32_T ifindex, UI32_T storm_clear_threshold)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTALARMCLEARTHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=storm_clear_threshold;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormStormAlarmThresholdEx
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the storm alarm fire/clear threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 *                                       0 indicates unchanged.
 *           storm_clear_threshold    -- which status of storm alarm clear threshold
 *                                       0 indicates unchanged.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormStormAlarmThresholdEx(UI32_T ifindex, UI32_T storm_alarm_threshold, UI32_T storm_clear_threshold)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //req_size=msg_buf_size;
    //resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTALARMTHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=storm_alarm_threshold;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=storm_clear_threshold;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetATCMulticastStormAction
 * -------------------------------------------------------------------------
 * 25.
 * FUNCTION: This function will set the action method of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           action    -- which status of action
 *                            --- 1. rate-control , 2. shutdown
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetATCMulticastStormAction(UI32_T ifindex, UI32_T action)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=msg_buf_size;
    //resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_MCASTACTION;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=action;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetATCMulticastStormEntry
 * -------------------------------------------------------------------------
 * 27.
 * FUNCTION: This function will get the entry of multicast
 *           storm control function
 * INPUT   : atc_multicast_storm_entry    -- which status of entry
 * OUTPUT  : atc_multicast_storm_entry    -- which status of entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetATCMulticastStormEntry(SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry)
{
    UI32_T rv = 0;
    const UI32_T msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE +
                            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.get_atc_mcast_entry);
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)space_msg;
    UI32_T req_size,resp_size;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;

   // req_size=sizeof(msg_data_p->data.get_atc_mcast_entry.port)
    //    +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
   // resp_size=msg_size;

    req_size=msg_size;
    resp_size=msg_size;

    memset(space_msg, 0, sizeof(space_msg));
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    /* assign req cmd*/
    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETMCASTSTORMENTRY;

    /* assgin req data */

    msg_data_p->data.get_atc_mcast_entry.port = atc_multicast_storm_entry->atc_multicast_storm_ifindex;
    /* Send IPC */
    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, rv);
        return FALSE;
    }

    /* prepare resp msg */
    *atc_multicast_storm_entry = msg_data_p->data.get_atc_mcast_entry.mcast_entry;
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetATCMulticastStormTimer
 * -------------------------------------------------------------------------
 * 29.
 * FUNCTION: This function will get the tc apply timer and tc release timer on status of multicast
 *           storm control function
 * INPUT   : a_multicast_storm_timer    -- which status of tc apply timer and tc release apply
 * OUTPUT  : a_multicast_storm_timer    -- which status of tc apply timer and tc release apply
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTimer(SWCTRL_ATCMulticastStormTimer_T *a_multicast_storm_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.atc_storm_timer)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETMCASTTIMEER;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
      *a_multicast_storm_timer = msg_data_p->data.atc_storm_timer.mcast_timer;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCMulticastStormEntry(SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry)
{
    UI32_T rv = 0;
    const UI32_T msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE +
                            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.get_atc_mcast_entry);
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)space_msg;
    UI32_T req_size,resp_size;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;

   // req_size=sizeof(msg_data_p->data.get_atc_mcast_entry.port)
    //    +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
   // resp_size=msg_size;

    req_size=msg_size;
    resp_size=msg_size;

    memset(space_msg, 0, sizeof(space_msg));
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    /* assign req cmd*/
    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGMCASTSTORMENTRY;

    /* assgin req data */

    msg_data_p->data.get_atc_mcast_entry.port = atc_multicast_storm_entry->atc_multicast_storm_ifindex;
    /* Send IPC */
    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, rv);
        return FALSE;
    }

    /* prepare resp msg */
    *atc_multicast_storm_entry = msg_data_p->data.get_atc_mcast_entry.mcast_entry;
    return msg_data_p->type.result_ui32;

}

SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCMulticastStormTimer(SWCTRL_ATCMulticastStormTimer_T *a_multicast_storm_timer)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.atc_storm_timer)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    resp_size=msg_buf_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGMCASTTIMEER;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
      *a_multicast_storm_timer = msg_data_p->data.atc_storm_timer.mcast_timer;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;

}


BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T trap_storm_alarm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapMStormAlarmStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_storm_alarm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T *trap_storm_alarm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapMStormAlarmStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_storm_alarm_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapStormClearStatus(UI32_T ifindex, UI32_T trap_storm_clear_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapMStormClearStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_storm_clear_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapStormClearStatus(UI32_T ifindex, UI32_T *trap_storm_clear_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapMStormClearStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_storm_clear_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T trap_traffic_control_on_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapMStormTCOnStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_traffic_control_on_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T *trap_traffic_control_on_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapMStormTCOnStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_traffic_control_on_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T trap_traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetTrapMStormTCReleaseStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=trap_traffic_control_release_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *trap_traffic_control_release_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetTrapMStormTCReleaseStatus;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
        *trap_traffic_control_release_status = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


BOOL_T SWCTRL_PMGR_SetATCMulticastStormSampleType(UI32_T ifindex, UI32_T atc_multicast_storm_sample_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_SetMStormSampleType;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=atc_multicast_storm_sample_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_GetNextATCMulticastStormEntry(SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry)
{
    UI32_T rv = 0;
    const UI32_T msg_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE +
                            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.get_atc_mcast_entry);
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)space_msg;
    UI32_T req_size,resp_size;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;

   // req_size=sizeof(msg_data_p->data.get_atc_mcast_entry.port)
    //    +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
   // resp_size=msg_size;

    req_size=msg_size;
    resp_size=msg_size;

    memset(space_msg, 0, sizeof(space_msg));
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    /* assign req cmd*/
    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_GetNextATCMStormEntry;

    /* assgin req data */

    msg_data_p->data.get_atc_mcast_entry.port = atc_multicast_storm_entry->atc_multicast_storm_ifindex;
    /* Send IPC */
    if((rv = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p))!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SendRequestMsg fail! (%ld)\n", __FUNCTION__, rv);
        return FALSE;
    }

    /* prepare resp msg */
    *atc_multicast_storm_entry = msg_data_p->data.get_atc_mcast_entry.mcast_entry;
    return msg_data_p->type.result_bool;

}

BOOL_T SWCTRL_PMGR_DisableMStormAfterClearThreshold(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_DisableMStormAfterSetClearThreshold;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



#endif /* End of SYS_CPNT_ATC_MSTORM */

UI32_T SWCTRL_PMGR_GetNextLogicalPort(UI32_T *l_port)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //resp_size=msg_buf_size;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;


    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETNEXTLOGICALPORT;

    /*assign input*/
    msg_data_p->data.ui32_v = *l_port;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *l_port = msg_data_p->data.ui32_v;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;

}

BOOL_T SWCTRL_PMGR_IsManagementPort(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //resp_size=msg_buf_size;

    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ISMANAGEMENTPORT;

    /*assign input*/
    msg_data_p->data.ui32_v = ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

UI32_T SWCTRL_PMGR_LogicalPortToUserPort(UI32_T ifindex,
                                                 UI32_T *unit,
                                                 UI32_T *port,
                                                 UI32_T *trunk_id)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    //req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    //resp_size=msg_buf_size;
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_LOGICALPORTTOUSERPORT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
      *unit = msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2;
      *port = msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3;
      *trunk_id = msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4;
    return msg_data_p->type.result_ui32;

}



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortUserDefaultPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set user default priority
 * INPUT   : ifindex  -- which port to set
 *           priority -- user default priority to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortUserDefaultPriority(UI32_T ifindex, UI32_T priority)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTUSERDEFAULTPRIORITY;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=priority;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPriorityMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of 10/100 ports
 * INPUT   : ifindex -- which port to set
 *           mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPriorityMapping(UI32_T ifindex, UI8_T mapping[8])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ifindex_mapping)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPRIORITYMAPPING;

    /*assign input*/
    msg_data_p->data.ifindex_mapping.ifindex=ifindex;
    memcpy(msg_data_p->data.ifindex_mapping.mapping,
        mapping,sizeof(msg_data_p->data.ifindex_mapping.mapping));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPriorityMappingPerSystem
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of per-system
 * INPUT   : mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPriorityMappingPerSystem(UI8_T mapping[8])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.mapping)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPRIORITYMAPPINGPERSYSTEM;

    /*assign input*/
    memcpy(msg_data_p->data.mapping,mapping,sizeof(msg_data_p->data.mapping));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable TOS/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_PMGR_DisableTosCosMap()
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=0;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLETOSCOSMAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_PMGR_EnableDscpCosMap()
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=0;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEDSCPCOSMAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_PMGR_DisableDscpCosMap()
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=0;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEDSCPCOSMAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
    }
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign output*/
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of TOS/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           tos          -- from which tos value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortTosCosMap(UI32_T ifindex, UI32_T tos, UI32_T cos_priority)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTTOSCOSMAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=tos;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=cos_priority;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           dscp         -- from which dscp value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDscpCosMap(UI32_T ifindex, UI32_T dscp, UI32_T cos_priority)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTDSCPCOSMAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=dscp;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=cos_priority;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of TCP_PORT/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           tcp_port     -- from which dscp value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortTcpPortCosMap(UI32_T ifindex, UI32_T tcp_port, UI32_T cos_priority)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTTCPPORTCOSMAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=tcp_port;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=cos_priority;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DelPortTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete one entry of TOS/COS mapping of a port
 * INPUT   : ifindex      -- which port to delete
 *           tos         -- which tos mapping will be deleted
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DelPortTosCosMap(UI32_T ifindex, UI32_T tos)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELPORTTOSCOSMAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=tos;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DelPortDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to delete
 *           dscp         -- which dscp mapping will be deleted
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DelPortDscpCosMap(UI32_T ifindex, UI32_T dscp)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELPORTDSCPCOSMAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=dscp;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DelPortTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to delete
 *           tcp_port     -- which tcp port mapping will be deleted
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DelPortTcpPortCosMap(UI32_T ifindex, UI32_T tcp_port)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELPORTTCPPORTCOSMAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=tcp_port;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
/*********************
 * Private VLAN APIs *
 *********************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePrivateVlan()
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEPRIVATEVLAN;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePrivateVlan()
{
    const UI32_T msg_buf_size=( SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEPRIVATEVLAN;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan
 * INPUT   : uplink_port_list  -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPrivateVlan(UI8_T uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                             UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.uplinkportlist_downlinkportlist)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPRIVATEVLAN;

    /*assign input*/
    memcpy(msg_data_p->data.uplinkportlist_downlinkportlist.uplink_port_list,
        uplink_port_list,sizeof(msg_data_p->data.uplinkportlist_downlinkportlist.uplink_port_list));
    memcpy(msg_data_p->data.uplinkportlist_downlinkportlist.downlink_port_list,
        downlink_port_list,sizeof(msg_data_p->data.uplinkportlist_downlinkportlist.downlink_port_list));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPrivateMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private mode for the specified port
 * INPUT   : ifindex            -- the specified port index
 *           port_private_mode  -- VAL_portPrivateMode_enabled (1L) : private port
 *                                 VAL_portPrivateMode_disabled (2L): public port
 * OUTPU   : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortPrivateMode(UI32_T ifindex, UI32_T port_private_mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTPRIVATEMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=port_private_mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */
#endif

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
/********************************************
 * Multi-Session of Private VLAN APIs       *
 *******************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPrivateVlanBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan by group session id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPrivateVlanBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.uplinkportlist_downlinkportlist)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPRIVATEVLANBYSESSIONID;

    /*assign input*/
    msg_data_p->data.uplinkportlist_downlinkportlist.session_id = session_id;
    memcpy(msg_data_p->data.uplinkportlist_downlinkportlist.uplink_port_list,
        uplink_port_list,sizeof(msg_data_p->data.uplinkportlist_downlinkportlist.uplink_port_list));
    memcpy(msg_data_p->data.uplinkportlist_downlinkportlist.downlink_port_list,
        downlink_port_list,sizeof(msg_data_p->data.uplinkportlist_downlinkportlist.downlink_port_list));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete private vlan port list with sesion Id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.uplinkportlist_downlinkportlist)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELETEPRIVATEVLANPORTLISTBYSESSIONID;

    /*assign input*/
    msg_data_p->data.uplinkportlist_downlinkportlist.session_id = session_id;
    memcpy(msg_data_p->data.uplinkportlist_downlinkportlist.uplink_port_list,
        uplink_port_list,sizeof(msg_data_p->data.uplinkportlist_downlinkportlist.uplink_port_list));
    memcpy(msg_data_p->data.uplinkportlist_downlinkportlist.downlink_port_list,
        downlink_port_list,sizeof(msg_data_p->data.uplinkportlist_downlinkportlist.downlink_port_list));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DestroyPrivateVlanSession
 * -------------------------------------------------------------------------
 * FUNCTION: Destroy entire Private VLAN session
 * INPUT   : session_id   -- pvlan group id
 *           is_uplink    -- is uplink port
 *           is_downlink  -- is downlink port
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DestroyPrivateVlanSession(UI32_T session_id, BOOL_T is_uplink, BOOL_T is_downlink)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DESTROYPRIVATEVLANSESSION;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = session_id;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = is_uplink;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = is_downlink;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable blocking traffic of uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode()
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEPRIVATEVLANUPLINKTOUPLINKBLOCKINGMODE;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable blocking traffic of uplink ports
 *           so every traffic can be forwarding different uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode()
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEPRIVATEVLANUPLINKTOUPLINKBLOCKINGMODE;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port ingress rate limit
 * INPUT   : ifindex -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePortIngressRateLimit(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEPORTINGRESSRATELIMIT;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port ingress rate limit
 * INPUT   : ifindex -- which port to disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePortIngressRateLimit(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEPORTINGRESSRATELIMIT;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port ingress rate limit
 * INPUT   : ifindex -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : # kbits
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortIngressRateLimit(UI32_T ifindex, UI32_T rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTINGRESSRATELIMIT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#endif

#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port Egress rate limit
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePortEgressRateLimit(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEPORTEGRESSRATELIMIT;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port Egress rate limit
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePortEgressRateLimit(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEPORTEGRESSRATELIMIT;

    /*assign input*/
    msg_data_p->data.ui32_v=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port egress rate limit
 * INPUT   : ifindex -- which port to set
 *           rate -- port egress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : # kbits
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortEgressRateLimit(UI32_T ifindex, UI32_T rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTEGRESSRATELIMIT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif

#if (SYS_CPNT_JUMBO_FRAMES == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetJumboFrameStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the jumbo frame
 * INPUT   : jumbo_frame_status --  SWCTRL_JUMBO_FRAME_ENABLE
 *                                  SWCTRL_JUMBO_FRAME_DISABLE
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetJumboFrameStatus (UI32_T jumbo_frame_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETJUMBOFRAMESTATUS;

    /*assign input*/
    msg_data_p->data.ui32_v=jumbo_frame_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif

#if 0 /* deprecated */
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the egress schedulering method
 * INPUT:    mode
 *               SWCTRL_WEIGHT_ROUND_ROBIN_METHOD
 *               SWCTRL_STRICT_PRIORITY_METHOD,
 *               SWCTRL_DEFICIT_ROUND_RBIN_METHOD,
 *               SWCTRL_SP_WRR_METHOD,
 *               SWCTRL_SP_DRR_METHOD
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPrioQueueMode(UI32_T mode)
{
    UI32_T lport = 0;

    while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        return SWCTRL_PMGR_SetPortPrioQueueMode(lport, mode);
    }

    return FALSE;
}
#endif /* (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the port egress schedulering method
 * INPUT:    l_port  -- which port to set
 *        mode  -- SWCTRL_WEIGHT_ROUND_ROBIN_METHOD
                SWCTRL_STRICT_PRIORITY_METHOD,
                SWCTRL_DEFICIT_ROUND_RBIN_METHOD,
                SWCTRL_SP_WRR_METHOD,
                SWCTRL_SP_DRR_METHOD
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPrioQueueMode(UI32_T l_port, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTPRIOQUEUEMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=l_port;
    msg_data_p->data.u32a1_u32a2.u32_a2=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T weight)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTWRRQUEUEWEIGHT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=l_port;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=q_id;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=weight;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetWrrQueueWeight(UI32_T q_id, UI32_T weight)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETWRRQUEUEWEIGHT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=q_id;
    msg_data_p->data.u32a1_u32a2.u32_a2=weight;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set name to a port
 * INPUT   : ifindex    -- which port to set
 *           port_name  -- the name to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/portMgt 1
 *           size (0..64)
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortName(UI32_T ifindex, UI8_T *port_name)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ifindex_portname)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTNAME;

    /*assign input*/
    msg_data_p->data.ifindex_portname.ifindex=ifindex;
    memcpy(msg_data_p->data.ifindex_portname.port_name,
        port_name,sizeof(msg_data_p->data.ifindex_portname));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAlias
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set alias to a port
 * INPUT   : ifindex    -- which port to set
 *           port_alias  -- the alias to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/portMgt 1
 *           size (0..64)
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAlias(UI32_T ifindex, UI8_T *port_alias)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ifindex_portname)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTALIAS;

    /*assign input*/
    msg_data_p->data.ifindex_portname.ifindex=ifindex;
    memcpy(msg_data_p->data.ifindex_portname.port_name,
        port_alias,sizeof(msg_data_p->data.ifindex_portname));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSpeedDpxCfg
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port speed and duplex mode
 * INPUT   : ifindex                        - interface index
 *           port_speed_dpx_cfg             - VAL_portSpeedDpxCfg_halfDuplex10
 *                                            VAL_portSpeedDpxCfg_fullDuplex10
 *                                            VAL_portSpeedDpxCfg_halfDuplex100
 *                                            VAL_portSpeedDpxCfg_fullDuplex100
 *                                            VAL_portSpeedDpxCfg_halfDuplex1000 <== no support
 *                                            VAL_portSpeedDpxCfg_fullDuplex1000
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSpeedDpxCfg(UI32_T ifindex, UI32_T port_speed_dpx_cfg)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSPEEDDPXCFG;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=port_speed_dpx_cfg;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortFlowCtrlCfg
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port flow control mechanism
 * INPUT   : ifindex                        - interface index
 *           port_flow_ctrl_cfg             - VAL_portFlowCtrlCfg_enabled
 *                                            VAL_portFlowCtrlCfg_disabled
 *                                            VAL_portFlowCtrlCfg_backPressure
 *                                            VAL_portFlowCtrlCfg_dot3xFlowControl
 *                                            VAL_portFlowCtrlCfg_tx
 *                                            VAL_portFlowCtrlCfg_rx
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortFlowCtrlCfg(UI32_T ifindex, UI32_T port_flow_ctrl_cfg)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTFLOWCTRLCFG;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=port_flow_ctrl_cfg;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCapabilities
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port capabilities
 * INPUT   : ifindex                        - interface index
 *           port_capabilities              - bitmap to set capability
 *
 *          SYS_VAL_portCapabilities_portCap10half          BIT_0
 *          SYS_VAL_portCapabilities_portCap10full          BIT_1
 *          SYS_VAL_portCapabilities_portCap100half         BIT_2
 *          SYS_VAL_portCapabilities_portCap100full         BIT_3
 *          SYS_VAL_portCapabilities_portCap1000half        BIT_4 <== not support
 *          SYS_VAL_portCapabilities_portCap1000full        BIT_5
 *          SYS_VAL_portCapabilities_portCapSym             BIT_14
 *          SYS_VAL_portCapabilities_portCapFlowCtrl        BIT_15
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 * Usage   : set 10half and 100full ==>
 *     bitmap = SYS_VAL_portCapabilities_portCap10half | SYS_VAL_portCapabilities_portCap100full
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCapabilities(UI32_T ifindex, UI32_T port_capabilities)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTCAPABILITIES;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=port_capabilities;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAutonegotiation
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port autonegotiation
 * INPUT   : ifindex                        - interface index
 *           port_autonegotiation           - VAL_portAutonegotiation_enabled
 *                                            VAL_portAutonegotiation_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAutonegotiation(UI32_T ifindex, UI32_T port_autonegotiation)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTAUTONEGOTIATION;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=port_autonegotiation;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMirrorType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the mirror type
 * INPUT   : ifindex_src   -- which ifindex to mirror
 *          ifindex_dest  -- which ifindex mirrors the received/transmitted packets
 *          port_autonegotiation           - VAL_mirrorType_rx
 *                                           VAL_mirrorType_tx
 *                                           VAL_mirrorType_both
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMirrorType(UI32_T ifindex_src, UI32_T ifindex_dest, UI32_T mirror_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMIRRORTYPE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex_src;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=ifindex_dest;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=mirror_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMirrorStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy the mirroring function
 * INPUT   : ifindex_src   -- which ifindex to mirror
 *           ifindex_dext  -- which ifindex mirrors the received/transmitted packets
 * OUTPUT  : mirror_status -- VAL_mirrorStatus_valid / VAL_mirrorStatus_invalid
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. ES3626A MIB/mirrorMgt 1
 *           2. No matter support SYS_CPNT_ALLOW_DUMMY_MIRRORING_DEST or not
 *              source == 0 is not valid.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMirrorStatus(UI32_T ifindex_src , UI32_T ifindex_dest, UI32_T mirror_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMIRRORSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex_src;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=ifindex_dest;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=mirror_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#if (SYS_CPNT_VLAN_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetVlanMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the vlan mirror table entry info
 * INPUT   : vlan_mirror_entry->mirror_dest_port        - mirror destination port
 *           vlan_mirror_entry->mirror_source_vlan      - mirror source vlan id
 * OUTPUT  : vlan_mirror_entry                          - The vlan mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : The input keys (ifindex & vid) will get current vlan entry.
 *           if specifies a mirror_dest_port = 0 , so return system dest port
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetVlanMirrorEntry(SWCTRL_VlanMirrorEntry_T *vlan_mirror_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    if(vlan_mirror_entry==NULL)
        return FALSE;

    /*assign size*/
    req_size=sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETVLANMIRRORENTRY;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=vlan_mirror_entry->mirror_dest_port;
    msg_data_p->data.u32a1_u32a2.u32_a2=vlan_mirror_entry->mirror_source_vlan;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    vlan_mirror_entry->mirror_dest_port=msg_data_p->data.u32a1_u32a2_u32a3.u32_a1;
    vlan_mirror_entry->mirror_source_vlan=msg_data_p->data.u32a1_u32a2_u32a3.u32_a2;
    vlan_mirror_entry->mirror_vlan_status=msg_data_p->data.u32a1_u32a2_u32a3.u32_a3;
    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextVlanMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next vlan mirror table entry info
 * INPUT   : vlan_mirror_entry->mirror_dest_port        - mirror destination port
 *           vlan_mirror_entry->mirror_source_vlan      - mirror source vlan id
 * OUTPUT  : vlan_mirror_entry                          - The vlan mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : The input key shall be contain vlan id and destination port, however
 *           the destination port can be specifies to 0, because we can get
 *           system destination port currently
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextVlanMirrorEntry(SWCTRL_VlanMirrorEntry_T *vlan_mirror_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    if(vlan_mirror_entry==NULL)
        return FALSE;

    /*assign size*/
    req_size=sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETNEXTVLANMIRRORENTRY;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=vlan_mirror_entry->mirror_dest_port;
    msg_data_p->data.u32a1_u32a2.u32_a2=vlan_mirror_entry->mirror_source_vlan;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    vlan_mirror_entry->mirror_dest_port=msg_data_p->data.u32a1_u32a2_u32a3.u32_a1;
    vlan_mirror_entry->mirror_source_vlan=msg_data_p->data.u32a1_u32a2_u32a3.u32_a2;
    vlan_mirror_entry->mirror_vlan_status=msg_data_p->data.u32a1_u32a2_u32a3.u32_a3;
    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will add the vlan mirror and desination port
 * INPUT   : vid           -- which vlan-id add to source mirrored table
 *           ifindex_dest  -- which ifindex-port received mirror packets
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be consistent whenever vlan-id created
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddVlanMirror(UI32_T vid, UI32_T ifindex_dest)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADDVLANMIRROR;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=vid;
    msg_data_p->data.u32a1_u32a2.u32_a2=ifindex_dest;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will delete the vlan mirror and destination port
 * INPUT   : vid           -- which vlan-id remove from source mirrored table
 *           ifindex_dest  -- which ifindex-port received mirror packets
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be removed when source vlan mirror has empty
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeleteVlanMirror(UI32_T vid, UI32_T ifindex_dest)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELETEVLANMIRROR;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=vid;
    msg_data_p->data.u32a1_u32a2.u32_a2=ifindex_dest;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set the MAC based MIRROR entry
 * INPUT    : ifindex_dest       -- destnation port
 *            mac_address        -- MAC address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMacMirrorEntry(UI32_T ifindex_dest, UI8_T *mac_address)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.mac_mirror_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMACMIRRORENTRY;

    /*assign input*/
    msg_data_p->data.mac_mirror_entry.ifindex_dest=ifindex_dest;
    memcpy(&(msg_data_p->data.mac_mirror_entry.mac), mac_address, sizeof(msg_data_p->data.mac_mirror_entry.mac));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will delete MAC based MIRROR entry
 * INPUT    : ifindex_dest       -- destnation port
 *            mac_address        -- MAC address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeleteMacMirrorEntry(UI32_T ifindex_dest, UI8_T *mac_address)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.mac_mirror_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DELETEMACMIRRORENTRY;

    /*assign input*/
    msg_data_p->data.mac_mirror_entry.ifindex_dest=ifindex_dest;
    memcpy(&(msg_data_p->data.mac_mirror_entry.mac), mac_address, sizeof(msg_data_p->data.mac_mirror_entry.mac));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /* end of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

#if (SYS_CPNT_ACL_BASED_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetAclMirrorDestPort
 *------------------------------------------------------------------------
 * FUNCTION: This function will setup dest port for  ACL-based mirror
 * INPUT   : ifindex_dest  -- which ifindex-port received mirror packets
 *           mirror_type   -- mirror type
 *                           (VAL_aclMirrorType_rx/VAL_aclMirrorType_tx/VAL_aclMirrorType_both)
 *           enable        -- TRUE to set, FALSE to remove
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetAclMirrorDestPort(UI32_T ifindex_dest, UI32_T mirror_type, BOOL_T enable)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETACLMIRRORDESTPORT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex_dest;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=mirror_type;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=enable;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the broadcast storm status
 * INPUT   : ifindex                      - interface index
 *           bcast_storm_status           - VAL_bcastStormStatus_enabled
 *                                          VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormStatus(UI32_T ifindex, UI32_T bcast_storm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBCASTSTORMSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=bcast_storm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the broadcast storm sample type
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_sample_type        - VAL_bcastStormSampleType_pkt_rate
 *                                            VAL_bcastStormSampleType_octet_rate
 *                                            VAL_bcastStormSampleType_percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormSampleType(UI32_T ifindex, UI32_T bcast_storm_sample_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBCASTSTORMSAMPLETYPE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=bcast_storm_sample_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_pkt_rate           - the broadcast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormPktRate(UI32_T ifindex, UI32_T bcast_storm_pkt_rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBCASTSTORMPKTRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=bcast_storm_pkt_rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_octet_rate         - the broadcast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormOctetRate(UI32_T ifindex, UI32_T bcast_storm_octet_rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBCASTSTORMOCTETRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=bcast_storm_octet_rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           bcast_storm_percent           - the broadcast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormPercent(UI32_T ifindex, UI32_T bcast_storm_percent)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETBCASTSTORMPERCENT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=bcast_storm_percent;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the multicast storm status
 * INPUT   : ifindex                      - interface index
 *           mcast_storm_status           - VAL_mcastStormStatus_enabled
 *                                          VAL_mcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormStatus(UI32_T ifindex, UI32_T mcast_storm_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMCASTSTORMSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mcast_storm_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the multicast storm sample type
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_sample_type        - VAL_mcastStormSampleType_pkt_rate
 *                                            VAL_mcastStormSampleType_octet_rate
 *                                            VAL_mcastStormSampleType_percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormSampleType(UI32_T ifindex, UI32_T mcast_storm_sample_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMCASTSTORMSAMPLETYPE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mcast_storm_sample_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_pkt_rate           - the multicast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormPktRate(UI32_T ifindex, UI32_T mcast_storm_pkt_rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMCASTSTORMPKTRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mcast_storm_pkt_rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_octet_rate         - the multicast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormOctetRate(UI32_T ifindex, UI32_T mcast_storm_octet_rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMCASTSTORMOCTETRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mcast_storm_octet_rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           mcast_storm_percent           - the multicast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormPercent(UI32_T ifindex, UI32_T mcast_storm_percent)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMCASTSTORMPERCENT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mcast_storm_percent;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnkucastStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the unknowunicast storm sample type
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_sample_type        - VAL_unkucastStormSampleType_pkt_rate
 *                                            VAL_unkucastStormSampleType_octet_rate
 *                                            VAL_unkucastStormSampleType_percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormSampleType(UI32_T ifindex, UI32_T unkucast_storm_sample_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMSAMPLETYPE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=unkucast_storm_sample_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnkucastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_pkt_rate           - the unknowunicast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormPktRate(UI32_T ifindex, UI32_T unkucast_storm_pkt_rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMPKTRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=unkucast_storm_pkt_rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnkucastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_octet_rate           - the unknowunicast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormOctetRate(UI32_T ifindex, UI32_T unkucast_storm_octet_rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMOCTETRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=unkucast_storm_octet_rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnkucastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           unkucast_storm_percent           - the unknowunicast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormPercent(UI32_T ifindex, UI32_T unkucast_storm_percent)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMPERCENT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=unkucast_storm_percent;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ShutdownSwitch
 *------------------------------------------------------------------------
 * FUNCTION: This function will shutdown the switch before warm start
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void SWCTRL_PMGR_ShutdownSwitch(void)
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=0;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SHUTDOWNSWITCH;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableIPMC(void)
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEIPMC;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableIPMC(void)
{
    const UI32_T msg_buf_size=( SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEIPMC;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DisableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown ip or mcast packet
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_DisableUMCASTIpTrap(void)
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEUMCASTIPTRAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will enable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown ip or mcast packet
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_EnableUMCASTIpTrap(void)
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEUMCASTIPTRAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DisableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown mac or mcast packet
 *              Aaron add, 2003/07/31
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_DisableUMCASTMacTrap(void)
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEUMCASTMACTRAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will enable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown mac or mcast packet
 *              Aaron add, 2003/07/31
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_EnableUMCASTMacTrap(void)
{
    const UI32_T msg_buf_size=(SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEUMCASTMACTRAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnitsBaseMacAddrTable
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to set the table of MAC address of each unit
 *           in CLI configuration file. This API should only be called by
 *           CLI and should be called before any command rovision.
 * INPUT   : mac_addr --- MAC addresses of all unit.
 *                        If some unit is not present the MAC address should
 *                        be 00-00-00-00-00-00.
 * OUTPUT  : None.
 * RETURN  : TRUE  --- Set table successfully.
 *           FALSE --- Set table fail.
 *                     1) Not in MASETR mode.
 *                     2) Provision has been already completed.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnitsBaseMacAddrTable(UI8_T mac_addr[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][6])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.mac_addrs)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNITSBASEMACADDRTABLE;

    /*assign input*/
    memcpy(msg_data_p->data.mac_addrs,
        mac_addr,sizeof(msg_data_p->data.mac_addrs));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnitsBaseMacAddrTable
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to set the table of device type of each unit
 *           in CLI configuration file. This API should only be called by
 *           CLI and should be called before any command rovision.
 * INPUT   : device_type --- Device Types of all unit.
 *                        If some unit is not present the device type should
 *                        be 0xffffffff.
 * OUTPUT  : None.
 * RETURN  : TRUE  --- Set table successfully.
 *           FALSE --- Set table fail.
 *                     1) Not in MASETR mode.
 *                     2) Provision has been already completed.
 * NOTE    : CLI shall call this API after call SWCTRL_SetUnitsBaseMacAddrTable()
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnitsDeviceTypeTable(UI32_T device_type[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.device_types)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETUNITSDEVICETYPETABLE;

    /*assign input*/
    memcpy(msg_data_p->data.device_types,
        device_type,sizeof(msg_data_p->data.device_types));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#if (SYS_CPNT_MAU_MIB == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set MAU status.
 * INPUT   : if_mau_ifindex  -- Which interface.
 *           if_mau_index    -- Which MAU.
 *           status          -- VAL_ifMauStatus_other
 *                              VAL_ifMauStatus_unknown
 *                              VAL_ifMauStatus_operational
 *                              VAL_ifMauStatus_standby
 *                              VAL_ifMauStatus_shutdown
 *                              VAL_ifMauStatus_reset
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauStatus (UI32_T if_mau_ifindex,
                              UI32_T if_mau_index,
                              UI32_T status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETIFMAUSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=if_mau_ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=if_mau_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauDefaultType
 * -------------------------------------------------------------------------
 * FUNCTION: Set the default MAU type.
 * INPUT   : if_mau_ifindex  -- Which interface.
 *           if_mau_index    -- Which MAU.
 *           default_type    -- Caller should use naming constant in SWCTRL_IF_MAU_TYPE_E.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauDefaultType (UI32_T if_mau_ifindex,
                                   UI32_T if_mau_index,
                                   UI32_T default_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETIFMAUDEFAULTTYPE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=if_mau_ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=if_mau_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=default_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set MAU auto-negoation admin status.
 * INPUT   : if_mau_ifindex        -- Which interface.
 *           if_mau_index          -- Which MAU.
 *           auto_neg_admin_status -- VAL_ifMauAutoNegAdminStatus_enabled
 *                                    VAL_ifMauAutoNegAdminStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegAdminStatus (UI32_T if_mau_ifindex,
                                          UI32_T if_mau_index,
                                          UI32_T auto_neg_admin_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGADMINSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=if_mau_ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=if_mau_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=auto_neg_admin_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegRestart
 * -------------------------------------------------------------------------
 * FUNCTION: Set MAU to auto-negoation restart.
 * INPUT   : if_mau_ifindex   -- Which interface.
 *           if_mau_index     -- Which MAU.
 *           auto_neg_restart -- VAL_ifMauAutoNegRestart_restart
                                 VAL_ifMauAutoNegRestart_norestart
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegRestart (UI32_T if_mau_ifindex,
                                      UI32_T if_mau_index,
                                      UI32_T auto_neg_restart)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGRESTART;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=if_mau_ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=if_mau_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=auto_neg_restart;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegCapAdvertisedBits
 * -------------------------------------------------------------------------
 * FUNCTION: Set advertised capability bits in the MAU.
 * INPUT   : if_mau_ifindex  -- Which interface.
 *           if_mau_index    -- Which MAU.
 *           auto_neg_cap_adv_bits -- (1 << VAL_ifMauAutoNegCapAdvertisedBits_bOther      )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b10baseT    )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b10baseTFD  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseT4  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseTX  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseTXFD)
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseT2  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseT2FD)
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxPause   )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxAPause  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxSPause  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxBPause  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseX  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseXFD)
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseT  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseTFD)
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegCapAdvertisedBits (UI32_T if_mau_ifindex,
                                                UI32_T if_mau_index,
                                                UI32_T auto_neg_cap_adv_bits)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGCAPADVERTISEDBITS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=if_mau_ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=if_mau_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=auto_neg_cap_adv_bits;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegRemoteFaultAdvertised
 * -------------------------------------------------------------------------
 * FUNCTION: Set advertised auto-negoation remote fault in the MAU.
 * INPUT   : if_mau_ifindex            -- Which interface.
 *           if_mau_index              -- Which MAU.
 *           auto_neg_remote_fault_adv -- VAL_ifMauAutoNegRemoteFaultAdvertised_noError
 *                                        VAL_ifMauAutoNegRemoteFaultAdvertised_offline
 *                                        VAL_ifMauAutoNegRemoteFaultAdvertised_linkFailure
 *                                        VAL_ifMauAutoNegRemoteFaultAdvertised_autoNegError
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegRemoteFaultAdvertised (UI32_T if_mau_ifindex,
                                                    UI32_T if_mau_index,
                                                    UI32_T auto_neg_remote_fault_adv)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGREMOTEFAULTADVERTISED;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=if_mau_ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=if_mau_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=auto_neg_remote_fault_adv;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold alarm trap
 * INPUT   : ifindex   -- which ifindex
 *           trap_enable -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSfpDdmTrapEnable(UI32_T ifindex, BOOL_T trap_enable)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);

    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTRAPENABLE;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1=ifindex;
    msg_data_p->data.u32a1_boola2.bool_a2=trap_enable;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold auto mode
 * INPUT   : ifindex   -- which ifindex
 *           auto_mode -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThresholdAutoMode(UI32_T ifindex, BOOL_T auto_mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);

    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLDAUTOMODE;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1=ifindex;
    msg_data_p->data.u32a1_boola2.bool_a2=auto_mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThresholdDefault
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold to default
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThresholdDefault(UI32_T ifindex, UI32_T threshold_type)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);

    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLDDEFAULT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=threshold_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *           val
 *           need_to_check_range -- when cli provision, no need to check range
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThreshold(UI32_T ifindex, UI32_T threshold_type, I32_T val, BOOL_T need_to_check_range)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_i32a3_bla4)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);

    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLD;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_i32a3_bla4.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_i32a3_bla4.u32_a2=threshold_type;
    msg_data_p->data.u32a1_u32a2_i32a3_bla4.i32_a3=val;
    msg_data_p->data.u32a1_u32a2_i32a3_bla4.bl_a4=need_to_check_range;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThresholdForWeb
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold for web
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *           high_alarm
 *           high_warning
 *           low_warning
 *           low_alarm
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThresholdForWeb(UI32_T ifindex, UI32_T threshold_type,
    I32_T high_alarm, I32_T high_warning, I32_T low_warning, I32_T low_alarm)
{
    const UI32_T msg_buf_size=
        (sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);

    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLDFORWEB;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6.u32_a2=threshold_type;
    msg_data_p->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6.i32_a3=high_alarm;
    msg_data_p->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6.i32_a4=high_warning;
    msg_data_p->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6.i32_a5=low_warning;
    msg_data_p->data.u32a1_u32a2_i32a3_i32a4_i32a5_i32a6.i32_a6=low_alarm;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif/* End of #if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE) */

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port force link media
 * INPUT   : ifindex     -- which port to set
 *           forcedmode  -- which mode of media
 *                      - VAL_portComboForcedMode_none
 *                              For trunk and non-combo port only.
 *                      - VAL_portComboForcedMode_copperForced
 *                              Force to copper more ignore SFP transceiver present state.
 *                      - VAL_portComboForcedMode_copperPreferredAuto
 *                              Obsoleted.
 *                      - VAL_portComboForcedMode_sfpForced
 *                              Force to fiber more ignore SFP transceiver present state.
 *                      - VAL_portComboForcedMode_sfpPreferredAuto
 *                              Copper/fiber depends on SFP transceiver present state.
 *                              SFP transceiver present       -> Fiber mode.
 *                                              not present   -> Copper more.
 *           fiber_speed  -- which speed (VAL_portType_hundredBaseFX/VAL_portType_thousandBaseSfp)
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk, trunk member, and normal port.
 * -------------------------------------------------------------------------*/
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
BOOL_T SWCTRL_PMGR_SetPortComboForcedMode(UI32_T ifindex, UI32_T forcedmode, UI32_T fiber_speed)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTCOMBOFORCEDMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=forcedmode;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=fiber_speed;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#else
BOOL_T SWCTRL_PMGR_SetPortComboForcedMode(UI32_T ifindex, UI32_T forcedmode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTCOMBOFORCEDMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=forcedmode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif
#endif

#if (SYS_CPNT_OSPF == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable OSPF trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableOSPFTrap(void)
{
    const UI32_T msg_buf_size=( SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEOSPFTRAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to disable OSPF trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableOSPFTrap(void)
{
    const UI32_T msg_buf_size=( SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLEOSPFTRAP;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortStateWithMstidx
 * -------------------------------------------------------------------------
 * PURPOSE  : Set the Stp port state
 * INPUT    : mstidx    -- multiple spanning tree instance index
 *            lport     -- ifindex of this logical port.
 *                         Only normal port and trunk port is allowed.
 *            state     -- port state 1) VAL_dot1dStpPortState_disabled
 *                                    2) VAL_dot1dStpPortState_blocking
 *                                    3) VAL_dot1dStpPortState_listening
 *                                    4) VAL_dot1dStpPortState_learning
 *                                    5) VAL_dot1dStpPortState_forwarding
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortStateWithMstidx (UI32_T mstidx, UI32_T lport, UI32_T state)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTSTATEWITHMSTIDX ;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=mstidx;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=lport;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=state;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}



#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddVlanToMst
 * -------------------------------------------------------------------------
 * FUNCTION: This function adds a VLAN to a given Spanning Tree instance.
 * INPUT   : vid                -- the VLAN will be added to a given Spanning Tree
 *           mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddVlanToMst(UI32_T vid, UI32_T mstidx)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADDVLANTOMST;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=vid;
    msg_data_p->data.u32a1_u32a2.u32_a2=mstidx;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif

#if (SYS_CPNT_DOT1X == TRUE)
/****************************************************************************/
/* DOT1X                                                                     */
/****************************************************************************/
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetDot1xAuthTrap
* -------------------------------------------------------------------------
* FUNCTION: This function will trap EtherType 888E packets to CPU
* INPUT   : ifindex
*           mode      -- SWCTRL_DOT1X_PACKET_DISCARD
*                        SWCTRL_DOT1X_PACKET_FORWARD
*                        SWCTRL_DOT1X_PACKET_TRAPTOCPU
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    :
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDot1xAuthTrap(UI32_T ifindex, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETDOT1XAUTHTRAP;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDot1xAuthControlMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set dot1x auth control mode
 * INPUT   : unit, port,
 *               mode
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDot1xAuthControlMode(UI32_T ifindex, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETDOT1XAUTHCONTROLMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ExecuteCableDiag
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ExecuteCableDiag(UI32_T lport, SWCTRL_Cable_Info_T *result)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.cable_diag_info)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=sizeof(msg_data_p->data.cable_diag_info.lport)
        +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_EXCUTECABLEDIAG;

    /*assign input*/
    msg_data_p->data.cable_diag_info.lport=lport;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *result=msg_data_p->data.cable_diag_info.result;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetRateBasedStormControl
* -------------------------------------------------------------------------
* FUNCTION: This function will set the rate based storm control
* INPUT   : ifindex
*           rate      -- kbits/s
*           mode      -- VAL_rateBasedStormMode_bcastStorm |
*                        VAL_rateBasedStormMode_mcastStorm |
*                        VAL_rateBasedStormMode_unknownUcastStorm
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    : specifically for BCM53115
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRateBasedStormControl(UI32_T ifindex, UI32_T rate, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETRATEBASEDSTORMCONTROL;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2=rate;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetRateBasedStormControlRate
* -------------------------------------------------------------------------
* FUNCTION: This function will set the rate based storm control
* INPUT   : ifindex
*           rate      -- kbits/s
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    : specifically for BCM53115
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRateBasedStormControlRate(UI32_T ifindex, UI32_T rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETRATEBASEDSTORMCONTROLRATE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetRateBasedStormControlMode
* -------------------------------------------------------------------------
* FUNCTION: This function will set the rate based storm control
* INPUT   : ifindex
*           mode      -- VAL_rateBasedStormMode_bcastStorm |
*                        VAL_rateBasedStormMode_mcastStorm |
*                        VAL_rateBasedStormMode_unknownUcastStorm
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    : specifically for BCM53115
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRateBasedStormControlMode(UI32_T ifindex, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETRATEBASEDSTORMCONTROLMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif
#if(SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)/*Tony.Lei*/
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortMACLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to set port status about Mac learning
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortMACLearningStatus(UI32_T ifindex, BOOL_T status)
{
    const UI32_T msg_buf_size = (sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
                                  + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p = (SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETMACLEARNINGBYPORT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2 = (UI32_T) status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }


    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableMldPacketTrap(SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLEMLDPACKETTRAP;

    /*assign input*/
    msg_data_p->data.ui32_v=owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DiableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to diable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableMldPacketTrap(SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DIABLEMLDPACKETTRAP;

    /*assign input*/
    msg_data_p->data.ui32_v=owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetRaAndRrPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: To enable/disable RA/RR packet trap.
 * INPUT   : is_enabled - TRUE to enable
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRaAndRrPacketTrap(
    BOOL_T  is_enabled)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bool_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETRAANDRRPACKETTRAP;

    /*assign input*/
    msg_data_p->data.bool_v = is_enabled;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortRaAndRrPacketDrop
 *------------------------------------------------------------------------
 * FUNCTION: To enable/disable RA/RR packet drop by specified ifindex.
 * INPUT   : ifindex    - ifindex to enable/disable
 *           is_enabled - TRUE to enable
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortRaAndRrPacketDrop(
    UI32_T ifindex, BOOL_T is_enabled)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTRAANDRRPACKETDROP;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1  = ifindex;
    msg_data_p->data.u32a1_boola2.bool_a2 = is_enabled;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE) */

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ExecuteInternalLoopBackTest
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to do the internal loop back test
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ExecuteInternalLoopBackTest(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_EXECUTEINTERNALLOOPBACKTEST;

    /*assign input*/
    msg_data_p->data.ui32_v=lport;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
}

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

#endif

#if(SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)

BOOL_T SWCTRL_PMGR_SetMDIXMode(UI32_T ifindex, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SetMDIXMode;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


BOOL_T SWCTRL_PMGR_GetMDIXMode(UI32_T ifindex, UI32_T *mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GetMDIXMode;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
    *mode = msg_data_p->data.u32a1_u32a2.u32_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}


#endif

#if (SYS_CPNT_POWER_SAVE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPowerSave
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port power saving status
 * INPUT   : ifindex --which port to enable/disable power save
 *               status--TRUE:enable
 *                           FALSE:disable
 * OUTPUT  :
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPowerSave(UI32_T ifindex,BOOL_T status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPORTPOWERSAVE;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1=ifindex;
    msg_data_p->data.u32a1_boola2.bool_a2=status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPortPowerSaveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the power save status
 * INPUT   : ifindex--the port to get power save status
 * OUTPUT  : status--the power save status of a specific port
 * RETURN  : True: Successfully
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPortPowerSaveStatus(UI32_T ifindex,BOOL_T *status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETPORTPOWERSAVESTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
    *status = msg_data_p->data.u32a1_boola2.bool_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetRunningPortPowerSaveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function get the power save status
 * INPUT   : ifindex--the port to get power save status
 * OUTPUT  : status--the power save status of a specific port
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningPortPowerSaveStatus(UI32_T ifindex, BOOL_T *status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();
    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETRUNNINGPORTPOWERSAVESTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1=ifindex;
    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
    *status = msg_data_p->data.u32a1_boola2.bool_a2;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

#endif /* end of #if (SYS_CPNT_POWER_SAVE == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapUnknownIpMcastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ip multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           owner  -- who call this api
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : to_cpu = 1 means trap to CPU
 *               to_cpu = 0 mans don't trap to CPU
 *               to_cpu = else means modify flood
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapUnknownIpMcastToCPU(UI8_T to_cpu, BOOL_T flood, SWCTRL_TrapPktOwner_T owner, UI32_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_TRAPUNKNOWNIPMCASTTOCPU;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = to_cpu;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = flood;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = owner;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = vid;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#if (SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapUnknownIpv6McastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ipv6 multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           owner  -- who call this api
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : to_cpu = 1 means trap to CPU
 *               to_cpu = 0 mans don't trap to CPU
 *               to_cpu = else means modify flood
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(UI8_T to_cpu, BOOL_T flood, SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_TRAPUNKNOWNIPV6MCASTTOCPU;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = to_cpu;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = flood;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

#if(SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapIpv6PIMToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap ipv6 PIM packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapIpv6PIMToCPU(BOOL_T to_cpu, SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_TRAPIPV6PIMTOCPU;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = to_cpu;
    msg_data_p->data.u32a1_u32a2.u32_a2 = owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetOamLoopback
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable efm oam loopback mode
 * INPUT   :
 *     l_port -- which logical port
 *     enable -- enable/disable loopback mode
 *     flag --
 *          SWCTRL_LOOPBACK_MODE_TYPE_ACTIVE
 *          SWCTRL_LOOPBACK_MODE_TYPE_PASSIVE
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetOamLoopback(UI32_T l_port, BOOL_T enable, UI32_T flag)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETOAMLOOPBACK);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3.u32_a1 = l_port;
        msg_p->data.u32a1_u32a2_u32a3.u32_a2 = enable;
        msg_p->data.u32a1_u32a2_u32a3.u32_a3 = flag;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}/* end of SWCTRL_PMGR_SetOamLoopback () */

#endif /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DropIpv6MulticastData
 * -------------------------------------------------------------------------
 * FUNCTION: Set port drop ip multicast data
 * INPUT   : lport - logical port
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : if this port is trunk port, it will set all trunk member
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DropIpv6MulticastData(UI32_T lport, BOOL_T enabled)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_DROPIPV6MULTICASTDATA);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2.u32_a1 = lport;
        msg_p->data.u32a1_u32a2.u32_a2 = enabled;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }

}
#endif

#if (SYS_CPNT_ITRI_MIM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ITRI_MIM_SetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 *           status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ITRI_MIM_SetStatus(UI32_T ifindex, BOOL_T status)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_ITRIMIMSETSTATUS);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2.u32_a2 = status;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableDhcpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable to trap DHCP packet function
 * INPUT   : owner  -- who want to enable dhcp trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ENABLE_DHCP_PACKET_TRAP;

    /*assign input*/
    msg_data_p->data.ui32_v=owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableDhcpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable to trap DHCP packet function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_TrapPktOwner_T owner)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.ui32_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_DISABLE_DHCP_PACKET_TRAP;

    /*assign input*/
    msg_data_p->data.ui32_v=owner;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

#if (SYS_CPNT_CLUSTER == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetOrgSpecificTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : Set whether organization specific frames are trapped to CPU.
 * INPUT   : trigger - who set the status (SWCTRL_OrgSpecificTrapTrigger_E)
 *           status  - TRUE / FALSE
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetOrgSpecificTrapStatus(UI32_T trigger, BOOL_T status)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_boola2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETORGSPECIFICTRAPSTATUS);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_boola2.u32_a1 = trigger;
        msg_p->data.u32a1_boola2.bool_a2 = status;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPPPoEDPktToCpu
 *-------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for specified ifindex.
 * INPUT  : ifindex   - ifindex to enable/disable
 *          is_enable - the packet trap is enabled or disabled
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTE   : 1. if ifindex is trunk, apply to all member ports
 *          2. if ifindex is normal/trunk member, apply to this port
 *          3. for projects who can install rule on trunk's member ports.
 *-------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPPPoEDPktToCpu(UI32_T ifindex, BOOL_T is_enable)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_boola2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPPPOEDPKTTOCPU;

    /*assign input*/
    msg_data_p->data.u32a1_boola2.u32_a1 = ifindex;
    msg_data_p->data.u32a1_boola2.bool_a2 = is_enable;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of SWCTRL_PMGR_SetPPPoEDPktToCpu*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem
 *-------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for specified ifindex.
 * INPUT  : ifindex   - ifindex to enable/disable
 *          is_enable - the packet trap is enabled or disabled
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTE   : 1. if ifindex is trunk, apply to all member ports
 *          2. if ifindex is normal/trunk member, apply to this port
 *          3. for projects who can install rule on trunk's member ports.
 *-------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem(BOOL_T is_enable)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bool_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPPPOEDPKTTOCPUPERSYSTEM;

    /*assign input*/
    msg_data_p->data.bool_v = is_enable;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem*/

#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_DOS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetDosProtectionFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           enable - TRUE to enable; FALSE to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDosProtectionFilter(SWCTRL_DosProtectionFilter_T type, BOOL_T enable)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETDOSPROTECTIONFILTER;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = type;
    msg_data_p->data.u32a1_u32a2.u32_a2 = enable;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetDosProtectionRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           rate   - rate in kbps. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDosProtectionRateLimit(SWCTRL_DosProtectionRateLimit_T type, UI32_T rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETDOSPROTECTIONRATELIMIT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = type;
    msg_data_p->data.u32a1_u32a2.u32_a2 = rate;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /* (SYS_CPNT_DOS == TRUE) */

#if ((SYS_CPNT_DHCPV6 == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableDhcp6PacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap dhcpv6 packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC    | Client packet action  |  Server packet action
 * =========|=======================|=========================
 *  Client  |        flood          |     copy to cpu
 *  relay   |     copy to cpu       |     copy to cpu
 * Snooping |   redirect to cpu     |   redirect to cpu
 *  Server  |     copy to cpu       |        flood
 *  NONE    |        flood          |        flood
 * =========|=======================|=========================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableDhcp6PacketTrap(SWCTRL_TrapPktOwner_T owner)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_ENABLE_DHCP6_PACKET_TRAP);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.ui32_v = owner;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableDhcp6PacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap dhcpv6 packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC   | Client packet action  |  Server packet action
 * ========|=======================|=========================
 *  Client |        flood          |     copy to cpu
 *  relay  |     copy to cpu       |     copy to cpu
 * Snooping|   redirect to cpu     |   redirect to cpu
 *  Server |     copy to cpu       |        flood
 *  NONE   |        flood          |        flood
 * ========|=======================|=========================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableDhcp6PacketTrap(SWCTRL_TrapPktOwner_T owner)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_DISABLE_DHCP6_PACKET_TRAP);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.ui32_v = owner;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPktTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : pkt_type  - which packet to trap
 *           owner     - who set the status
 *           to_cpu    - trap to cpu or not
 *           drop      - drop packet or not
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPktTrapStatus(SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T to_cpu, BOOL_T drop)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.pkt_trap)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SET_PKT_TRAP_STATUS;

    /*assign input*/
    msg_data_p->data.pkt_trap.pkt_type = pkt_type;
    msg_data_p->data.pkt_trap.owner = owner;
    msg_data_p->data.pkt_trap.to_cpu = to_cpu;
    msg_data_p->data.pkt_trap.drop = drop;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPortPktTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : ifindex
 *           pkt_type  - which packet to trap
 *           owner     - who set the status
 *           to_cpu    - trap to cpu or not
 *           drop      - drop packet or not
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortPktTrapStatus(UI32_T ifindex, SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T to_cpu, BOOL_T drop)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.pkt_trap)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SET_PORT_PKT_TRAP_STATUS;

    /*assign input*/
    msg_data_p->data.pkt_trap.ifindex = ifindex;
    msg_data_p->data.pkt_trap.pkt_type = pkt_type;
    msg_data_p->data.pkt_trap.owner = owner;
    msg_data_p->data.pkt_trap.to_cpu = to_cpu;
    msg_data_p->data.pkt_trap.drop = drop;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

#if (SYS_HWCFG_SUPPORT_PD==TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPDPortstatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PD port status
 * INPUT   : entry_p->port_pd_ifindex
 * OUTPUT  : entry_p->port_pd_status --- SWDRV_POWER_SOURCE_NONE
 *                                       SWDRV_POWER_SOURCE_UP
 *                                       SWDRV_POWER_SOURCE_DOWN
 *           entry_p->port_pd_mode   --- SWDRV_POWERED_DEVICE_MODE_NONE
 *                                       SWDRV_POWERED_DEVICE_MODE_AF
 *                                       SWDRV_POWERED_DEVICE_MODE_AT
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_GetPDPortStatus(SWCTRL_PortPD_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.pd_info)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE+sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.pd_info);

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETPDPORTSTATUS;

    /*assign input*/
    msg_data_p->data.pd_info.port_pd_ifindex = entry_p->port_pd_ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
    entry_p->port_pd_status = msg_data_p->data.pd_info.port_pd_status;
    entry_p->port_pd_mode = msg_data_p->data.pd_info.port_pd_mode;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextPDPortstatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get next PD port status
 * INPUT   : entry_p->port_pd_ifindex
 * OUTPUT  : entry_p->port_pd_ifindex
 *           entry_p->port_pd_status --- SWDRV_POWER_SOURCE_NONE
 *                                       SWDRV_POWER_SOURCE_UP
 *                                       SWDRV_POWER_SOURCE_DOWN
 *           entry_p->port_pd_mode   --- SWDRV_POWERED_DEVICE_MODE_NONE
 *                                       SWDRV_POWERED_DEVICE_MODE_AF
 *                                       SWDRV_POWERED_DEVICE_MODE_AT
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_GetNextPDPortStatus(SWCTRL_PortPD_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.pd_info)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE+sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.pd_info);

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETNEXTPDPORTSTATUS;

    /*assign input*/
    msg_data_p->data.pd_info.port_pd_ifindex = entry_p->port_pd_ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
    entry_p->port_pd_ifindex = msg_data_p->data.pd_info.port_pd_ifindex;
    entry_p->port_pd_status = msg_data_p->data.pd_info.port_pd_status;
    entry_p->port_pd_mode = msg_data_p->data.pd_info.port_pd_mode;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif

#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PSE check status
 * INPUT   : None
 * OUTPUT  : pse_check_status_p --- TRUE: PSE check enabled, FALSE: PSE check disabled
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPSECheckStatus(BOOL_T* pse_check_status_p)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bool_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETPSECHECKSTATUS;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    /*assign output*/
    *pse_check_status_p = msg_data_p->data.bool_v;
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 * INPUT   : pse_check_status --- TRUE: PSE check enabled, FALSE: PSE check disabled
 * OUTPUT  : None
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPSECheckStatus(BOOL_T pse_check_status)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bool_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETPSECHECKSTATUS;
    msg_data_p->data.bool_v=pse_check_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

#endif

#if (SYS_CPNT_NDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableNdPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap nd packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC    | Client packet action  
 * =========|=======================
 *  NDSNP   |    redirect to cpu          
 *  NONE    |        flood          
 * =========|=======================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableNdPacketTrap(SWCTRL_TrapPktOwner_T owner)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_ENABLE_ND_PACKET_TRAP);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.ui32_v = owner;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableNdPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap nd packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC    | Client packet action  
 * =========|=======================
 *  NDSNP   |    redirect to cpu          
 *  NONE    |        flood          
 * =========|=======================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableNdPacketTrap(SWCTRL_TrapPktOwner_T owner)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_DISABLE_ND_PACKET_TRAP);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.ui32_v = owner;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif 

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPortLearningStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port learning status
 * INPUT    :   ifindex
 *              learning
 *              owner
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortLearningStatus(UI32_T ifindex, BOOL_T learning, SWCTRL_LearningDisabledOwner_T owner)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTLEARNINGSTATUS);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2_u32a3.u32_a2 = learning;
        msg_p->data.u32a1_u32a2_u32a3.u32_a3 = owner;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPortPfcStatus
 * -------------------------------------------------------------------------
 * PURPOSE: To set the PFC port status by specified ifidx.
 * INPUT  : ifidx      -- ifindex to set
 *          rx_en      -- enable/disable PFC response
 *          tx_en      -- enable/disable PFC triggering
 *          pri_en_vec -- bitmap of enable status per priority
 *                         set bit to enable PFC; clear to disable.
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPfcStatus(
    UI32_T  ifidx,
    BOOL_T  rx_en,
    BOOL_T  tx_en,
    UI16_T  pri_en_vec)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(ifidx_pfc_data),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTPFCSTATUS);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.ifidx_pfc_data.ifidx = ifidx;
        msg_p->data.ifidx_pfc_data.rx_en = rx_en;
        msg_p->data.ifidx_pfc_data.tx_en = tx_en;
        msg_p->data.ifidx_pfc_data.pri_en_vec = pri_en_vec;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_UpdatePfcPriMap
 * -------------------------------------------------------------------------
 * PURPOSE : This function update PFC priority to queue mapping.
 * INPUT   : None
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_UpdatePfcPriMap(void)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_UPDATEPFCPRIMAP);

    {
        BOOL_T  ret = FALSE;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_ETS == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCosGroupMapping
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets mapping between CoS Queue and CoS group
 * INPUT   : ifindex
 *           cosq2group -- array of cos groups.
 *                         element 0 is cos group of cosq 0,
 *                         element 1 is cos group of cosq 1, ...
 *                         NULL to map all cos to single cos group
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCosGroupMapping(
    UI32_T ifindex,
    UI32_T cosq2group[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE])
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(cos_group_mapping),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTCOSGROUPMAPPING);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.cos_group_mapping.ifindex = ifindex;

        if (cosq2group)
        {
            memcpy(msg_p->data.cos_group_mapping.cosq2group, cosq2group, sizeof(msg_p->data.cos_group_mapping.cosq2group));
            msg_p->data.cos_group_mapping.cosq2group_is_valid = TRUE;
        }
        else
        {
            msg_p->data.cos_group_mapping.cosq2group_is_valid = FALSE;
        }

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCosGroupSchedulingMethod
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets scheduling method for CoS groups
 * INPUT   : ifindex
 *           method  -- SWCTRL_Egress_Scheduling_Method_E
 *           weights -- weights for cos groups.
 *                      NULL if method is STRICT
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCosGroupSchedulingMethod(
    UI32_T ifindex,
    UI32_T method,
    UI32_T weights[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS])
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(cos_group_scheduling),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTCOSGROUPSCHEDULINGMETHOD);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.cos_group_scheduling.ifindex = ifindex;
        msg_p->data.cos_group_scheduling.method = method;

        if (weights)
        {
            memcpy(msg_p->data.cos_group_scheduling.weights, weights, sizeof(msg_p->data.cos_group_scheduling.weights));
            msg_p->data.cos_group_scheduling.weights_is_valid = TRUE;
        }
        else
        {
            msg_p->data.cos_group_scheduling.weights_is_valid = FALSE;
        }

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif /* (SYS_CPNT_ETS == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetQcnCnmPriority
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets 802.1p priority of egress QCN CNM
 * INPUT   : pri -- 802.1p priority of egress QCN CNM
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetQcnCnmPriority(UI32_T pri)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETQCNCNMPRIORITY);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.ui32_v = pri;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortQcnCpq
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets CP Queue of the CoS Queue
 * INPUT   : ifindex
 *           cosq -- CoS Queue
 *           cpq  -- CP Queue. SWCTRL_QCN_CPQ_INVALID means to disable QCN
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortQcnCpq(
    UI32_T ifindex,
    UI32_T cosq,
    UI32_T cpq)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTQCNCPQ);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2_u32a3.u32_a2 = cosq;
        msg_p->data.u32a1_u32a2_u32a3.u32_a3 = cpq;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets removal of CN-Tag of egress pkts
 * INPUT   : ifindex
 *           no_cntag_bitmap - bit 0 for pri 0, and so on.
 *                             set the bit to remove CN-tag of packets with the pri.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(
    UI32_T ifindex,
    UI8_T no_cntag_bitmap)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTQCNEGRCNTAGREMOVAL);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2.u32_a2 = no_cntag_bitmap;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPortQcnCpid
 *------------------------------------------------------------------------------
 * FUNCTION: This function gets CPID
 * INPUT   : ifindex
 *           cosq -- CoS Queue
 *           cpid -- CPID
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPortQcnCpid(
    UI32_T ifindex,
    UI32_T cosq,
    UI8_T cpid[8])
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2),
        SWCTRL_MGR_GET_MSG_SIZE(qcn_cpid),
        SWCTRL_MGR_IPCCMD_GETPORTQCNCPID);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2.u32_a1 = ifindex;
        msg_p->data.u32a1_u32a2.u32_a2 = cosq;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        if (ret)
        {
            memcpy(cpid, msg_p->data.qcn_cpid.cpid, sizeof(msg_p->data.qcn_cpid.cpid));
        }

        return ret;
    }
}
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMimService
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy a MiM service instance.
 * INPUT   : mim_p            -- MiM service instance info.
 *           is_valid         -- TRUE to create/update; FALSE to destroy.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMimService(SWCTRL_MimServiceInfo_T *mim_p, BOOL_T is_valid)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(mim_service),
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_IPCCMD_SETMIMSERVICE);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.mim_service.mim = *mim_p;
        msg_p->data.mim_service.is_valid = is_valid;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        if (ret)
        {
            mim_p->hw_idx = msg_p->data.ui32_v;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMimServicePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add/delete member port to a MiM service instance.
 * INPUT   : mim_port_p       -- MiM port info.
 *           is_valid         -- TRUE to add; FALSE to delete.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMimServicePort(SWCTRL_MimPortInfo_T *mim_port_p, BOOL_T is_valid)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(mim_service_port),
        SWCTRL_MGR_GET_MSG_SIZE(ui32_v),
        SWCTRL_MGR_IPCCMD_SETMIMSERVICEPORT);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.mim_service_port.mim_port = *mim_port_p;
        msg_p->data.mim_service_port.is_valid = is_valid;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        if (ret)
        {
            mim_port_p->hw_idx = msg_p->data.ui32_v;
        }

        return ret;
    }
}

#if (SYS_CPNT_IAAS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetMimServicePortLearningStatusForStationMove
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set MiM port learning status
 *              for station move handling only
 * INPUT    :   learning
 *              to_cpu
 *              drop
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetMimServicePortLearningStatusForStationMove(BOOL_T learning, BOOL_T to_cpu, BOOL_T drop)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETMIMSERVICEPORTLEARNINGSTATUSFORSTATIONMOVE);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3.u32_a1 = learning;
        msg_p->data.u32a1_u32a2_u32a3.u32_a2 = to_cpu;
        msg_p->data.u32a1_u32a2_u32a3.u32_a3 = drop;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif /* (SYS_CPNT_IAAS == TRUE) */
#endif /* (SYS_CPNT_MAC_IN_MAC == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To configure CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 *           rate      -- in pkt/s. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetCpuRateLimit(UI32_T pkt_type, UI32_T rate)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETCPURATELIMIT);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2.u32_a1 = pkt_type;
        msg_p->data.u32a1_u32a2.u32_a2 = rate;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

#if (SYS_CPNT_SFLOW == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetSflowPortPacketSamplingRate
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port sampling rate.
 * INPUT    : ifindex  -- interface index
 *            rate     -- sampling rate
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_PMGR_SetSflowPortPacketSamplingRate(
    UI32_T ifindex,
    UI32_T rate)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    req_size = msg_buf_size;
    resp_size = SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    msg_p = (SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;
    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SET_SFLOW_PORT_SAMPLING_RATE;

    msg_data_p->data.u32a1_u32a2.u32_a1 = ifindex;
    msg_data_p->data.u32a1_u32a2.u32_a2 = rate;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    return msg_data_p->type.result_bool;
} /* End of SWCTRL_PMGR_SetSflowPortPacketSamplingRate */    
#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

#if (TRUE == SYS_CPNT_APP_FILTER)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DropPortCdpPacket
 *-------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           ifindex -- interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_PMGR_DropPortCdpPacket(
    BOOL_T enable,
    UI32_T ifindex)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_boola2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_DROP_PORT_CDP_PACKE);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_boola2.u32_a1 = ifindex;
        msg_p->data.u32a1_boola2.bool_a2 = enable;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DropPortPvstPacket
 *-------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           ifindex -- interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_PMGR_DropPortPvstPacket(
    BOOL_T enable,
    UI32_T ifindex)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_boola2),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_DROP_PORT_PVST_PACKET);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_boola2.u32_a1 = ifindex;
        msg_p->data.u32a1_boola2.bool_a2 = enable;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_GetPortFigureType
 * ------------------------------------------------------------------------|
 * FUNCTION : This function returns the port figure type of the specified
 *            port.
 * INPUT    : ifindex            : interface index.
 * OUTPUT   : port_figure_type_p : port figure type of the specified port.
 * RETURN   : TRUE  -  Success
 *            FALSE -  Failed
 * NOTE     : 1. This function is for WEB to draw the apperance of the
 *               specified port on the front panel of web pages by the port
 *               figure type.
 *            2. ifindex can only be a ifindex for a physical port.
 *            3. port_figure_type will be STKTPLG_TYPE_PORT_FIGURE_TYPE_NULL
 *               if the port is on a expansion module and the expansion module
 *               is not inserted.
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPortFigureType(UI32_T ifindex, STKTPLG_TYPE_Port_Figure_Type_T *port_figure_type_p)
{
    UI32_T unit, port;

    if (STKTPLG_OM_IS_USER_PORT(ifindex))
    {
	    unit = STKTPLG_OM_IFINDEX_TO_UNIT(ifindex);
        port = STKTPLG_OM_IFINDEX_TO_PORT(ifindex);

        if (FALSE == STKTPLG_OM_GetPortFigureType(unit, port, port_figure_type_p))
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

#endif /* #if (TRUE == SYS_CPNT_APP_FILTER) */


#if (SYS_CPNT_VRRP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapVrrpToCpu
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap VRRP packet to cpu
 * INPUT   : trap or not
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapVrrpToCpu(BOOL_T is_trap)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bool_v)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SET_VRRP_TRAP;

    /*assign input*/
    msg_data_p->data.bool_v=is_trap;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;

}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL__PMGR_PortTypeToMediaType
 *-------------------------------------------------------------------------
 * PURPOSE  : Convert port type to media type.
 * INPUT    : port_type  -- port type (e.g. VAL_portType_hundredBaseTX)
 * OUTPUT   : None
 * RETUEN   : The media type of the specified port type.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */

SWCTRL_Port_Media_Type_T SWCTRL_PMGR_PortTypeToMediaType(UI32_T port_type)
{

    switch (port_type)
    {
        case VAL_portType_other:
	    return SWCTRL_PORT_MEDIA_TYPE_UNKNOWN;
        case VAL_portType_hundredBaseTX:
	    return SWCTRL_PORT_MEDIA_TYPE_COPPER;
        case VAL_portType_hundredBaseFX:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_thousandBaseLX:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_thousandBaseSX:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_thousandBaseT:
	    return SWCTRL_PORT_MEDIA_TYPE_COPPER;
        case VAL_portType_thousandBaseGBIC:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_thousandBaseSfp:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_hundredBaseFxScSingleMode:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_hundredBaseFxScMultiMode:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_thousandBaseCX:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_tenG:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_hundredBaseBX:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_thousandBaseBX:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_tenGBaseT:
	    return SWCTRL_PORT_MEDIA_TYPE_COPPER;
        case VAL_portType_tenGBaseXFP:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_tenGBaseSFP:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_twentyFiveGBaseSFP:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_fortyGBaseQSFP:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        case VAL_portType_hundredGBaseQSFP:
	    return SWCTRL_PORT_MEDIA_TYPE_FIBER;
        default:
	    return SWCTRL_PORT_MEDIA_TYPE_UNKNOWN;
	}
}

#if (SYS_CPNT_MAU_MIB == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetIfMauEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get MAU entry specified in RFC-3636.
 * INPUT   : if_mau_entry->ifMauIfIndex  -- Which interface.
 *           if_mau_entry->ifMauIndex    -- Which MAU.
 * OUTPUT  : if_mau_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetIfMauEntry (SWCTRL_IfMauEntry_T *if_mau_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.if_mau_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=sizeof(msg_data_p->data.if_mau_entry.ifMauIfIndex)
        +sizeof(msg_data_p->data.if_mau_entry.ifMauIndex)
        +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETIFMAUENTRY ;

    /*assign input*/
    msg_data_p->data.if_mau_entry.ifMauIfIndex=if_mau_entry->ifMauIfIndex;
    msg_data_p->data.if_mau_entry.ifMauIndex=if_mau_entry->ifMauIndex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *if_mau_entry=msg_data_p->data.if_mau_entry;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextIfMauEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next MAU entry specified in RFC-3636.
 * INPUT   : if_mau_entry->ifMauIfIndex  -- Next to which interface.
 *           if_mau_entry->ifMauIndex    -- Next to which MAU.
 * OUTPUT  : if_mau_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextIfMauEntry (SWCTRL_IfMauEntry_T *if_mau_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.if_mau_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=sizeof(msg_data_p->data.if_mau_entry.ifMauIfIndex)
        +sizeof(msg_data_p->data.if_mau_entry.ifMauIndex)
        +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETNEXTIFMAUENTRY;

    /*assign input*/
    msg_data_p->data.if_mau_entry.ifMauIfIndex=if_mau_entry->ifMauIfIndex;
    msg_data_p->data.if_mau_entry.ifMauIndex=if_mau_entry->ifMauIndex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *if_mau_entry=msg_data_p->data.if_mau_entry;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetIfMauAutoNegEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get auto-negoation entry of the MAU, that is specified in REF-3636
 * INPUT   : if_mau_auto_neg_entry->ifMauIfIndex -- Which interface.
 *           if_mau_auto_neg_entry->ifMauIndex   -- Which MAU.
 * OUTPUT  : if_mau_auto_neg_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetIfMauAutoNegEntry (SWCTRL_IfMauAutoNegEntry_T *if_mau_auto_neg_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.if_mau_auto_neg_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=sizeof(msg_data_p->data.if_mau_auto_neg_entry.ifMauIfIndex)
        +sizeof(msg_data_p->data.if_mau_auto_neg_entry.ifMauIndex)
        +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETIFMAUAUTONEGENTRY;

    /*assign input*/
    msg_data_p->data.if_mau_auto_neg_entry.ifMauIfIndex=if_mau_auto_neg_entry->ifMauIfIndex;
    msg_data_p->data.if_mau_auto_neg_entry.ifMauIndex=if_mau_auto_neg_entry->ifMauIndex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *if_mau_auto_neg_entry=msg_data_p->data.if_mau_auto_neg_entry;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextIfMauAutoNegEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next auto-negoation entry of the MAU, one is specified in RFC-3636
 * INPUT   : if_mau_auto_neg_entry->ifMauIfIndex  -- Next to which interface.
 *           if_mau_auto_neg_entry->ifMauIndex    -- Next to which Which MAU.
 * OUTPUT  : if_mau_auto_neg_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextIfMauAutoNegEntry (SWCTRL_IfMauAutoNegEntry_T *if_mau_auto_neg_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.if_mau_auto_neg_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=sizeof(msg_data_p->data.if_mau_auto_neg_entry.ifMauIfIndex)
        +sizeof(msg_data_p->data.if_mau_auto_neg_entry.ifMauIndex)
        +SWCTRL_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETNEXTIFMAUAUTONEGENTRY;

    /*assign input*/
    msg_data_p->data.if_mau_auto_neg_entry.ifMauIfIndex=if_mau_auto_neg_entry->ifMauIfIndex;
    msg_data_p->data.if_mau_auto_neg_entry.ifMauIndex=if_mau_auto_neg_entry->ifMauIndex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *if_mau_auto_neg_entry=msg_data_p->data.if_mau_auto_neg_entry;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetIfJackEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get jack entry of the jack in some MAU, one is specified in RFC-3636.
 * INPUT   : if_jack_entry->ifMauIfIndex  -- Which interface.
 *           if_jack_entry->ifMauIndex    -- Which MAU.
 *           if_jack_entry->ifJackIndex   -- Which jack.
 * OUTPUT  : if_jack_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetIfJackEntry (SWCTRL_IfJackEntry_T *if_jack_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.if_jack_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETIFJACKENTRY ;

    /*assign input*/
    msg_data_p->data.if_jack_entry=*if_jack_entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *if_jack_entry=msg_data_p->data.if_jack_entry;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextIfJackEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next jack entry of the jack in some MAU, one is specified in RFC-3636.
 * INPUT   : if_jack_entry->ifMauIfIndex  -- Next to which interface.
 *           if_jack_entry->ifMauIndex    -- Next to which MAU.
 *           if_jack_entry->ifJackIndex   -- Next to which jack.
 * OUTPUT  : if_jack_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextIfJackEntry (SWCTRL_IfJackEntry_T *if_jack_entry)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.if_jack_entry)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_GETNEXTIFJACKENTRY;

    /*assign input*/
    msg_data_p->data.if_jack_entry=*if_jack_entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *if_jack_entry=msg_data_p->data.if_jack_entry;

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortEgressBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block port.
 * INPUT   : lport              - source lport
 *           egr_lport          - lport to block
 *           is_block           - TRUE to enable egress block status
 *                                FALSE to disable egress block status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortEgressBlock(
    UI32_T lport,
    UI32_T egr_lport,
    BOOL_T is_block)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(u32a1_u32a2_u32a3),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTEGRESSBLOCK);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.u32a1_u32a2_u32a3.u32_a1 = lport;
        msg_p->data.u32a1_u32a2_u32a3.u32_a2 = egr_lport;
        msg_p->data.u32a1_u32a2_u32a3.u32_a3 = is_block;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortEgressBlockEx
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block ports.
 * INPUT   : lport               - source lport
 *           egr_lport_list      - lport list to update egress block status.
 *                                 NULL to indicate all lport list.
 *           blk_lport_list      - lport list to specify egress block status.
 *                                 set bit to enable egress block status, clear bit to disable.
 *                                 NULL to indicate empty lport list.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortEgressBlockEx(
    UI32_T lport,
    UI8_T egr_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI8_T blk_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(port_egress_block),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_SETPORTEGRESSBLOCKEX);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.port_egress_block.lport = lport;
        msg_p->data.port_egress_block.egr_lport_list_is_specified = FALSE;
        msg_p->data.port_egress_block.blk_lport_list_is_specified = FALSE;

        if (egr_lport_list)
        {
            memcpy(
                msg_p->data.port_egress_block.egr_lport_list,
                egr_lport_list,
                sizeof(msg_p->data.port_egress_block.egr_lport_list));
            msg_p->data.port_egress_block.egr_lport_list_is_specified = TRUE;
        }

        if (blk_lport_list)
        {
            memcpy(
                msg_p->data.port_egress_block.blk_lport_list,
                blk_lport_list,
                sizeof(msg_p->data.port_egress_block.blk_lport_list));
            msg_p->data.port_egress_block.blk_lport_list_is_specified = TRUE;
        }

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_BindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bind_hash_service)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_BINDHASHSELFORSERVICE;

    /*assign input*/
    msg_data_p->data.bind_hash_service.service = service;
    msg_data_p->data.bind_hash_service.list_index = list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_UnBindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.bind_hash_service)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_UNBINDHASHSELFORSERVICE;

    /*assign input*/
    msg_data_p->data.bind_hash_service.service = service;
    msg_data_p->data.bind_hash_service.list_index = list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: add hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection list has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.set_hash_sel)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_ADDHASHSELECTION;

    /*assign input*/
    msg_data_p->data.set_hash_sel.list_index = list_index;
    memcpy(&msg_data_p->data.set_hash_sel.selection, selection_p, sizeof(SWCTRL_OM_HashSelection_T));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_RemoveHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: remove hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection list has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_RemoveHashSelection(
    UI8_T list_index ,
    SWCTRL_OM_HashSelection_T *selection_p)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.set_hash_sel)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_REMOVEHASHSELECTION;

    /*assign input*/
    msg_data_p->data.set_hash_sel.list_index = list_index;
    memcpy(&msg_data_p->data.set_hash_sel.selection, selection_p, sizeof(SWCTRL_OM_HashSelection_T));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/

    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetSwitchingMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set switching mode
 * INPUT   : lport  - which port to configure.
 *           mode   - VAL_swctrlSwitchModeSF
 *                    VAL_swctrlSwitchModeCT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetSwitchingMode(UI32_T lport, UI32_T mode)
{
    const UI32_T msg_buf_size=(sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        + SWCTRL_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    SWCTRL_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;
    SWCTRL_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=SWCTRL_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_SWCTRL;
    msg_p->msg_size = req_size;

    msg_data_p=(SWCTRL_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = SWCTRL_MGR_IPCCMD_SETSWITCHINGMODE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1=lport;
    msg_data_p->data.u32a1_u32a2.u32_a2=mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        SWCTRL_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    SWCTRL_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/

#if(SYS_CPNT_WRED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_RandomDetect
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port ecn marking percentage
 * INPUT    :   lport      - which port to set
 *              value_p    - percentage value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   queue_id = -1 means all queue
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_RandomDetect(UI32_T lport, SWCTRL_RandomDetect_T *value_p)
{
    SWCTRL_PMGR_FUNC_BEGIN(
        SWCTRL_MGR_GET_MSG_SIZE(random_detect),
        SWCTRL_MGR_MSGBUF_TYPE_SIZE,
        SWCTRL_MGR_IPCCMD_RANDOM_DETECT);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.random_detect.lport = lport;
        msg_p->data.random_detect.value = *value_p;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_bool;
        }

        return ret;
    }
}
#endif /*End SWCTRL_PMGR_RandomDetect*/
