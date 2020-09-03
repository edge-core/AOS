/* MODULE NAME:  stktplg_pmgr.c
 * PURPOSE:
 *    STKTPLG PMGR
 *
 * NOTES:
 *
 * HISTORY
 *    7/31/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_cpnt.h"

#include "stktplg_mgr.h"
#include "stktplg_pmgr.h"

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

/* all IPC requests for STKTPLG_ENGINE should use this msgq.
 */
static SYSFUN_MsgQ_T task_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKTPLG_PMGR_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG PMGR for the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_InitiateProcessResources(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_CSC_STKTPLG_TASK_MSGK_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &task_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    if(SYSFUN_GetMsgQ(SYS_BLD_STKTPLG_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_PMGR_GetSwitchInfo
 * PURPOSE: This function is used to get this switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(switch_info==NULL)
        return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_GET_SWITCH_INFO;
    memcpy(&(ipcmsg_p->data.switch_info), switch_info, sizeof(STKTPLG_MGR_Switch_Info_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T), sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        memcpy(switch_info, &(ipcmsg_p->data.switch_info), sizeof(STKTPLG_MGR_Switch_Info_T));
    }

    return (BOOL_T)(ipcmsg_p->type.result);
}

/* FUNCTION NAME: STKTPLG_PMGR_GetNextSwitchInfo
 * PURPOSE: This function is used to get_next switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- next switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetNextSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(switch_info==NULL)
        return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_GET_NEXT_SWITCH_INFO;
    memcpy(&(ipcmsg_p->data.switch_info), switch_info, sizeof(STKTPLG_MGR_Switch_Info_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T), sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        memcpy(switch_info, &(ipcmsg_p->data.switch_info), sizeof(STKTPLG_MGR_Switch_Info_T));
    }

    return (BOOL_T)(ipcmsg_p->type.result);

}

/* FUNCTION NAME: STKTPLG_PMGR_GetSwitchModuleInfoEntry
 * PURPOSE: This function is used to get this module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(entry==NULL)
        return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_GET_SWITCH_MODULE_INFO_ENTRY;
    memcpy(&(ipcmsg_p->data.switch_module_info_entry), entry, sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T), sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        memcpy(entry, &(ipcmsg_p->data.switch_module_info_entry), sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));
    }

    return (BOOL_T)(ipcmsg_p->type.result);


}

/* FUNCTION NAME: STKTPLG_PMGR_GetNextSwitchInfo
 * PURPOSE: This function is used to get_next module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetNextSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(entry==NULL)
        return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_GET_NEXT_SWITCH_MODULE_INFO_ENTRY;
    memcpy(&(ipcmsg_p->data.switch_module_info_entry), entry, sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T), sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        memcpy(entry, &(ipcmsg_p->data.switch_module_info_entry), sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));
    }

    return (BOOL_T)(ipcmsg_p->type.result);

}

/* FUNCTION NAME: STKTPLG_PMGR_SlaveReady
 * PURPOSE: This utiity function is to keep information about is unit is entering
 *          slave mode completely.
 * INPUT:   ready -- ready or not.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_PMGR_SlaveReady(BOOL_T ready)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(UI32_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SLAVE_READY;
    ipcmsg_p->data.one_ui32 = ready;

    SYSFUN_SendRequestMsg(task_ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE, sysfun_msg_p);
}

BOOL_T STKTPLG_PMGR_SetEntPhysicalAlias(I32_T ent_physical_index, UI8_T *ent_physical_alias)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_ALIAS;
    ipcmsg_p->data.ent_physical_entry.ent_physical_index = ent_physical_index;
    memcpy(ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_alias,ent_physical_alias,sizeof(ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_alias));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));
}

BOOL_T STKTPLG_PMGR_SetEntPhysicalAssetId(I32_T ent_physical_index, UI8_T *ent_physical_asset_id)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_ASSETID;
    ipcmsg_p->data.ent_physical_entry.ent_physical_index = ent_physical_index;
    memcpy(ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_asset_id,ent_physical_asset_id,sizeof(ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_asset_id));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));
}

BOOL_T STKTPLG_PMGR_SetEntPhysicalSeriaNum(I32_T ent_physical_index, UI8_T *ent_physical_seriaNum)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_SERIANUM;
    ipcmsg_p->data.ent_physical_entry.ent_physical_index = ent_physical_index;
    memcpy(ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_serial_num,ent_physical_seriaNum,sizeof(ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_serial_num));
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_OM_EntPhysicalEntry_T),
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));
}
#if 0
/* FUNCTION NAME: STKTPLG_PMGR_SetStackRoleLed
 * PURPOSE: This function is used to set the role which the unit is playing
 * INPUT:   stk_role
 *          	LEDDRV_STACK_ARBITRATION
 *				LEDDRV_STACK_PRE_MASTER
 *				LEDDRV_STACK_MASTER
 *				LEDDRV_STACK_SLAVE
 *				defined in leddrv.h
 * OUTPUT:  none
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:
 *
 */
BOOL_T STKTPLG_PMGR_SetStackRoleLed(UI32_T stk_role)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(UI32_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_STACK_ROLE_LED;
    ipcmsg_p->data.one_ui32 = stk_role;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));

}
#endif
/* FUNCTION NAME: STKTPLG_PMGR_SetTransitionMode
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *			Master only
 */
void STKTPLG_PMGR_SetTransitionMode(void)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE;

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_TRANSITION_MODE;

    SYSFUN_SendRequestMsg(task_ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p);
}

/* FUNCTION NAME: STKTPLG_PMGR_UnitIDReNumbering
 * PURPOSE: This function is to result STKTPLG TCN and reassigned unit ID to all units
 *          based on the stacking spec.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE   -- Renumbering work (not standalone or unit ID not equal to 1)
 *          FALSE  -- otherwise
 * NOTES:   
 *
 */
BOOL_T STKTPLG_PMGR_UnitIDReNumbering(void)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE;

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_UNIT_ID_RENUMBERING;

    if(SYSFUN_SendRequestMsg(task_ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return (BOOL_T)(ipcmsg_p->type.result);
}

void STKTPLG_PMGR_ProvisionCompleted(BOOL_T ready)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_GET_MSGBUFSIZE(UI32_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_GET_MSGBUFSIZE(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_PROVISION_COMPLETED;
    ipcmsg_p->data.one_ui32 = ready;

    SYSFUN_SendRequestMsg(task_ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p);
}

/* FUNCTION NAME: STKTPLG_PMGR_SyncModuleProcess
 * PURPOSE: To sync the module status.
 * INPUT:   unit_id --- Module of which unit.
 * OUTPUT:  *is_insertion   -- insertion or removal.
 *          *starting_port  -- starting port ID of changed module.
 *          *number_of_port -- port number of changed module.
 *          *use_default    -- use default setting if insertion. I
 *                             Ignore this argument if removal.
 * RETUEN:  TRUE  -- Need to process.
 *          FALSE -- Not necessary to process.
 * NOTES:   None.
 */ 
BOOL_T STKTPLG_PMGR_SyncModuleProcess(UI32_T unit_id, 
                                      BOOL_T *is_insertion, 
                                      UI32_T *starting_port, 
                                      UI32_T *number_of_port, 
                                      BOOL_T *use_default)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*3)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(is_insertion==NULL || starting_port==NULL ||
       number_of_port==NULL || use_default==NULL)
       return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SYNC_MODULE_PROCESS;
    ipcmsg_p->data.one_ui32 = unit_id;

    SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*3,
        sysfun_msg_p);

    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        *starting_port=ipcmsg_p->data.three_ui32[0];
        *number_of_port=ipcmsg_p->data.three_ui32[1];
        *is_insertion = (BOOL_T)(ipcmsg_p->data.three_ui32[2] >> 24);
        *use_default  = (BOOL_T)((ipcmsg_p->data.three_ui32[2] >> 16) & 0xFF);
    }

    return (BOOL_T)(ipcmsg_p->type.result);
}


/* FUNCTION NAME: STKTPLG_PMGR_GetStackingPortLinkStatusByUnitId
 * PURPOSE: This function is used to get link status of the stacking ports on a unit.
 * INPUT:   unit_id     -- which unit id.
 * OUTPUT:  up_link_status  -- phy status of stacking up port
 *          down_link_status-- phy status of stacking down port
 * RETUEN:  TRUE  -- success
 *          FALSE -- fail to get link status.
 * NOTES:   
 */
BOOL_T STKTPLG_PMGR_GetStackingPortLinkStatusByUnitId(
            UI32_T unit_id, BOOL_T *up_link_status, BOOL_T *down_link_status)
{ 
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*2)];
 
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(up_link_status==NULL || down_link_status==NULL)
        return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_GET_STACKING_PORT_LINK_STATUS;
    
    ipcmsg_p->data.one_ui32 = unit_id;
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*2, 
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        *up_link_status=(BOOL_T)ipcmsg_p->data.two_ui32[0];
        *down_link_status=(BOOL_T)ipcmsg_p->data.two_ui32[1];
    }

    return (BOOL_T)(ipcmsg_p->type.result);
}

/* FUNCTION NAME: STKTPLG_PMGR_GetStackingUserPortByUnitId
 * PURPOSE: This function is used to get the stacking ports in user port id for a unit.
 * INPUT:   unit_id     -- which unit id.
 * OUTPUT:  up_uport_p  -- user port index of stacking up port
 *          down_uport_p-- user port index of stacking down port
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure. Input parameter error ,or no stacking port on that unit.
 * NOTES:   Get board_id and find the stacking port index. 
 *          Check the port_type to tell if the unit gets stacking port on it.
 */
BOOL_T STKTPLG_PMGR_GetStackingUserPortByUnitId(UI32_T unit_id, UI32_T *up_uport_p, UI32_T *down_uport_p)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*2)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(up_uport_p==NULL || down_uport_p==NULL)
        return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_GET_STACKING_PORT_USER_PORT_INDEX;

    ipcmsg_p->data.one_ui32 = unit_id;
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*2, 
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }
    
    if((BOOL_T)(ipcmsg_p->type.result)==TRUE)
    {
        *up_uport_p=ipcmsg_p->data.two_ui32[0];
        *down_uport_p=ipcmsg_p->data.two_ui32[1];
    }

    return (BOOL_T)(ipcmsg_p->type.result);
}

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
/* FUNCTION NAME : STKTPLG_PMGR_SetMasterButtonStatus
 * PURPOSE: This function is to set the master button status setting into flash for a specified unit 
 * INPUT:   unit_id
 *          status : master button status
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_SetMasterButtonStatus(UI32_T unit_id, BOOL_T status)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*2)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*2;

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_MASTER_BUTTON_STATUS;
    ipcmsg_p->data.two_ui32[0] = unit_id;
    ipcmsg_p->data.two_ui32[1] = status;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));

}
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME : STKTPLG_PMGR_SetMasterButtonStatus
 * PURPOSE: process insert/remove a unit.
 * INPUT:   None.
 * OUTPUT:  *is_insertion   -- insertion or removal.
 *          *starting_port  -- starting port ID .
 *          *number_of_port -- port number .
 *          *use_default    -- use default setting if insertion. I
 *                             Ignore this argument if removal.
 * RETUEN:  TRUE  -- Need to process.
 *          FALSE -- Not necessary to process.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_ProcessUnitInsertRemove(BOOL_T *is_insertion, 
                                      UI32_T *starting_port, 
                                      UI32_T *number_of_port,
                                      UI8_T *unit_id,
                                      BOOL_T *use_default)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*3)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    if(is_insertion==NULL || starting_port==NULL ||
       number_of_port==NULL || use_default==NULL || unit_id ==NULL)
       return FALSE;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_PROCESS_UNIT_INSERT_REMOVE;
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE+sizeof(UI32_T)*3,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

        *starting_port=ipcmsg_p->data.three_ui32[0];
        *number_of_port=ipcmsg_p->data.three_ui32[1];
        *is_insertion = (BOOL_T)(ipcmsg_p->data.three_ui32[2] >> 24);
        *use_default  = (BOOL_T)((ipcmsg_p->data.three_ui32[2] >> 16) & 0xFF);
        *unit_id=(UI8_T)((ipcmsg_p->data.three_ui32[2] >> 8) & 0xFFFF);
    return (BOOL_T)(ipcmsg_p->type.result);
}

/* FUNCTION NAME: STKTPLG_PMGR_SetTransitionMode
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *			Master only
 */
void STKTPLG_PMGR_SetSlavePastMasterMac(void)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE;

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_SLAVE_PAST_MAC;

    SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p);
}
#endif/* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
/* FUNCTION NAME : STKTPLG_PMGR_SetStackingButtonStatus
 * PURPOSE: This function is to set the stacking button status setting into flash
 * INPUT:   status : stacking button status
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- stacking button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_SetStackingButtonStatus(BOOL_T status)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T))];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T);

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_STACKING_BUTTON_STATUS;
    ipcmsg_p->data.one_ui32 = status;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));

}
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_PMGR_SetCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set HW port mode config
 * INPUT   : unit
 *           port
 *           hw_port_mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_PMGR_SetCfgHwPortMode(
    UI32_T unit,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode)
{
    UI8_T buf[SYSFUN_SIZE_OF_MSG(STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*3)];
    SYSFUN_Msg_T* sysfun_msg_p;
    STKTPLG_MGR_IPCMsg_T* ipcmsg_p;

    sysfun_msg_p = (SYSFUN_Msg_T*)buf;
    sysfun_msg_p->cmd = SYS_MODULE_STKTPLG;
    sysfun_msg_p->msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*3;

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)sysfun_msg_p->msg_buf;
    ipcmsg_p->type.cmd = STKTPLG_MGR_IPC_CMD_SET_CFG_HW_PORT_MODE;
    ipcmsg_p->data.three_ui32[0] = unit;
    ipcmsg_p->data.three_ui32[1] = port;
    ipcmsg_p->data.three_ui32[2] = hw_port_mode;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, sysfun_msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        STKTPLG_MGR_MSGBUF_TYPE_SIZE,
        sysfun_msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return ((BOOL_T)(ipcmsg_p->type.result));
}
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */

