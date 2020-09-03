/*-----------------------------------------------------------------------------
 * FILE NAME: AMTR_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for AMTR MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/14     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)

/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "aaa_om.h"
#include "aaa_mgr.h"
#include "aaa_pmgr.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */

/* MACRO FUNCTION DECLARATIONS
 */


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
 * ROUTINE NAME : AAA_PMGR_Init
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AAA_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void AAA_PMGR_Init(void)
{
    /* get the ipc message queues for AAA MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_AUTH_PROTOCOL_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
    }

    return;
} /* End of AAA_PMGR_Init */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusGroupEntry(const AAA_RadiusGroupEntryInterface_T* entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupentry)
        + AAA_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSGROUPENTRY;

    /*assign input*/
    msg_data_p->data.setradiusgroupentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyRadiusGroupEntry(const char *name, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusgroupentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusgroupentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusgroupentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYRADIUSGROUPENTRY;

    /*assign input*/
    memcpy(msg_data_p->data.destroyradiusgroupentry.req.name,
        name,
        sizeof(msg_data_p->data.destroyradiusgroupentry.req.name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *warning = msg_data_p->data.destroyradiusgroupentry.resp.warning;
    return msg_data_p->type.result_bool;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupEntry(const AAA_TacacsPlusGroupEntryInterface_T* entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSGROUPENTRY;

    /*assign input*/
    msg_data_p->data.settacacsplusgroupentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusgroupentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusgroupentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusgroupentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYTACACSPLUSGROUPENTRY;

    /*assign input*/
    memcpy(msg_data_p->data.destroytacacsplusgroupentry.req.name,
        name,
        sizeof(msg_data_p->data.destroytacacsplusgroupentry.req.name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *warning = msg_data_p->data.destroytacacsplusgroupentry.resp.warning;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry by group_index
 * INPUT    : group_index, radius_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *            radius_index will be ignored
 *            don't allow to modify the members of "default radius group"
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSENTRY;

    /*assign input*/
    msg_data_p->data.setradiusentry.req.group_index=group_index;
    msg_data_p->data.setradiusentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *warning = msg_data_p->data.setradiusentry.resp.warning;
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntryJoinDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry join default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntryJoinDefaultRadiusGroup(UI32_T radius_server_index)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentryjoindefaultradiusgroup)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentryjoindefaultradiusgroup.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentryjoindefaultradiusgroup.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSENTRYJOINDEFAULTRADIUSGROUP;

    /*assign input*/
    msg_data_p->data.setradiusentryjoindefaultradiusgroup.req.radius_server_index=radius_server_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntryDepartDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry depart default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntryDepartDefaultRadiusGroup(UI32_T radius_server_index)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentrydepartdefaultradiusgroup)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentrydepartdefaultradiusgroup.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentrydepartdefaultradiusgroup.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSENTRYDEPARTDEFAULTRADIUSGROUP;

    /*assign input*/
    msg_data_p->data.setradiusentrydepartdefaultradiusgroup.req.radius_server_index=radius_server_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry by group_index
 * INPUT    : group_index, ip_address
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentrybyipaddress)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentrybyipaddress.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusentrybyipaddress.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSENTRYBYIPADDRESS;

    /*assign input*/
    msg_data_p->data.setradiusentrybyipaddress.req.group_index=group_index;
    msg_data_p->data.setradiusentrybyipaddress.req.ip_address=ip_address;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and entry content
 * INPUT    : group_index, entry->radius_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYRADIUSENTRY;

    /*assign input*/
    msg_data_p->data.destroyradiusentry.req.group_index=group_index;
    msg_data_p->data.destroyradiusentry.req.entry=*entry;
    msg_data_p->data.destroyradiusentry.req.warning=*warning;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyRadiusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and ip_address
 * INPUT    : group_index, ip_address
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusentrybyipaddress)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusentrybyipaddress.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyradiusentrybyipaddress.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYRADIUSENTRYBYIPADDRESS;

    /*assign input*/
    msg_data_p->data.destroyradiusentrybyipaddress.req.group_index=group_index;
    msg_data_p->data.destroyradiusentrybyipaddress.req.ip_address=ip_address;
    msg_data_p->data.destroyradiusentrybyipaddress.req.warning=*warning;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry by group_index
 * INPUT    : group_index, tacacs_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *            tacacs_index will be ignored
 *            don't allow to modify the members of "default tacacs+ group"
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSENTRY;

    /*assign input*/
    msg_data_p->data.settacacsplusentry.req.group_index=group_index;
    msg_data_p->data.settacacsplusentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *warning = msg_data_p->data.settacacsplusentry.resp.warning;
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry join default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(UI32_T tacacs_server_index)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentryjoindefaulttacacsplusgroup)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentryjoindefaulttacacsplusgroup.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentryjoindefaulttacacsplusgroup.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSENTRYJOINDEFAULTTACACSPLUSGROUP;

    /*assign input*/
    msg_data_p->data.settacacsplusentryjoindefaulttacacsplusgroup.req.tacacs_server_index=tacacs_server_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry depart default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(UI32_T tacacs_server_index)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentrydepartdefaulttacacsplusgroup)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentrydepartdefaulttacacsplusgroup.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentrydepartdefaulttacacsplusgroup.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSENTRYDEPARTDEFAULTTACACSPLUSGROUP;

    /*assign input*/
    msg_data_p->data.settacacsplusentrydepartdefaulttacacsplusgroup.req.tacacs_server_index=tacacs_server_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry by group_index
 * INPUT    : group_index, ip_address
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentrybyipaddress)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentrybyipaddress.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusentrybyipaddress.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSENTRYBYIPADDRESS;

    /*assign input*/
    msg_data_p->data.settacacsplusentrybyipaddress.req.group_index=group_index;
    msg_data_p->data.settacacsplusentrybyipaddress.req.ip_address=ip_address;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus entry by group_index and entry content
 * INPUT    : group_index, entry->tacacs_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYTACACSPLUSENTRY;

    /*assign input*/
    msg_data_p->data.destroytacacsplusentry.req.group_index=group_index;
    msg_data_p->data.destroytacacsplusentry.req.entry=*entry;
    msg_data_p->data.destroytacacsplusentry.req.warning=*warning;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus entry by group_index and ip_address
 * INPUT    : group_index, ip_address
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusentrybyipaddress)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusentrybyipaddress.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroytacacsplusentrybyipaddress.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYTACACSPLUSENTRYBYIPADDRESS;

    /*assign input*/
    msg_data_p->data.destroytacacsplusentrybyipaddress.req.group_index=group_index;
    msg_data_p->data.destroytacacsplusentrybyipaddress.req.ip_address=ip_address;
    msg_data_p->data.destroytacacsplusentrybyipaddress.req.warning=*warning;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the accounting update interval (minutes)
 * INPUT    : update_interval
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccUpdateInterval(UI32_T update_interval)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccupdateinterval)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccupdateinterval.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccupdateinterval.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCUPDATEINTERVAL;

    /*assign input*/
    msg_data_p->data.setaccupdateinterval.req.update_interval=update_interval;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the accounting update interval
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccUpdateInterval()
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccupdateinterval)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccupdateinterval.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccupdateinterval.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DISABLEACCUPDATEINTERVAL;

    /*assign input*/

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : enable dot1x accounting on specified port
 * INPUT    : ifindex, list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccDot1xEntry(const AAA_AccDot1xEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccdot1xentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccdot1xentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccdot1xentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCDOT1XENTRY;

    /*assign input*/
    msg_data_p->data.setaccdot1xentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable dot1x accounting on specified port
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccDot1xEntry(UI32_T ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccdot1xentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccdot1xentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccdot1xentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DISABLEACCDOT1XENTRY;

    /*assign input*/
    msg_data_p->data.disableaccdot1xentry.req.ifindex=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}



/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec accounting by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccExecEntry(const AAA_AccExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccexecentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccexecentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccexecentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCEXECENTRY;

    /*assign input*/
    msg_data_p->data.setaccexecentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec accounting by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccExecEntry(AAA_ExecType_T exec_type)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccexecentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccexecentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableaccexecentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DISABLEACCEXECENTRY;

    /*assign input*/
    msg_data_p->data.disableaccexecentry.req.exec_type=exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command accounting by specified privilege level
 * INPUT    : privilege, list name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore to change the configure mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccCommandEntry(const AAA_AccCommandEntry_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacccmdentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacccmdentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacccmdentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCCOMMANDENTRY;

    /*assign input*/
    memcpy(&msg_data_p->data.setacccmdentry.req.cmd_entry, entry_p, sizeof(*entry_p));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the command accounting entry on specified privilege level
 *            and EXEC type
 * INPUT    : priv_lvl
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccCommandEntry(const AAA_AccCommandEntry_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacccmdentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacccmdentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacccmdentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DISABLEACCCOMMANDENTRY;

    /*assign input*/
    memcpy(&msg_data_p->data.setacccmdentry.req.cmd_entry, entry_p, sizeof(*entry_p));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry by list_name and client_type
 * INPUT    : entry->list_name, entry->group_name, entry->client_type, entry->working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccListEntry(const AAA_AccListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacclistentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacclistentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setacclistentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCLISTENTRY;

    /*assign input*/
    msg_data_p->data.setacclistentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_DestroyAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyAccListEntry(const char *name, AAA_ClientType_T client_type, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyacclistentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyacclistentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyacclistentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYACCLISTENTRY;

    /*assign input*/
    memcpy(msg_data_p->data.destroyacclistentry.req.name,
        name,
        sizeof(msg_data_p->data.destroyacclistentry.req.name));
    msg_data_p->data.destroyacclistentry.req.client_type=client_type;
    msg_data_p->data.destroyacclistentry.req.warning=*warning;


    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_DestroyAccListEntry2
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyAccListEntry2(AAA_AccListEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyacclistentry2)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyacclistentry2.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyacclistentry2.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYACCLISTENTRY2;

    /*assign input*/
    memcpy(&msg_data_p->data.destroyacclistentry2.req.entry,
        entry,
        sizeof(*entry));
    msg_data_p->data.destroyacclistentry2.req.warning=*warning;


    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0'), working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetDefaultList(const AAA_AccListEntryInterface_T *entry_p)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setdefaultlist)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setdefaultlist.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setdefaultlist.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETDEFAULTLIST;

    /*assign input*/
    memcpy(&msg_data_p->data.setdefaultlist.req.entry, entry_p, sizeof(*entry_p));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_AsyncAccountingRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : ifindex, client_type, request_type, identifier,
 *            user_name       --  User name (terminated with '\0')
 *            call_back_func  --  Callback function pointer.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_AsyncAccountingRequest(AAA_AccRequest_T *request)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.asyncaccountingrequest)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.asyncaccountingrequest.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.asyncaccountingrequest.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_ASYNC_ASYNCACCOUNTINGREQUEST;

    /*assign input*/
    msg_data_p->data.asyncaccountingrequest.req.request=*request;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_NOWAIT,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
}


#endif

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the moethod_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
 *              method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodName(UI16_T method_index, char* method_name)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodname)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodname.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodname.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETMETHODNAME;

    /*assign input*/
    msg_data_p->data.setmethodname.req.method_index=method_index;
    memcpy(msg_data_p->data.setmethodname.req.method_name,
        method_name,
        sizeof(msg_data_p->data.setmethodname.req.method_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the group_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodGroupName(UI16_T method_index, char* method_group_name)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodgroupname)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodgroupname.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodgroupname.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETMETHODGROUPNAME;

    /*assign input*/
    msg_data_p->data.setmethodgroupname.req.method_index=method_index;
    memcpy(msg_data_p->data.setmethodgroupname.req.method_group_name,
        method_group_name,
        sizeof(msg_data_p->data.setmethodgroupname.req.method_group_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the client type by list_index.
 * INPUT    : method_index, client_type
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetMethodClientType(UI16_T method_index, AAA_ClientType_T client_type)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodclienttype)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodclienttype.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodclienttype.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETMETHODCLIENTTYPE;

    /*assign input*/
    msg_data_p->data.setmethodclienttype.req.method_index=method_index;
    msg_data_p->data.setmethodclienttype.req.client_type=client_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the mode into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_mode    -- VAL_aaaMethodMode_start_stop
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodMode(UI16_T method_index, UI8_T method_mode)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodmode)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodmode.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodmode.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETMETHODMODE;

    /*assign input*/
    msg_data_p->data.setmethodmode.req.method_index=method_index;
    msg_data_p->data.setmethodmode.req.method_mode=method_mode;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the status into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_status  -- Set to 1 to initiate the aaaMethodTable, 2 to destroy the table.
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodStatus(UI16_T method_index, UI8_T method_status)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodstatus)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodstatus.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setmethodstatus.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETMETHODSTATUS;

    /*assign input*/
    msg_data_p->data.setmethodstatus.req.method_index=method_index;
    msg_data_p->data.setmethodstatus.req.method_status=method_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}


#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetRadiusGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
                radius_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetRadiusGroupName(UI16_T radius_group_index, char* radius_group_name)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupname)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupname.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupname.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSGROUPNAME;

    /*assign input*/
    msg_data_p->data.setradiusgroupname.req.radius_group_index=radius_group_index;
    memcpy(msg_data_p->data.setradiusgroupname.req.radius_group_name,
        radius_group_name,
        sizeof(msg_data_p->data.setradiusgroupname.req.radius_group_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetRadiusGroupServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
                radius_group_server_bitmap
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetRadiusGroupServerBitMap(UI16_T radius_group_index, UI8_T radius_group_server_bitmap)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgrpsvrbmp)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgrpsvrbmp.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgrpsvrbmp.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSSERVERGROUPBITMAP;

    /*assign input*/
    msg_data_p->data.setradiusgrpsvrbmp.req.radius_grp_idx=radius_group_index;
    msg_data_p->data.setradiusgrpsvrbmp.req.radius_grp_svr_bmp=radius_group_server_bitmap;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetRadiusGroupStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the group status
 * INPUT    :   radius_group_index
                radius_group_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetRadiusGroupStatus(UI16_T radius_group_index, UI32_T radius_group_status)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupstatus)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupstatus.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setradiusgroupstatus.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETRADIUSGROUPSTATUS;

    /*assign input*/
    msg_data_p->data.setradiusgroupstatus.req.radius_group_index=radius_group_index;
    msg_data_p->data.setradiusgroupstatus.req.radius_group_status=radius_group_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetTacacsPlusGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupName(UI16_T tacacsplus_group_index, char* tacacsplus_group_name)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupname)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupname.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupname.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSGROUPNAME;

    /*assign input*/
    msg_data_p->data.settacacsplusgroupname.req.tacacsplus_group_index=tacacsplus_group_index;
    memcpy(msg_data_p->data.settacacsplusgroupname.req.tacacsplus_group_name,
        tacacsplus_group_name,
        sizeof(msg_data_p->data.settacacsplusgroupname.req.tacacsplus_group_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetTacacsPlusGroupServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_TacacsPlusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_server_bitmap
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupServerBitMap(UI16_T tacacsplus_group_index, UI8_T tacacsplus_group_server_bitmap)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupserverbitmap)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupserverbitmap.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupserverbitmap.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSGROUPSERVERBITMAP;

    /*assign input*/
    msg_data_p->data.settacacsplusgroupserverbitmap.req.tacacsplus_group_index=tacacsplus_group_index;
    msg_data_p->data.settacacsplusgroupserverbitmap.req.tacacsplus_group_server_bitmap=tacacsplus_group_server_bitmap;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetTacacsPlusGroupStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the group status of TacacsPlusGroupTable into AAA_PMGR_SetTacacsPlusGroupStatus
 *              by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupStatus(UI16_T tacacsplus_group_index, UI32_T tacacsplus_group_status)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupstatus)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupstatus.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.settacacsplusgroupstatus.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETTACACSPLUSGROUPSTATUS;

    /*assign input*/
    msg_data_p->data.settacacsplusgroupstatus.req.tacacsplus_group_index=tacacsplus_group_index;
    msg_data_p->data.settacacsplusgroupstatus.req.tacacsplus_group_status=tacacsplus_group_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetUpdate
 * ------------------------------------------------------------------------
 * PURPOSE  :   Setting update interval
 * INPUT    :   update_interval
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetUpdate(UI32_T update_interval)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setupdate)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setupdate.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setupdate.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETUPDATE;

    /*assign input*/
    msg_data_p->data.setupdate.req.update_interval=update_interval;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_ACCOUNTING == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetAccountMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-name into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetAccountMethodName(UI32_T ifindex, char* account_method_name)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccountmethodname)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccountmethodname.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccountmethodname.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCOUNTMETHODNAME;

    /*assign input*/
    msg_data_p->data.setaccountmethodname.req.ifindex=ifindex;
    memcpy(msg_data_p->data.setaccountmethodname.req.account_method_name,
        account_method_name,
        sizeof(msg_data_p->data.setaccountmethodname.req.account_method_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetAccountStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-status into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetAccountStatus(UI32_T ifindex, UI8_T account_status)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccountstatus)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccountstatus.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setaccountstatus.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETACCOUNTSTATUS;

    /*assign input*/
    msg_data_p->data.setaccountstatus.req.ifindex=ifindex;
    msg_data_p->data.setaccountstatus.req.account_status=account_status;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQty(UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqty)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqty.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqty.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETACCUSERENTRYQTY;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *qty=msg_data_p->data.getaccuserentryqty.resp.qty;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbynameandtype)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbynameandtype.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbynameandtype.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAMEANDTYPE;

    /*assign input*/
    memcpy(msg_data_p->data.getaccuserentryqtyfilterbynameandtype.req.name,
        name,
        sizeof(msg_data_p->data.getaccuserentryqtyfilterbynameandtype.req.name));
    msg_data_p->data.getaccuserentryqtyfilterbynameandtype.req.client_type=client_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *qty=msg_data_p->data.getaccuserentryqtyfilterbynameandtype.resp.qty;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbytype)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbytype.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbytype.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYTYPE;

    /*assign input*/
    msg_data_p->data.getaccuserentryqtyfilterbytype.req.client_type=client_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *qty=msg_data_p->data.getaccuserentryqtyfilterbytype.resp.qty;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQtyFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbyport)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbyport.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getaccuserentryqtyfilterbyport.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYPORT;

    /*assign input*/
    msg_data_p->data.getaccuserentryqtyfilterbyport.req.ifindex=ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *qty=msg_data_p->data.getaccuserentryqtyfilterbyport.resp.qty;

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user entry by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntry(AAA_AccUserInfoInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETNEXTACCUSERENTRY;

    /*assign input*/
    msg_data_p->data.getnextaccuserentry.req.user_index=entry->user_index;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextaccuserentry.resp.entry;
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntryFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name and client_type by index.
 * INPUT    : entry->user_index, entry->user_name, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntryFilterByNameAndType(AAA_AccUserInfoInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbynameandtype)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbynameandtype.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbynameandtype.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYNAMEANDTYPE;

    /*assign input*/
    msg_data_p->data.getnextaccuserentryfilterbynameandtype.req.user_index=entry->user_index;
    msg_data_p->data.getnextaccuserentryfilterbynameandtype.req.ifindex=entry->ifindex;
    msg_data_p->data.getnextaccuserentryfilterbynameandtype.req.client_type=entry->client_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextaccuserentryfilterbynameandtype.resp.entry;
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntryFilterByType(AAA_AccUserInfoInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbytype)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbytype.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbytype.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYTYPE;

    /*assign input*/
    msg_data_p->data.getnextaccuserentryfilterbytype.req.user_index=entry->user_index;
    msg_data_p->data.getnextaccuserentryfilterbytype.req.client_type=entry->client_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }
    *entry=msg_data_p->data.getnextaccuserentryfilterbytype.resp.entry;
    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntryFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user from specified port by index.
 * INPUT    : entry->user_index, entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntryFilterByPort(AAA_AccUserInfoInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbyport)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbyport.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.getnextaccuserentryfilterbyport.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYPORT;

    /*assign input*/
    msg_data_p->data.getnextaccuserentryfilterbyport.req.user_index=entry->user_index;
    msg_data_p->data.getnextaccuserentryfilterbyport.req.ifindex=entry->ifindex;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *entry=msg_data_p->data.getnextaccuserentryfilterbyport.resp.entry;
    return msg_data_p->type.result_bool;
}


#endif

#if (SYS_CPNT_AUTHORIZATION == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_AuthorRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : request to do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_AuthorRequest(AAA_AuthorRequest_T *request,AAA_AuthorReply_T *reply)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.authorrequest)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.authorrequest.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.authorrequest.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_AUTHORREQUEST;

    /*assign input*/
    msg_data_p->data.authorrequest.req.request=*request;
    memset(&msg_data_p->data.authorrequest.req.reply, 0, sizeof(AAA_AuthorReply_T));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

   memcpy(reply, &msg_data_p->data.authorrequest.resp.reply, sizeof(AAA_AuthorReply_T));

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAuthorExecEntry(const AAA_AuthorExecEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthorexecentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthorexecentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthorexecentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETAUTHOREXECENTRY;

    /*assign input*/
    msg_data_p->data.setauthorexecentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAuthorExecEntry(AAA_ExecType_T exec_type)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableauthorexecentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableauthorexecentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disableauthorexecentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DISABLEAUTHOREXECENTRY;

    /*assign input*/
    msg_data_p->data.disableauthorexecentry.req.exec_type=exec_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0')
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetAuthorDefaultList(const AAA_ListType_T *list_type, const char *group_name)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthordefaultlist)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthordefaultlist.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthordefaultlist.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETAUTHORDEFAULTLIST;

    /*assign input*/
    msg_data_p->data.setauthordefaultlist.req.list_type = *list_type;
    memcpy(msg_data_p->data.setauthordefaultlist.req.group_name,
        group_name,
        sizeof(msg_data_p->data.setauthordefaultlist.req.group_name));

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry by list_name and client_type
 * INPUT    : entry->list_name, entry->group_name, entry->client_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAuthorListEntry(const AAA_AuthorListEntryInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthorlistentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthorlistentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.setauthorlistentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SETAUTHORLISTENTRY;

    /*assign input*/
    msg_data_p->data.setauthorlistentry.req.entry=*entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_DestroyAuthorListEntry(const char *list_name, const AAA_ListType_T *list_type, AAA_WarningInfo_T *warning)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyauthorlistentry)
        + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyauthorlistentry.req)
        + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size=sizeof(((AAA_MGR_IPCMsg_T *)0)->data.destroyauthorlistentry.resp)
        + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DESTROYAUTHORLISTENTRY;

    /*assign input*/
    memcpy(msg_data_p->data.destroyauthorlistentry.req.list_name,
        list_name,
        sizeof(msg_data_p->data.destroyauthorlistentry.req.list_name));
    msg_data_p->data.destroyauthorlistentry.req.list_type = *list_type;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *warning = msg_data_p->data.destroyauthorlistentry.resp.warning;
    return msg_data_p->type.result_bool;
}

#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetAuthorCommandEntry(const AAA_AuthorCommandEntry_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.set_author_command_entry)
                               + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size = sizeof(((AAA_MGR_IPCMsg_T *)0)->data.set_author_command_entry.req)
                + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size = sizeof(((AAA_MGR_IPCMsg_T *)0)->data.set_author_command_entry.resp)
                + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_SET_AUTHOR_COMMAND_ENTRY;

    /*assign input*/
    msg_data_p->data.set_author_command_entry.req.entry = *entry;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable command authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_DisableAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type)
{
    const UI32_T msg_buf_size=(sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disable_author_command_entry)
                               + AAA_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    AAA_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size,resp_size;

    /*assign size*/
    req_size = sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disable_author_command_entry.req)
                + AAA_MGR_MSGBUF_TYPE_SIZE;
    resp_size = sizeof(((AAA_MGR_IPCMsg_T *)0)->data.disable_author_command_entry.resp)
                + AAA_MGR_MSGBUF_TYPE_SIZE;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_AAA;
    msg_p->msg_size = req_size;

    msg_data_p=(AAA_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = AAA_MGR_IPCCMD_DISABLE_AUTHOR_COMMAND_ENTRY;

    /*assign input*/
    msg_data_p->data.disable_author_command_entry.req.exec_type = exec_type;
    msg_data_p->data.disable_author_command_entry.req.priv_lvl = priv_lvl;

    /*send ipc*/
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


#endif /* #if (SYS_CPNT_AAA == TRUE) */


