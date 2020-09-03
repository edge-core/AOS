/* MODULE NAME:  cfm_pmgr.c
 * PURPOSE:
 *    This is a sample code for implementation of MGR.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/2/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "cfm_type.h"
#include "cfm_mgr.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_mm.h"
#include "sys_bld.h"
#if (SYS_CPNT_CFM == TRUE)
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the IGMPSNP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of request message.
 *           res_size  - the size of response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : TRUE   - success
 *                 FALSe - fail
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_PMGR_SendMsg(SYSFUN_Msg_T *msg_p,
                                             UI32_T cmd,
                                             const UI32_T req_size,
                                             const UI32_T resp_size);
/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------
 * ROUTINE NAME : CFM_PMGR_Init
 *---------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CFM_MGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  - Success.
 *    FALSE - Fail
 * NOTES:
 *    None.
 *---------------------------------------------------------------------------
 */
BOOL_T CFM_PMGR_InitiateProcessResources(void)
{
    /* Given that CFM PMGR requests are handled in CFMGROUP of CFM_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_CFM_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_AddnewMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will add new Mep to a MD/MA
 * INPUT    : lport       - the logical port
 *            mep_id      - the mep identifier
 *            md_name_a   - the Md name
 *            md_name_len - the Md name length
 *            ma_name_a   - the Ma name
 *            ma_name_len - the Ma name length
 *            direction   - the direction of mep
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at mep parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_AddnewMEP(
    UI32_T                  lport,
    UI32_T                  mep_id,
    char                    md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T                  md_name_len,
    char                    ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T                  ma_name_len,
    CFM_TYPE_MP_Direction_T direction)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.new_mep)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.new_mep)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_ADDNEWMEP;

    msg_data_p->data.new_mep.lport       = lport;
    msg_data_p->data.new_mep.mep_id      = mep_id;
    msg_data_p->data.new_mep.direction   = direction;
    msg_data_p->data.new_mep.md_name_len = md_name_len;
    msg_data_p->data.new_mep.ma_name_len = ma_name_len;

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.new_mep.md_name_ar, md_name_a, md_name_len);
    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.new_mep.ma_name_ar, ma_name_a, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_AddRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function add a cross check remote mep
 * INPUT    : md_index - the md index
 *            mep_id   - the mep identifier
 *            ma_name_a- the ma name array
 *            name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_AddRemoteMep(UI32_T md_index ,UI32_T mep_id,
        char ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH], UI32_T name_len)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_name_u32a3)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_name_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_ADDREMOTEMEP;

    msg_data_p->data.ma_name_u32a3.u32_a1 = md_index;
    msg_data_p->data.ma_name_u32a3.u32_a2 = mep_id;
    memcpy(msg_data_p->data.ma_name_u32a3.ma_name_ar, ma_name_a,
        sizeof(msg_data_p->data.ma_name_u32a3.ma_name_ar ));
    msg_data_p->data.ma_name_u32a3.u32_a3 = name_len;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 *            levle       - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     : you can specify the md name or the level
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearErrorList()
{
    const UI32_T msg_buf_size=CFM_MGR_MSGBUF_TYPE_SIZE;
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARERRORLIST;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 *            levle       - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     : you can specify the md name or the level
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearErrorsListByMdNameOrLevel
        (char *md_name_ap, UI32_T name_len,CFM_TYPE_MdLevel_T level)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.clear_error_by_md_level)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.clear_error_by_md_level)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARERRORSLISTBYMDNAMEORLEVEL;

    if (NULL != md_name_ap)
        memcpy(&msg_data_p->data.clear_error_by_md_level.md_name_ar, md_name_ap, name_len);
    msg_data_p->data.clear_error_by_md_level.name_len = name_len;
    msg_data_p->data.clear_error_by_md_level.level    = level;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearLinktraceCache
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear all the link trace reply in om
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearLinktraceCache()
{
    const UI32_T msg_buf_size=CFM_MGR_MSGBUF_TYPE_SIZE;
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARLINKTRACECACHE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearLoopBackList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the loop back message be sent
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_PMGR_ClearLoopBackList()
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARLOOPBACKLIST;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_NOWAIT,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        return;
    }
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearRemoteMepAll
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD domain
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 * OUTPUT   : None
 * RETURN   : * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR) - clear failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearRemoteMepAll()
{
    const UI32_T msg_buf_size=CFM_MGR_MSGBUF_TYPE_SIZE;
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=msg_buf_size;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARREMOTEMEPALL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD domain
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearRemoteMepByDomain(char *md_name_ap, UI32_T name_len)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.remote_mep_by_domain)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.remote_mep_by_domain)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARREMOTEMEPBYDOMAIN;

    memcpy(&msg_data_p->data.remote_mep_by_domain.md_name_ar, md_name_ap,
        sizeof(msg_data_p->data.remote_mep_by_domain.md_name_ar ));
    msg_data_p->data.remote_mep_by_domain.name_len = name_len;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearRemoteMepByLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD level
 * INPUT    : level - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearRemoteMepByLevel(CFM_TYPE_MdLevel_T level)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_level)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_level)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_CLEARREMOTEMEPBYLEVEL;

    msg_data_p->data.md_level = level;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteDot1agCfmMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. If there still exist the Mep in this MA, this MA can't be deleted
 *           2. for mib only
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteDot1agCfmMa(UI32_T md_index, UI32_T ma_index)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEDOT1AGCFMMA;

    msg_data_p->data.u32a1_u32a2.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2.u32_a2 = ma_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_DeleteDot1agCfmMaMepListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the ma mep id list
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. not support this function, it will just return false
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteDot1agCfmMaMepListEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEDOT1AGCFMMAMEPLISTENTRY;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteDot1agCfmMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MD
 * INPUT    : md_index - the MD index
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at md parameter
 *            CFM_TYPE_CONFIG_ERROR    - Operation mode error
 * NOTE     :1. If there exist MA in this domain, this MD can't be deleted
 *           2. for mib
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteDot1agCfmMd(UI32_T md_index)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEDOT1AGCFMMD;

    msg_data_p->data.ui32_v = md_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_DeleteDot1agCfmMepEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the mep entry
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteDot1agCfmMepEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEDOT1AGCFMMEPENTRY;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : If there still exist the Mep in this MA, this MA can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteMA(UI32_T md_index,UI32_T ma_index)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEMA;

    msg_data_p->data.u32a1_u32a2.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2.u32_a2 = ma_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteMAVlan
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA vlan
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            vid      - the vlan id
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : if this vid is primary vid and ma still exit mep, this vid can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteMAVlan(UI32_T md_index, UI32_T ma_index, UI16_T vid)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u16a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u16a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEMAVLAN;

    msg_data_p->data.u32a1_u32a2_u16a1.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u16a1.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u16a1.u16_a1 = vid;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MD
 * INPUT    : md_index - the MD index
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at md parameter
 * NOTE     :If there exist MA in this domain, this MD can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteMD(UI32_T md_index)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEMD;

    msg_data_p->data.ui32_v = md_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete a Mep from MD/MA
 * INPUT    : lport       - the lport which mep reside
 *            mep_id      - the mep id
 *            md_name_a   - the array store the Md name
 *            md_name_len - the md_name length
 *            ma_name_a   - the array store the Ma name
 *            ma_name_len - the ma_name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at mep parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteMEP(
    UI32_T  lport,
    UI32_T  mep_id,
    char    md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T  md_name_len,
    char    ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  ma_name_len)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_ma_name_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_ma_name_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEMEP;

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a2.md_name_ar, md_name_a, md_name_len);
    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a2.ma_name_ar, ma_name_a, ma_name_len);
    msg_data_p->data.md_ma_name_u32a2.md_name_len = md_name_len;
    msg_data_p->data.md_ma_name_u32a2.ma_name_len = ma_name_len;
    msg_data_p->data.md_ma_name_u32a2.u32_a1      = lport;
    msg_data_p->data.md_ma_name_u32a2.u32_a2      = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DeleteRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete a cross check remote mep
 * INPUT    : md_index  - the md index
 *            mep_id    - the mep identifier
 *            ma_name_a - the ma name stored array
 *            name_len  - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - delete failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DeleteRemoteMep(UI32_T md_index,
    UI32_T mep_id, char ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],UI32_T name_len)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_name_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_name_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DELETEREMOTEMEP;

    msg_data_p->data.ma_name_u32a3.u32_a1 = md_index;
    msg_data_p->data.ma_name_u32a3.u32_a2 = mep_id;
    memcpy(msg_data_p->data.ma_name_u32a3.ma_name_ar, ma_name_a,
        sizeof(msg_data_p->data.ma_name_u32a3.ma_name_ar ));
    msg_data_p->data.ma_name_u32a3.u32_a3 = name_len;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetArchiveHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the remote mep info store hold time
 * INPUT    : md_index  - the md index
 *            hold_time - the remote mep info hold time
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at hold_time parameter
 * NOTE     : input in minutes, engine run in second.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetArchiveHoldTime(UI32_T md_index, UI32_T hold_time)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETARCHIVEHOLDTIME;

    msg_data_p->data.u32a1_u32a2.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2.u32_a2 = hold_time;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the CCM transmit interval level
 * INPUT    : md_name_ap  - the md name array pointer
 *            md_name_len - md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - ma name length
 *            interval    - the ccm interval level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - configure success
 *            CFM_TYPE_CONFIG_ERROR   - configure fail at interval parameter
 * NOTE     : for our switch use 1 sec. as time unit, so contraint interval level as 1 sec.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetCcmInterval(
    char                    *md_name_ap,
    UI32_T                  md_name_len,
    char                    *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmInterval_T  interval)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.setting_ccm_interval)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.setting_ccm_interval)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETCCMINTERVAL;

    msg_data_p->data.setting_ccm_interval.interval = interval;
    if (NULL != md_name_ap)
        memcpy(msg_data_p->data.setting_ccm_interval.md_name_ar, md_name_ap, md_name_len);
    msg_data_p->data.setting_ccm_interval.md_name_len = md_name_len;
    if (NULL != ma_name_ap)
        memcpy(msg_data_p->data.setting_ccm_interval.ma_name_ar, ma_name_ap, ma_name_len);
    msg_data_p->data.setting_ccm_interval.ma_name_len = ma_name_len;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetCcmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the CCM transmit status
 * INPUT    : md_name_ap  - the MD name array pointer
 *            md_name_len - the MD name length
 *            ma_name_ap  - the MA name array pointer
 *            ma_name_len - the MA name length
 *            status      - the CFM enable status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at status parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetCcmStatus(
    char                    *md_name_ap,
    UI32_T                  md_name_len,
    char                    *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmStatus_T    status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.setting_ccm_status)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.setting_ccm_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETCCMSTATUS;

    if (NULL != md_name_ap)
        memcpy(msg_data_p->data.setting_ccm_status.md_name_ar, md_name_ap, md_name_len);
    msg_data_p->data.setting_ccm_status.md_name_len = md_name_len;

    if (NULL != ma_name_ap)
        memcpy(msg_data_p->data.setting_ccm_status.ma_name_ar, ma_name_ap, ma_name_len);
    msg_data_p->data.setting_ccm_status.ma_name_len = ma_name_len;

    msg_data_p->data.setting_ccm_status.status = status;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetCFMGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the globl CFM status
 * INPUT    : staus - the CFM status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetCFMGlobalStatus(CFM_TYPE_CfmStatus_T status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.cfm_status)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.cfm_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETCFMGLOBALSTATUS ;

    msg_data_p->data.cfm_status = status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetCFMPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the each port CFM status
 * INPUT    : lport  - the logical port
 *            status - the CFM status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetCFMPortStatus(UI32_T lport, CFM_TYPE_CfmStatus_T status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.port_status)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.port_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETCFMPORTSTATUS ;

    msg_data_p->data.port_status.lport = lport;
    msg_data_p->data.port_status.status = status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check start delay
 * INPUT    : delay  - the cross check start delay
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetCrossCheckStartDelay(UI32_T delay)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETCROSSCHECKSTARTDELAY ;

    msg_data_p->data.ui32_v = delay;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetCrossCheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check status
 * INPUT    : status      - the cross check status
 *            md_name_ap  - the md name array pointer
 *            md_name_len - the md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     : CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
 *            CFM_TYPE_CROSS_CHECK_STATUS_ENABLE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetCrossCheckStatus(
    CFM_TYPE_CrossCheckStatus_T     status,
    char                            *md_name_ap,
    UI32_T                          md_name_len,
    char                            *ma_name_ap,
    UI32_T                          ma_name_len)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.crosscheck_status)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.crosscheck_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETCROSSCHECKSTATUS ;

    msg_data_p->data.crosscheck_status.status = status;
    msg_data_p->data.crosscheck_status.md_name_len = md_name_len;
    msg_data_p->data.crosscheck_status.ma_name_len = ma_name_len;
    if (NULL != md_name_ap)
        memcpy(msg_data_p->data.crosscheck_status.md_name_ar, md_name_ap, md_name_len);
    if (NULL != md_name_ap)
        memcpy(msg_data_p->data.crosscheck_status.ma_name_ar, ma_name_ap, ma_name_len);

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais perirod
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            period      - the ais period
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : CFM_TYPE_AIS_PERIOD_1S, CFM_TYPE_AIS_PERIOD_60S
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetAisPeriod(
    char    *md_name_a,
    UI32_T  md_name_len,
    char    *ma_name_a,
    UI32_T  ma_name_len,
    UI32_T  period)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_ma_name_u32a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_ma_name_u32a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETAISPERIOD;

    msg_data_p->data.md_ma_name_u32a1.md_name_len = md_name_len;
    msg_data_p->data.md_ma_name_u32a1.ma_name_len = ma_name_len;
    msg_data_p->data.md_ma_name_u32a1.u32_a1      = period;

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.md_name_ar, md_name_a, md_name_len);

    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.ma_name_ar, ma_name_a, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}/*End of CFM_MGR_SetAisPeriod*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais level
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_level   - the ais level
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : it only can be configure from 1 to 7, bacause no level can notify level 0
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetAisLevel(
    char                *md_name_a,
    UI32_T              md_name_len,
    char                *ma_name_a,
    UI32_T              ma_name_len,
    CFM_TYPE_MdLevel_T  ais_level)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_ma_name_u32a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_ma_name_u32a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETAISLEVEL;

    msg_data_p->data.md_ma_name_u32a1.md_name_len = md_name_len;
    msg_data_p->data.md_ma_name_u32a1.ma_name_len = ma_name_len;
    msg_data_p->data.md_ma_name_u32a1.u32_a1      = ais_level;

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.md_name_ar, md_name_a, md_name_len);
    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.ma_name_ar, ma_name_a, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}/*End of CFM_MGR_SetAisLevel*/


/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais status
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_status  - the ais status
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetAisStatus(
    char                    *md_name_a,
    UI32_T                  md_name_len,
    char                    *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_ma_name_u32a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_ma_name_u32a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETAISSTATUS;

    msg_data_p->data.md_ma_name_u32a1.md_name_len = md_name_len;
    msg_data_p->data.md_ma_name_u32a1.ma_name_len = ma_name_len;
    msg_data_p->data.md_ma_name_u32a1.u32_a1      = ais_status;

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.md_name_ar, md_name_a, md_name_len);
    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.ma_name_ar, ma_name_a, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}/*End of CFM_MGR_SetAisStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais supress status
 * INPUT    : md_name_a          - the md name
 *            md_name_len        - the md name length
 *            ma_name_a          - the ma name
 *            ma_name_len        - the ma name length
 *            ais_supress_status - the ais suppress status
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetAisSuppressStatus(
    char                    *md_name_a,
    UI32_T                  md_name_len,
    char                    *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_supress_status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_ma_name_u32a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_ma_name_u32a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETAISSUPRESSSTATUS;

    msg_data_p->data.md_ma_name_u32a1.md_name_len = md_name_len;
    msg_data_p->data.md_ma_name_u32a1.ma_name_len = ma_name_len;
    msg_data_p->data.md_ma_name_u32a1.u32_a1      = ais_supress_status;

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.md_name_ar, md_name_a, md_name_len);

    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.md_ma_name_u32a1.ma_name_ar, ma_name_a, ma_name_len);


    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmDefaultMdIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level
 * INPUT    : md_primary_vid  - the md primary vid
 *           level            - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmDefaultMdIdPermission
    (UI16_T md_primary_vid, CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.vid_permission)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.vid_permission)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMDEFAULTMDIDPERMISSION ;

    msg_data_p->data.vid_permission.md_primary_vid = md_primary_vid;
    msg_data_p->data.vid_permission.send_id_permission = send_id_permission;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmDefaultMdLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level
 * INPUT    : md_primary_vid  - the md primary vid
 *           level            - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmDefaultMdLevel
            (UI16_T md_primary_vid, CFM_TYPE_MdLevel_T level)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.default_md_level)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.default_md_level)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMDEFAULTMDLEVEL ;

    msg_data_p->data.default_md_level.vid = md_primary_vid;
    msg_data_p->data.default_md_level.level = level;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma nam formate
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            format    - the ma name format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :  only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMa(UI32_T md_index, UI32_T ma_index)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMA;

    msg_data_p->data.u32a1_u32a2.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2.u32_a2 = ma_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma send ccm interval
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            interval  - the sending ccm interval
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *            only support 4-7
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaCcmInterval
    (UI32_T md_index, UI32_T ma_index, CFM_TYPE_CcmInterval_T interval)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_ccm_interval)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_ccm_interval)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMACCMINTERVAL;

    msg_data_p->data.ma_ccm_interval.md_index = md_index;
    msg_data_p->data.ma_ccm_interval.ma_index = ma_index;
    msg_data_p->data.ma_ccm_interval.interval = interval;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma nam formate
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            format    - the ma name format
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     :  only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaFormat
        (UI32_T md_index, UI32_T ma_index, CFM_TYPE_MA_Name_T format)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_format)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_format)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMAFORMAT;

    msg_data_p->data.ma_format.md_index = md_index;
    msg_data_p->data.ma_format.ma_index = ma_index;
    msg_data_p->data.ma_format.format = format;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaMepListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mep id list
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. not support this function, it will just return false
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaMepListEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMAMEPLISTENTRY;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mhf creation type
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            create_type -the mhf cration type
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *        CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaMhfCreation
        (UI32_T md_index, UI32_T ma_index, CFM_TYPE_MhfCreation_T create_type)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_creation)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_creation)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMAMHFCREATION;

    msg_data_p->data.ma_creation.md_index = md_index;
    msg_data_p->data.ma_creation.ma_index = ma_index;
    msg_data_p->data.ma_creation.create_type = create_type;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           ma_index   - the ma index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaMhfIdPermission
        (UI32_T md_index, UI32_T ma_index, CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_permission)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_permission)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMAMHFIDPERMISSION ;

    msg_data_p->data.ma_permission.md_index = md_index;
    msg_data_p->data.ma_permission.ma_index = ma_index;
    msg_data_p->data.ma_permission.send_id_permission = send_id_permission;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma name
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *name_ap    - the ma name array pointer
 *            name_length - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaName
        (UI32_T md_index, UI32_T ma_index, char *name_ap, UI32_T name_length)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_name_u32a3)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_name_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMANAME;

    msg_data_p->data.ma_name_u32a3.u32_a1 = md_index;
    msg_data_p->data.ma_name_u32a3.u32_a2 = ma_index;
    memcpy(msg_data_p->data.ma_name_u32a3.ma_name_ar, name_ap,
        sizeof(msg_data_p->data.ma_name_u32a3.ma_name_ar ));
    msg_data_p->data.ma_name_u32a3.u32_a3 = name_length;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMaPrimaryVlanVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma vlan id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            primary_vid - the vlan id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMaPrimaryVlanVid
            (UI32_T md_index, UI32_T ma_index, UI16_T primary_vid)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u16a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u16a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMAPRIMARYVLANVID;

    msg_data_p->data.u32a1_u32a2_u16a1.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u16a1.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u16a1.u16_a1 = primary_vid;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the md
 * INPUT    : md_index  - the md index
 *            md_format - the md format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMd(UI32_T md_index)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMD;

    msg_data_p->data.ui32_v = md_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMdFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md name formate
 * INPUT    : md_index  - the md index
 *            md_format - the md format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMdFormat(UI32_T md_index, CFM_TYPE_MD_Name_T md_format)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_format)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_format)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMDFORMAT;

    msg_data_p->data.md_format.md_index = md_index;
    msg_data_p->data.md_format.md_format = md_format;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMdLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md level
 * INPUT    : md_index  - the md index
 *            level     - the md level
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMdLevel(UI32_T md_index, CFM_TYPE_MdLevel_T level)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_index_by_level)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_index_by_level)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMDLEVEL ;

    msg_data_p->data.md_index_by_level.level = level;
    msg_data_p->data.md_index_by_level.md_index = md_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMdMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *            create_type - the mhf create type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMdMhfCreation(UI32_T md_index, CFM_TYPE_MhfCreation_T create_type)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_creation)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_creation)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMDMHFCREATION;

    msg_data_p->data.md_creation.md_index = md_index;
    msg_data_p->data.md_creation.create_type = create_type;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMdMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMdMhfIdPermission
    (UI32_T md_index, CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_permission)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_permission)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMDMHFIDPERMISSION;

    msg_data_p->data.md_permission.md_index = md_index;
    msg_data_p->data.md_permission.send_id_permission = send_id_permission;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMdName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md name
 * INPUT    : md_index    - the md index
 *            *name_ap    - the md name array pointer
 *            name_length - the md name length
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMdName(UI32_T md_index, char *name_ap, UI32_T name_length)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_index_by_name)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_index_by_name)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMDNAME ;

    msg_data_p->data.md_index_by_name.md_index = md_index;
    msg_data_p->data.md_index_by_name.name_len = name_length;
    memcpy(&msg_data_p->data.md_index_by_name.md_name_ar, name_ap,
        sizeof(msg_data_p->data.md_index_by_name.md_name_ar ));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepActive
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active    - the mep active status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepActive
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T active)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_bool)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPACTIVE;

    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1 = active;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepCciEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            status    - the cci status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_CCI_STATUS_ENABLE
 *               CFM_TYPE_CCI_STATUS_DISABLE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepCciEnable
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_CcmStatus_T cci_status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.mep_cci_status)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.mep_cci_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPCCIENABLE;

    msg_data_p->data.mep_cci_status.md_index = md_index;
    msg_data_p->data.mep_cci_status.ma_index = ma_index;
    msg_data_p->data.mep_cci_status.mep_id = mep_id;
    msg_data_p->data.mep_cci_status.status = cci_status;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ccm_ltm_priority- the ccm and ltm default priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepCcmLtmPriority
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ccm_ltm_priority)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPCCMLTMPRIORITY;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = ccm_ltm_priority;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            direction  - the mep direction
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepDirection
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id,CFM_TYPE_MP_Direction_T direction )
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.mep_direction_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.mep_direction_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPDIRECTION;

    msg_data_p->data.mep_direction_u32a3.u32_a1 = md_index;
    msg_data_p->data.mep_direction_u32a3.u32_a2 = ma_index;
    msg_data_p->data.mep_direction_u32a3.u32_a3 = mep_id;
    msg_data_p->data.mep_direction_u32a3.direction = direction;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the new mep
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPENTRY;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepFngAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fng alarm time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            alarm_time  - the fault alarm time by ticks
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepFngAlarmTime
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T alarm_time)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPFNGALARMTIME;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = alarm_time;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepFngResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm reset time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            reset_time- the fault alarm reset time by ticks
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepFngResetTime(
        UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T reset_time)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPFNGRESETTIME;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = reset_time;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepIfIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ifindex   - the logical port
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. the mep can't be configured on trunk member
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepIfIndex
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPIFINDEX;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = ifindex;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowest priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *               CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepLowPrDef(UI32_T md_index,
        UI32_T ma_index, UI32_T mep_id, CFM_TYPE_FNG_LowestAlarmPri_T low_pri)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.mep_low_pri)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.mep_low_pri)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPLOWPRDEF;

    msg_data_p->data.mep_low_pri.md_index = md_index;
    msg_data_p->data.mep_low_pri.ma_index = ma_index;
    msg_data_p->data.mep_low_pri.mep_id = mep_id;
    msg_data_p->data.mep_low_pri.priority = low_pri;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepPrimaryVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepPrimaryVid
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI16_T primary_vid)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u16a1)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u16a1)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPPRIMARYVID;

    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u16_a1 = primary_vid;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's tranmit lbm include data
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. this not suuport, it will just retrun true
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmDataTlv
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, char *content, UI32_T content_length)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.mep_error_ccm_fail)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.mep_error_ccm_fail)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDATATLV ;

    msg_data_p->data.mep_error_ccm_fail.md_index = md_index;
    msg_data_p->data.mep_error_ccm_fail.ma_index = ma_index;
    msg_data_p->data.mep_error_ccm_fail.mep_id = mep_id;
    msg_data_p->data.mep_error_ccm_fail.content_len = content_length;
    memcpy(&msg_data_p->data.mep_error_ccm_fail.content_ar,content,
        sizeof(msg_data_p->data.mep_error_ccm_fail.content_ar ));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmDestIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination use mep id or mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - set the destination address is mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmDestIsMepId
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id )
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_bool)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTISMEPID;

    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1 = is_mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmDestMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination mac address
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmDestMacAddress
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_mac)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_mac)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS;

    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a3 = mep_id;
    memcpy(&msg_data_p->data.u32a1_u32a2_u32a3_mac.mac_ar,dst_mac,
        sizeof(msg_data_p->data.u32a1_u32a2_u32a3_mac.mac_ar ));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmDestMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id - the lbm destination mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmDestMepId
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T dst_mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTMEPID;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = dst_mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmMessages
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit the lbm counts
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            counts    - the lbm message counts
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmMessages
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T counts)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMMESSAGES;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = counts;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function transmit the lbm
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lbm_status - the lbm status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmStatus
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMSTATUS;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmVlanDropEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's vlan drop ability
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_enable - the lbm vlan drop enable
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. this is not support will just return FALSE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmVlanDropEnable
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_enable)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_bool)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE;

    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1 = is_enable;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLbmVlanPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit blm vlan priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            priority  - the lbm priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLbmVlanPriority
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T priority)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMVLANPRIORITY;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = priority;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmEgressIdentifier
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's egress identifier
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            identifer - the egress identifier
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. this ability not suport, it will just return false
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmEgressIdentifier
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T identifer[8])
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_idary)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_idary)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER;

    msg_data_p->data.u32a1_u32a2_u32a3_idary.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_idary.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_idary.u32_a3 = mep_id;
    memcpy(&msg_data_p->data.u32a1_u32a2_u32a3_idary.identifer_ar,identifer,
        sizeof(msg_data_p->data.u32a1_u32a2_u32a3_idary.identifer_ar ));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            is_useFDBonly - set only use the fdb or not
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmFlags
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_useFDBonly)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_bool)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMFLAGS;

    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1 = is_useFDBonly;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmStatus
            (UI32_T md_index, UI32_T ma_index, UI32_T mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMSTATUS;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm target is the mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - the ltm target is the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetIsMepId
        (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_bool)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETISMEPID;

    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1 = is_mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mac-the ltm target address
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetMacAddress
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_mac)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_mac)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS;

    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a3 = mep_id;
    memcpy(&msg_data_p->data.u32a1_u32a2_u32a3_mac.mac_ar, target_mac,
        sizeof(msg_data_p->data.u32a1_u32a2_u32a3_mac.mac_ar ));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mp id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mep_id - the target mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmTargetMepId
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T target_mep_id)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETMEPID;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = target_mep_id;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmMepTransmitLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmMepTransmitLtmTtl
    (UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ttl)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3_u32a4)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTTL;

    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3 = mep_id;
    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4 = ttl;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_PMGR_SetDot1agCfmNumOfVids
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma can have more than on vid
 * INPUT    : md_index   - the md index
 *            ma_index   - the ma index
 *            vid_num    - can have more than one vid
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetDot1agCfmNumOfVids(UI32_T md_index, UI32_T ma_index, UI32_T vid_num)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2_u32a3)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETDOT1AGCFMNUMOFVIDS;

    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2 = ma_index;
    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3 = vid_num;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetFaultNotifyAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify alarm time
 * INPUT    : md_index   - the md index
 *            alarm_time - the fault notify alarm time in second
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetFaultNotifyAlarmTime(UI32_T md_index, UI32_T alarm_time)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETFAULTNOTIFYALARMTIME;

    msg_data_p->data.u32a1_u32a2.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2.u32_a2 = alarm_time;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify lowest priority
 * INPUT    : md_ndex  - the md index
 *            priority - the lowest fault notify priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :  this function will set all the mep under this domaiin to this priority
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetFaultNotifyLowestPriority
        (UI32_T md_index, CFM_TYPE_FNG_LowestAlarmPri_T priority)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.fault_notify_pri)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.fault_notify_pri)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETFAULTNOTIFYLOWESTPRIORITY ;

    msg_data_p->data.fault_notify_pri.md_index = md_index;
    msg_data_p->data.fault_notify_pri.priority = priority;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetFaultNotifyRestTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify reset time
 * INPUT    : md_index   - the md index
 *            reset_time - the fault notify rest time in second
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetFaultNotifyRestTime(UI32_T md_index, UI32_T reset_time)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.u32a1_u32a2)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETFAULTNOTIFYRESETTIME;

    msg_data_p->data.u32a1_u32a2.u32_a1 = md_index;
    msg_data_p->data.u32a1_u32a2.u32_a2 = reset_time;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetLinkTraceCacheHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache entries hold time
 * INPUT    : hold_time - the link trace cache entries hold time
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at hold_time parameter
 * NOTE     : input in minutes, engine run in second.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetLinkTraceCacheHoldTime(UI32_T hold_time)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETLINKTRACECACHEHOLDTIME;

    msg_data_p->data.ui32_v = hold_time;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetLinkTraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the link trace reply cache size
 * INPUT    : size - the link trace cache size
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at size parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetLinkTraceCacheSize(UI32_T size)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETLINKTRACECACHESIZE;

    msg_data_p->data.ui32_v = size;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetLinkTraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the link trace cache status.
 * INPUT    : status  - the link trace cache status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at status parameter
 * NOTE     : If the link cache status set enable, then the om start to record the link trace reply
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetLinkTraceCacheStatus(CFM_TYPE_LinktraceStatus_T status)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.linktrace_status)
        + CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.linktrace_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETLINKTRACECACHESTATUS ;

    msg_data_p->data.linktrace_status = status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create or modify the MA info
 * INPUT    : ma_index    - the ma index
 *            name_len    - the ma name
 *            md_index    - the md index
 *            primary_vid - the primary vid of the maS
 *            vlid_list   - the array store the vids
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetMA(UI32_T ma_index, char *name,
    UI32_T name_len, UI32_T md_index, UI16_T primary_vid, UI32_T vid_num,
    UI8_T vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1], CFM_TYPE_MhfCreation_T create_way)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_create_entry)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    memset(space_msg,0,SYSFUN_SIZE_OF_MSG(msg_buf_size));

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_create_entry)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETMA;

    msg_data_p->data.ma_create_entry.ma_index = ma_index;
    strncpy((char *)msg_data_p->data.ma_create_entry.ma_name_ar, name,
        sizeof(msg_data_p->data.ma_create_entry.ma_name_ar ));
    msg_data_p->data.ma_create_entry.name_len = name_len;
    msg_data_p->data.ma_create_entry.md_index = md_index;
    msg_data_p->data.ma_create_entry.primary_vid = primary_vid;
    msg_data_p->data.ma_create_entry.vid_num = vid_num;
    memcpy(&msg_data_p->data.ma_create_entry.vid_list_ar, vid_list,
        sizeof(msg_data_p->data.ma_create_entry.vid_list_ar ));
    msg_data_p->data.ma_create_entry.create_way = create_way;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetMANameFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : To set the name format of MA
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            name_format - the name format
 *                          (CFM_TYPE_MA_NAME_CHAR_STRING,
 *                           CFM_TYPE_MA_NAME_ICC_BASED)
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetMANameFormat(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_TYPE_MA_Name_T      name_format)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ma_format)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ma_format)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETMANAMEFORMAT;

    msg_data_p->data.ma_format.md_index = md_index;
    msg_data_p->data.ma_format.ma_index = ma_index;
    msg_data_p->data.ma_format.format = name_format;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create or modify the MD
 * INPUT    : md_index - the MD index
 *            *name_ap - the MD name array pointer
 *            name_len - the MD name length
 *            level    - the MD level
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR    - Operation mode error
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetMD(UI32_T md_index,
    char *name_ap, UI16_T name_len, CFM_TYPE_MdLevel_T level, CFM_TYPE_MhfCreation_T create_way)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.md_create_entry)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    memset(space_msg,0,SYSFUN_SIZE_OF_MSG(msg_buf_size));
    /*assign size*/
    req_size=sizeof(msg_data_p->data.md_create_entry)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETMD;

    msg_data_p->data.md_create_entry.md_index = md_index;
    msg_data_p->data.md_create_entry.level = level;

    if (NULL != name_ap)
    {
        strncpy((char *)&msg_data_p->data.md_create_entry.md_name_ar, name_ap, name_len);
    }
    msg_data_p->data.md_create_entry.name_len = name_len;

    msg_data_p->data.md_create_entry.create_way = create_way;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetSNMPCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set which SNMP trap enable
 * INPUT    : trap         - the snamp CC trap type
 *            trap_enabled - the trap status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set faileds
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetSNMPCcStatus(CFM_TYPE_SnmpTrapsCC_T trap, BOOL_T trap_enabled)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.snmp_cc_status)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.snmp_cc_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETSNMPCCSTATUS;

    msg_data_p->data.snmp_cc_status.trap = trap;
    msg_data_p->data.snmp_cc_status.trap_enabled = trap_enabled;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SetSNMPCrosscheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the the SNMP trap status
 * INPUT    : trap        - the snmp cross check trap type
 *            trap_enable - the trap status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set faileds
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_SetSNMPCrosscheckStatus
        (CFM_TYPE_SnmpTrapsCrossCheck_T trap, BOOL_T trap_enabled)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.snmp_trap_cc_status)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.snmp_trap_cc_status)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_SETSNMPCROSSCHECKSTATUS;

    msg_data_p->data.snmp_trap_cc_status.trap = trap;
    msg_data_p->data.snmp_trap_cc_status.trap_enabled = trap_enabled;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_TransmitLinktrace
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the link trace message be sent
 * INPUT    : src_mep_id  - the source mep id
 *            dst_mep_id  - the destination mep id
 *            mac_addr    - the dest mac address of link trace message
 *            md_name_a   - the md name array
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name array
 *            ma_name_len - the ma name length
 *            ttl         - the time to live
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail transmit linktrace
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_TransmitLinktrace(
    UI32_T  src_mep_id,
    UI32_T  dst_mep_id,
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    char    md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T  md_name_len,
    char    ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  ma_name_len,
    UI32_T  ttl,
    UI16_T  pkt_pri)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.transmit_settings)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.transmit_settings)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_TRANSMITLINKTRACE;

    msg_data_p->data.transmit_settings.u32_a1 = src_mep_id;
    msg_data_p->data.transmit_settings.u32_a2 = dst_mep_id;
    msg_data_p->data.transmit_settings.u32_a3 = ttl;
    msg_data_p->data.transmit_settings.md_name_len = md_name_len;
    msg_data_p->data.transmit_settings.ma_name_len = ma_name_len;
    msg_data_p->data.transmit_settings.u32_a4      = pkt_pri;

    if ( 0==dst_mep_id )
    {
        memcpy(msg_data_p->data.transmit_settings.mac_ar, mac_addr,
        sizeof(msg_data_p->data.transmit_settings.mac_ar ));
    }

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.transmit_settings.md_name_ar, md_name_a, md_name_len);

    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.transmit_settings.ma_name_ar, ma_name_a, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_TransmitLoopback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the loop back message be sent
 * INPUT    : mep_id      - the target mep_id
 *            mac_addr    - the target dest mac
 *            md_name_a   - the md name array
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail transmit loopback
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_TransmitLoopback(
    UI32_T      mep_id,
    UI8_T       mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    char        md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T      md_name_len,
    char        ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T      ma_name_len,
    UI32_T      counts)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.transmit_settings)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.transmit_settings)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_TRANSMITLOOPBACK;

    msg_data_p->data.transmit_settings.u32_a1 = mep_id;
    msg_data_p->data.transmit_settings.u32_a2 = counts;
    msg_data_p->data.transmit_settings.md_name_len = md_name_len;
    msg_data_p->data.transmit_settings.ma_name_len = ma_name_len;

    if (0==mep_id )
    {
        memcpy(msg_data_p->data.transmit_settings.mac_ar, mac_addr,
        sizeof(msg_data_p->data.transmit_settings.mac_ar ));
    }

    if (NULL != md_name_a)
        memcpy(msg_data_p->data.transmit_settings.md_name_ar, md_name_a,
            md_name_len +1);

    if (NULL != ma_name_a)
        memcpy(msg_data_p->data.transmit_settings.ma_name_ar, ma_name_a,
            ma_name_len +1);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ProcessPortAdminDisable
 *-------------------------------------------------------------------------
 * PURPOSE  : the function process the port admin disable
 * INPUT    : lport - the shutdowned lport
 * OUTPUT   : None
 * RETURN   : None
  * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_PMGR_ProcessPortAdminDisable(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.ui32_v)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.ui32_v)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=msg_buf_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_PROCESSPORTADMINDISABLED;

    msg_data_p->data.ui32_v = lport;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return;
    }
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_ClearAisError
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear ais error
 * INPUT    : mep_id      - the mep id
 *            md_name_p   - pointer to md name
 *            md_name_len - md name length
 *            ma_name_p   - pointer to ma name
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail transmit loopback
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_ClearAisError(
    UI32_T  mep_id,
    char    *md_name_p,
    UI32_T  md_name_len,
    char    *ma_name_p,
    UI32_T  ma_name_len)
{
    SYSFUN_Msg_T *msg_p;
    CFM_MGR_IPCMsg_T *msg_data_p;
    const UI32_T buff_size=sizeof(CFM_MGR_DATA.md_ma_name_u32a1)
                                       +CFM_MGR_MSGBUF_TYPE_SIZE;
    UI8_T  space[SYSFUN_SIZE_OF_MSG(buff_size)];

    /*assign size*/
    msg_p=(SYSFUN_Msg_T *) &space;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;

    msg_data_p->data.md_ma_name_u32a1.md_name_len = md_name_len;
    msg_data_p->data.md_ma_name_u32a1.ma_name_len = ma_name_len;
    msg_data_p->data.md_ma_name_u32a1.u32_a1      = mep_id;

    if (NULL != md_name_p)
        memcpy(msg_data_p->data.md_ma_name_u32a1.md_name_ar, md_name_p, md_name_len);
    if (NULL != ma_name_p)
        memcpy(msg_data_p->data.md_ma_name_u32a1.ma_name_ar, ma_name_p, ma_name_len);

    if (FALSE == CFM_PMGR_SendMsg(msg_p,
                                CFM_MGR_IPCCMD_CLEARAISERROR,
                                buff_size,
                                CFM_MGR_MSGBUF_TYPE_SIZE))
        return CFM_TYPE_CONFIG_ERROR;

    return msg_data_p->type.result_ui32;
}/*End of CFM_PMGR_ClearAisError*/

#if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AbortDelayMeasureByDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the delay measure in progress.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_AbortDelayMeasureByDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    char        md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    char        ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len)
{
    const UI32_T        msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.abort_xmit)
                            +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T        *msg_p;
    CFM_MGR_IPCMsg_T    *msg_data_p;
    UI32_T              req_size, resp_size;
    UI8_T               space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    /*assign size*/
    req_size=sizeof(msg_data_p->data.abort_xmit)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_ABORTDELAYMEASUREBYDMM;

    msg_data_p->data.abort_xmit.src_mep_id = src_mep_id;
    msg_data_p->data.abort_xmit.dst_mep_id = dst_mep_id;
    msg_data_p->data.abort_xmit.md_name_len= md_name_len;
    msg_data_p->data.abort_xmit.ma_name_len= ma_name_len;

    memcpy(&msg_data_p->data.abort_xmit.dst_mac_ar, dst_mac_ar,
    sizeof(msg_data_p->data.abort_xmit.dst_mac_ar));

    if (NULL != ma_name_ar)
        memcpy(msg_data_p->data.abort_xmit.md_name_ar, md_name_ar, md_name_len);
    if (NULL != ma_name_ar)
        memcpy(msg_data_p->data.abort_xmit.ma_name_ar, ma_name_ar, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DoDelayMeasureByDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To do delay measure by sending DMM.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 *            interval    - the transmit interval
 *            timeout     - the timeout for waiting dmr
 *            pkt_size    - the transmit packet size
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DoDelayMeasureByDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    char        md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    char        ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len,
    UI32_T      counts,
    UI32_T      interval,
    UI32_T      timeout,
    UI32_T      pkt_size,
    UI16_T      pkt_pri)
{
    const UI32_T        msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.transmit_dmm)
                            +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T        *msg_p;
    CFM_MGR_IPCMsg_T    *msg_data_p;
    UI32_T              req_size, resp_size;
    UI8_T               space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    /*assign size*/
    req_size=sizeof(msg_data_p->data.transmit_dmm)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DODELAYMEASUREBYDMM;

    msg_data_p->data.transmit_dmm.src_mep_id = src_mep_id;
    msg_data_p->data.transmit_dmm.dst_mep_id = dst_mep_id;
    msg_data_p->data.transmit_dmm.counts     = counts;
    msg_data_p->data.transmit_dmm.interval   = interval;
    msg_data_p->data.transmit_dmm.timeout    = timeout;
    msg_data_p->data.transmit_dmm.pkt_size   = pkt_size;
    msg_data_p->data.transmit_dmm.md_name_len= md_name_len;
    msg_data_p->data.transmit_dmm.ma_name_len= ma_name_len;
    msg_data_p->data.transmit_dmm.pkt_pri    = pkt_pri;

    if(0 == dst_mep_id )
    {
        memcpy(&msg_data_p->data.transmit_dmm.dst_mac_ar, dst_mac_ar,
        sizeof(msg_data_p->data.transmit_dmm.dst_mac_ar));
    }

    if (NULL != md_name_ar)
    {
        memcpy(msg_data_p->data.transmit_dmm.md_name_ar, md_name_ar,
            md_name_len+1);
    }

    if (NULL != ma_name_ar)
    {
        memcpy(msg_data_p->data.transmit_dmm.ma_name_ar, ma_name_ar,
            ma_name_len+1);
    }

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return msg_data_p->type.result_ui32;
}
#endif /* #if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_AbortThrptMeasureByLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the throughput measure in progressing.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_AbortThrptMeasureByLBM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    char        md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    char        ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len)
{
    const UI32_T        msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.abort_xmit)
                            +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T        *msg_p;
    CFM_MGR_IPCMsg_T    *msg_data_p;
    UI32_T              req_size, resp_size;
    UI8_T               space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    /*assign size*/
    req_size=sizeof(msg_data_p->data.abort_xmit)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_ABORTTHRPTMEASUREBYLBM;

    msg_data_p->data.abort_xmit.src_mep_id = src_mep_id;
    msg_data_p->data.abort_xmit.dst_mep_id = dst_mep_id;
    msg_data_p->data.abort_xmit.md_name_len= md_name_len;
    msg_data_p->data.abort_xmit.ma_name_len= ma_name_len;
    memcpy(msg_data_p->data.abort_xmit.dst_mac_ar, dst_mac_ar,
    sizeof(msg_data_p->data.abort_xmit.dst_mac_ar));
    if (NULL != md_name_ar)
        memcpy(msg_data_p->data.abort_xmit.md_name_ar, md_name_ar, md_name_len);
    if (NULL != ma_name_ar)
        memcpy(msg_data_p->data.abort_xmit.ma_name_ar, ma_name_ar, ma_name_len);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_DoThrptMeasureByLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To do throughput measure by sending LBM.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 *            pkt_size    - the transmit packet size
 *            pattern     - the pattern included in data TLV
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_PMGR_DoThrptMeasureByLBM(
    UI32_T  src_mep_id,
    UI32_T  dst_mep_id,
    UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    char    md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T  md_name_len,
    char    ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T  ma_name_len,
    UI32_T  counts,
    UI32_T  pkt_size,
    UI16_T  pattern,
    UI16_T  pkt_pri)
{
    const UI32_T msg_buf_size=(sizeof(((CFM_MGR_IPCMsg_T *)0)->data.do_thrpt_by_lbm)
        +CFM_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T        *msg_p;
    CFM_MGR_IPCMsg_T    *msg_data_p;
    UI8_T               space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T              req_size, resp_size;

    /*assign size*/
    req_size=sizeof(msg_data_p->data.do_thrpt_by_lbm)+CFM_MGR_MSGBUF_TYPE_SIZE;
    resp_size=CFM_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_p->msg_size = req_size;

    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = CFM_MGR_IPCCMD_DOTHRPTMEASUREBYLBM;

    msg_data_p->data.do_thrpt_by_lbm.src_mep_id = src_mep_id;
    msg_data_p->data.do_thrpt_by_lbm.dst_mep_id = dst_mep_id;
    msg_data_p->data.do_thrpt_by_lbm.md_name_len= md_name_len;
    msg_data_p->data.do_thrpt_by_lbm.ma_name_len= ma_name_len;
    msg_data_p->data.do_thrpt_by_lbm.counts     = counts;
    msg_data_p->data.do_thrpt_by_lbm.pkt_size   = pkt_size;
    msg_data_p->data.do_thrpt_by_lbm.pattern    = pattern;
    msg_data_p->data.do_thrpt_by_lbm.pkt_pri    = pkt_pri;

    if (0==dst_mep_id )
    {
        memcpy(msg_data_p->data.do_thrpt_by_lbm.dst_mac_ar, dst_mac_ar,
        sizeof(msg_data_p->data.do_thrpt_by_lbm.dst_mac_ar));
    }

    if (NULL != md_name_ar)
        memcpy(msg_data_p->data.do_thrpt_by_lbm.md_name_ar, md_name_ar, md_name_len+1);

    if (NULL != ma_name_ar)
        memcpy(msg_data_p->data.do_thrpt_by_lbm.ma_name_ar, ma_name_ar, ma_name_len+1);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    return msg_data_p->type.result_ui32;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the IGMPSNP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of request message.
 *           res_size  - the size of response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : TRUE   - success
 *                 FALSe - fail
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_PMGR_SendMsg(SYSFUN_Msg_T *msg_p,
                               UI32_T       cmd,
                               const UI32_T req_size,
                               const UI32_T resp_size)
{
    CFM_MGR_IPCMsg_T *msg_data_p;

    /*assign size*/
    msg_p->msg_size = req_size;
    msg_p->cmd = SYS_MODULE_CFM;
    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = cmd;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
}
#endif /*#if (SYS_CPNT_CFM == TRUE)*/
