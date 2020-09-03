/* MODULE NAME:  aaa_pom.c
 * PURPOSE: For accessing om through IPC
 *
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    8/8/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"

#include "sys_module.h"
#include "sysfun.h"
#include "aaa_om.h"
#include "aaa_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : AAA_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
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
void AAA_POM_Init(void)
{
    /* Given that SWCTRL is run in MGMT_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_AUTH_PROTOCOL_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next radius group entry by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningradiusgroupentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningradiusgroupentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningradiusgroupentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRUNNINGRADIUSGROUPENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningradiusgroupentry.req.group_index=entry->group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *entry=msg_data_p->data.getnextrunningradiusgroupentry.resp.entry;

    return msg_data_p->type.result_ui32;
}

 /*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetRadiusGroupEntry_Ex(AAA_RadiusGroupEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusgroupentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusgroupentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusgroupentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRADIUSGROUPENTRY_EX;

    /*assign input*/
    memcpy(msg_data_p->data.getradiusgroupentry_ex.req.group_name,
        entry->group_name,sizeof(entry->group_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    entry->group_index=msg_data_p->data.getradiusgroupentry_ex.resp.group_index;

    /*assign output*/

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusgroupentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusgroupentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusgroupentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRADIUSGROUPENTRY;

    /*assign input*/
    msg_data_p->data.getnextradiusgroupentry.req.group_index=entry->group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextradiusgroupentry.resp.entry;

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next tacacs group entry by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningtacacsplusgroupentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningtacacsplusgroupentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningtacacsplusgroupentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRUNNINGTACACSPLUSGROUPENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningtacacsplusgroupentry.req.group_index=entry->group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *entry=msg_data_p->data.getnextrunningtacacsplusgroupentry.resp.entry;

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetTacacsPlusGroupEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetTacacsPlusGroupEntry_Ex(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusgroupentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusgroupentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusgroupentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETTACACSPLUSGROUPENTRY_EX;

    /*assign input*/
    memcpy(msg_data_p->data.gettacacsplusgroupentry_ex.req.group_name,entry->group_name,
        sizeof(msg_data_p->data.gettacacsplusgroupentry_ex.req.group_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    entry->group_index=msg_data_p->data.gettacacsplusgroupentry_ex.resp.group_index;


    /*assign output*/

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusgroupentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusgroupentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusgroupentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTTACACSPLUSGROUPENTRY;

    /*assign input*/
    msg_data_p->data.getnexttacacsplusgroupentry.req.group_index=entry->group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnexttacacsplusgroupentry.resp.entry;

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index, entry->radius_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningradiusentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningradiusentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningradiusentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRUNNINGRADIUSENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningradiusentry.req.group_index=group_index;
    msg_data_p->data.getnextrunningradiusentry.req.radius_index=entry->radius_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *entry=msg_data_p->data.getnextrunningradiusentry.resp.entry;

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index (1-based), entry->radius_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRADIUSENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningradiusentry.req.group_index=group_index;
    msg_data_p->data.getnextrunningradiusentry.req.radius_index=entry->radius_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *entry=msg_data_p->data.getnextrunningradiusentry.resp.entry;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRadiusEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based);
 * OUTPUT   : radius_server_index (1-based);
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetRadiusEntry_Ex(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRADIUSENTRY_EX;

    /*assign input*/
    msg_data_p->data.getradiusentry_ex.req.group_index=group_index;
    msg_data_p->data.getradiusentry_ex.req.radius_index=entry->radius_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    entry->radius_server_index=msg_data_p->data.getradiusentry_ex.resp.radius_server_index;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:
 *--------------------------------AAA_POM_GetRadiusEntryOrder-----------------------------------------
 * PURPOSE  : get the radius entry's order in group
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetRadiusEntryOrder(UI16_T group_index, UI16_T radius_index, UI16_T *order)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusentryorder)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusentryorder.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusentryorder.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRADIUSENTRYORDER;

    /*assign input*/
    msg_data_p->data.getradiusentryorder.req.group_index=group_index;
    msg_data_p->data.getradiusentryorder.req.radius_index=radius_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *order=msg_data_p->data.getradiusentryorder.resp.order;

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_IsRadiusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_IsRadiusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.isradiusgroupvalid)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.isradiusgroupvalid.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.isradiusgroupvalid.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_ISRADIUSGROUPVALID;

    /*assign input*/
    msg_data_p->data.isradiusgroupvalid.req.group_index=group_index;
    msg_data_p->data.isradiusgroupvalid.req.client_type=client_type;
    msg_data_p->data.isradiusgroupvalid.req.ifindex=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_IsRadiusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the radius entry is valid or not according to input params
 * INPUT    : radius_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_IsRadiusEntryValid(UI16_T radius_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.isradiusentryvalid)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.isradiusentryvalid.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.isradiusentryvalid.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_ISRADIUSENTRYVALID;

    /*assign input*/
    msg_data_p->data.isradiusentryvalid.req.radius_index=radius_index;
    msg_data_p->data.isradiusentryvalid.req.client_type=client_type;
    msg_data_p->data.isradiusentryvalid.req.ifindex=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}
#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs plus accounting entry by group_index and tacacs_index.
 * INPUT    : group_index, entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningtacacsplusentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningtacacsplusentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningtacacsplusentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRUNNINGTACACSPLUSENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningtacacsplusentry.req.group_index=group_index;
    msg_data_p->data.getnextrunningtacacsplusentry.req.tacacs_index=entry->tacacs_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *entry=msg_data_p->data.getnextrunningtacacsplusentry.resp.entry;

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs plus accounting entry by group_index and tacacs_index.
 * INPUT    : group_index (1-based), entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTTACACSPLUSENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningtacacsplusentry.req.group_index=group_index;
    msg_data_p->data.getnextrunningtacacsplusentry.req.tacacs_index=entry->tacacs_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *entry=msg_data_p->data.getnextrunningtacacsplusentry.resp.entry;

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs plus entry by group_index, tacacs_index
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : tacacs_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetTacacsPlusEntry_Ex(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETTACACSPLUSENTRY_EX;

    /*assign input*/
    msg_data_p->data.gettacacsplusentry_ex.req.group_index=group_index;
    msg_data_p->data.gettacacsplusentry_ex.req.tacacs_index=entry->tacacs_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    entry->tacacs_server_index = msg_data_p->data.gettacacsplusentry_ex.resp.tacacs_server_index;

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting update interval (minutes)
 * INPUT    : none
 * OUTPUT   : update_interval
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccUpdateInterval(UI32_T *update_interval)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccupdateinterval)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccupdateinterval.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccupdateinterval.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRUNNINGACCUPDATEINTERVAL;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *update_interval= msg_data_p->data.getrunningaccupdateinterval.resp.update_interval;

    return msg_data_p->type.result_ui32;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting update interval (minutes)
 * INPUT    : none
 * OUTPUT   : update_interval
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccUpdateInterval(UI32_T *update_interval)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccupdateinterval)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccupdateinterval.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccupdateinterval.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETACCUPDATEINTERVAL;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *update_interval= msg_data_p->data.getaccupdateinterval.resp.update_interval;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccExecEntry(AAA_AccExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccexecentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccexecentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccexecentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRUNNINGACCEXECENTRY;

    /*assign input*/
    msg_data_p->data.getrunningaccexecentry.req.exec_type=entry->exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(entry->list_name,
        msg_data_p->data.getrunningaccexecentry.resp.list_name,sizeof(entry->list_name));
    entry->configure_mode=msg_data_p->data.getrunningaccexecentry.resp.configure_mode;

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccExecEntry(AAA_AccExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextaccexecentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextaccexecentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextaccexecentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTACCEXECENTRY;

    /*assign input*/
    msg_data_p->data.getnextaccexecentry.req.exec_type=entry->exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *entry=msg_data_p->data.getnextaccexecentry.resp.entry;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccExecEntry_Ex(AAA_AccExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccexecentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccexecentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccexecentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETACCEXECENTRY_EX;

    /*assign input*/
    msg_data_p->data.getaccexecentry_ex.req.exec_type=entry->exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(entry->list_name,
        msg_data_p->data.getrunningaccexecentry.resp.list_name,sizeof(entry->list_name));
    entry->configure_mode=msg_data_p->data.getrunningaccexecentry.resp.configure_mode;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccdot1xentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccdot1xentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningaccdot1xentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRUNNINGACCDOT1XENTRY;

    /*assign input*/
    msg_data_p->data.getrunningaccdot1xentry.req.ifindex=entry->ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memcpy(entry->list_name,
        msg_data_p->data.getrunningaccdot1xentry.resp.list_name,sizeof(entry->list_name));
    entry->configure_mode=msg_data_p->data.getrunningaccdot1xentry.resp.configure_mode;


    /*assign output*/

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccDot1xEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccDot1xEntry_Ex(AAA_AccDot1xEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccdot1xentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccdot1xentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccdot1xentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETACCDOT1XENTRY_EX;

    /*assign input*/
    msg_data_p->data.getaccdot1xentry_ex.req.ifindex=entry->ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(entry->list_name,
        msg_data_p->data.getaccdot1xentry_ex.resp.list_name,sizeof(entry->list_name));
    entry->configure_mode=msg_data_p->data.getaccdot1xentry_ex.resp.configure_mode;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningAccListEntry(AAA_AccListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningacclistentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningacclistentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningacclistentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRUNNINGACCLISTENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningacclistentry.req.list_index=entry->list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *entry=msg_data_p->data.getnextrunningacclistentry.resp.entry;

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccListEntry(AAA_AccListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacclistentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacclistentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacclistentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTACCLISTENTRY;

    /*assign input*/
    msg_data_p->data.getnextacclistentry.req.list_index=entry->list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextacclistentry.resp.entry;

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccListEntryFilterByClientType(AAA_AccListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacclistentryfilterbyclienttype)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacclistentryfilterbyclienttype.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacclistentryfilterbyclienttype.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTACCLISTENTRYFILTERBYCLIENTTYPE;

    /*assign input*/
    msg_data_p->data.getnextacclistentryfilterbyclienttype.req.list_index=entry->list_index;
    msg_data_p->data.getnextacclistentryfilterbyclienttype.req.client_type=entry->client_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextacclistentryfilterbyclienttype.resp.entry;

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_QueryAccDot1xPortList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port list associated with specific list_index
 * INPUT    : query_result->list_index
 * OUTPUT   : query_result->port_list
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_QueryAccDot1xPortList(AAA_QueryAccDot1xPortListResult_T *query_result)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.queryaccdot1xportlist)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.queryaccdot1xportlist.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.queryaccdot1xportlist.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_QUERYACCDOT1XPORTLIST;

    /*assign input*/
    msg_data_p->data.queryaccdot1xportlist.req.list_index=query_result->list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    memcpy(query_result->port_list,
        msg_data_p->data.queryaccdot1xportlist.resp.port_list,sizeof(query_result->port_list));

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccCommandEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the command accounting entry by specified privilege level
 *            and EXEC type
 * INPUT    : entry_p->priv_lvl
 * OUTPUT   : entry_p
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccCommandEntryInterface(AAA_AccCommandEntry_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getacccommandentryinf)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getacccommandentryinf.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getacccommandentryinf.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETACCCOMMANDENTRYINTERFACE;

    /*assign input*/
    msg_data_p->data.getacccommandentryinf.req.entry = *entry_p;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry_p = msg_data_p->data.getacccommandentryinf.resp.entry;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next command accounting entry by index.
 * INPUT    : index, use 0 to get first
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccCommandEntry(UI32_T *index, AAA_AccCommandEntry_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacccommandentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacccommandentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextacccommandentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTACCCOMMANDENTRY;

    /*assign input*/
    msg_data_p->data.getnextacccommandentry.req.index = *index;
    msg_data_p->data.getnextacccommandentry.req.entry = *entry_p;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *index = msg_data_p->data.getnextacccommandentry.resp.index;
    *entry_p = msg_data_p->data.getnextacccommandentry.resp.entry;

    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the command accounting entry by priv-lvl and exec type.
 * INPUT    : entry_p->priv_lvl, entry_p->exec_type
 * OUTPUT   : entry_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccCommandEntry(AAA_AccCommandEntry_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getacccommandentryinf)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getacccommandentryinf.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getacccommandentryinf.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRUNNINGACCCOMMANDENTRY;

    /*assign input*/
    msg_data_p->data.getacccommandentryinf.req.entry = *entry_p;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry_p = msg_data_p->data.getacccommandentryinf.resp.entry;

    return msg_data_p->type.result_bool;
}

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

#endif



#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetMethodTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get the AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getmethodtable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getmethodtable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getmethodtable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETMETHODTABLE;

    /*assign input*/
    msg_data_p->data.getmethodtable.req.method_index=method_index;
    msg_data_p->data.getmethodtable.req.list_index=entry->list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry->list_name,msg_data_p->data.getmethodtable.resp.list_name
        ,sizeof(entry->list_name));
    memcpy(entry->group_name,msg_data_p->data.getmethodtable.resp.group_name
        ,sizeof(entry->group_name));
    entry->client_type=msg_data_p->data.getmethodtable.resp.client_type;
    entry->group_type=msg_data_p->data.getmethodtable.resp.group_type;
    entry->working_mode=msg_data_p->data.getmethodtable.resp.working_mode;

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextMethodTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get the next AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetNextMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextmethodtable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextmethodtable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextmethodtable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTMETHODTABLE;

    /*assign input*/
    msg_data_p->data.getnextmethodtable.req.method_index=method_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextmethodtable.resp.entry;

    return msg_data_p->type.result_bool;
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetRadiusGroupTable(UI16_T radius_group_index, AAA_RadiusGroupEntryMIBInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusgrouptable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusgrouptable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getradiusgrouptable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRADIUSGROUPTABLE;

    /*assign input*/
    msg_data_p->data.getradiusgrouptable.req.radius_group_index=radius_group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getradiusgrouptable.resp.entry;
    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetNextRadiusGroupTable(UI16_T radius_group_index, AAA_RadiusGroupEntryMIBInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusgrouptable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusgrouptable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextradiusgrouptable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRADIUSGROUPTABLE;

    /*assign input*/
    msg_data_p->data.getnextradiusgrouptable.req.radius_group_index=radius_group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextradiusgrouptable.resp.entry;

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusgrouptable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusgrouptable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.gettacacsplusgrouptable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETTACACSPLUSGROUPTABLE;

    /*assign input*/
    msg_data_p->data.gettacacsplusgrouptable.req.tacacsplus_group_index=tacacsplus_group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.gettacacsplusgrouptable.resp.entry;

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusgrouptable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusgrouptable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnexttacacsplusgrouptable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTTACACSPLUSGROUPTABLE;

    /*assign input*/
    msg_data_p->data.getnexttacacsplusgrouptable.req.tacacsplus_group_index=tacacsplus_group_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnexttacacsplusgrouptable.resp.entry;

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetUpdate
 * ------------------------------------------------------------------------
 * PURPOSE  :   Setting update interval
 * INPUT    :   update_interval
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetUpdate(UI32_T *update_interval)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getupdate)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getupdate.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getupdate.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETUPDATE;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *update_interval=msg_data_p->data.getupdate.resp.update_interval;
    return msg_data_p->type.result_bool;
}


#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetAccountTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_POM_GetAccountTable entry by ifindex.
 * INPUT    :   ifindex
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccounttable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccounttable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getaccounttable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETACCOUNTTABLE;

    /*assign input*/
    msg_data_p->data.getaccounttable.req.ifindex=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getaccounttable.resp.entry;

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextAccountTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetNextAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextaccounttable)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextaccounttable.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextaccounttable.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTACCOUNTTABLE;

    /*assign input*/
    msg_data_p->data.getnextaccounttable.req.ifindex=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextaccounttable.resp.entry;

    return msg_data_p->type.result_bool;
}

#endif
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)
#if (SYS_CPNT_ACCOUNTING == TRUE)


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_POM_Backdoor_ShowAccUser
 * ---------------------------------------------------------------------
 * PURPOSE  : show accounting user information
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : for aaa backdoor
 * ---------------------------------------------------------------------*/
void AAA_POM_Backdoor_ShowAccUser()
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.backdoor_showaccuser);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.backdoor_showaccuser.req);
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.backdoor_showaccuser.resp);

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_BACKDOOR_SHOWACCUSER;


    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return;
    }

    return;
}
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */
#if (SYS_CPNT_AUTHORIZATION == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAuthorListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAuthorListEntryFilterByClientType(AAA_AuthorListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextauthorlistentryfilterbyclienttype)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextauthorlistentryfilterbyclienttype.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextauthorlistentryfilterbyclienttype.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTAUTHORLISTENTRYFILTERBYCLIENTTYPE;

    /*assign input*/
    msg_data_p->data.getnextauthorlistentryfilterbyclienttype.req.list_index=entry->list_index;
    msg_data_p->data.getnextauthorlistentryfilterbyclienttype.req.list_type = entry->list_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextauthorlistentryfilterbyclienttype.resp.entry;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAuthorExecEntry(AAA_AuthorExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextauthorexecentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextauthorexecentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextauthorexecentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTAUTHOREXECENTRY;

    /*assign input*/
    msg_data_p->data.getnextauthorexecentry.req.exec_type=entry->exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextauthorexecentry.resp.entry;
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAuthorExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAuthorExecEntry_Ex(AAA_AuthorExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getauthorexecentry_ex)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getauthorexecentry_ex.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getauthorexecentry_ex.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETAUTHOREXECENTRY_EX;

    /*assign input*/
    msg_data_p->data.getauthorexecentry_ex.req.exec_type=entry->exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry->list_name,
        msg_data_p->data.getauthorexecentry_ex.resp.list_name,sizeof(entry->list_name));
    entry->configure_mode=msg_data_p->data.getauthorexecentry_ex.resp.configure_mode;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningauthorexecentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningauthorexecentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getrunningauthorexecentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETRUNNINGAUTHOREXECENTRY;

    /*assign input*/
    msg_data_p->data.getauthorexecentry_ex.req.exec_type=entry->exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memcpy(entry->list_name,
        msg_data_p->data.getauthorexecentry_ex.resp.list_name,sizeof(entry->list_name));
    entry->configure_mode=msg_data_p->data.getauthorexecentry_ex.resp.configure_mode;

    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningAuthorListEntry(AAA_AuthorListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningauthorlistentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningauthorlistentry.req)
        + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_OM_IPCMsg_T *)0)->data.getnextrunningauthorlistentry.resp)
        + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GETNEXTRUNNINGAUTHORLISTENTRY;

    /*assign input*/
    msg_data_p->data.getnextrunningauthorlistentry.req.list_index=entry->list_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *entry=msg_data_p->data.getnextrunningauthorlistentry.resp.entry;

    return msg_data_p->type.result_ui32;
}

#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry)
{
    const UI32_T msg_buf_size = (sizeof(((AAA_OM_IPCMsg_T *)0)->data.get_author_command_entry)
                                                    + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size = sizeof(((AAA_OM_IPCMsg_T *)0)->data.get_author_command_entry.req)
                                                    + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size = sizeof(((AAA_OM_IPCMsg_T *)0)->data.get_author_command_entry.resp)
                                                    + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GET_AUTHOR_COMMAND_ENTRY;

    /*assign input*/
    msg_data_p->data.get_author_command_entry.req.priv_lvl  = priv_lvl;
    msg_data_p->data.get_author_command_entry.req.exec_type = exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry = msg_data_p->data.get_author_command_entry.resp.entry;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry)
{
    const UI32_T msg_buf_size = (sizeof(((AAA_OM_IPCMsg_T *)0)->data.get_author_command_entry)
                                                    + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size = sizeof(((AAA_OM_IPCMsg_T *)0)->data.get_author_command_entry.req)
                                                    + AAA_OM_MSGBUF_TYPE_SIZE;
    resp_size = sizeof(((AAA_OM_IPCMsg_T *)0)->data.get_author_command_entry.resp)
                                                    + AAA_OM_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p = (AAA_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_OM_IPCCMD_GET_RUNNING_AUTHOR_COMMAND_ENTRY;

    /*assign input*/
    msg_data_p->data.get_author_command_entry.req.priv_lvl  = priv_lvl;
    msg_data_p->data.get_author_command_entry.req.exec_type = exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry = msg_data_p->data.get_author_command_entry.resp.entry;

    return msg_data_p->type.result_ui32;
}

#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


#endif /* #if (SYS_CPNT_AAA == TRUE) */

