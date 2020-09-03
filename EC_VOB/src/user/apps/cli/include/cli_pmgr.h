/* MODULE NAME:  cli_pmgr.h
 * PURPOSE:
 *    PMGR for cli
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    06/02/2007 - rich Lee, Created for Linux platform.
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef CLI_PMGR_H
#define CLI_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
#include "cli_om_exp.h"
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
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
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T CLI_PMGR_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PMGR_AllCscsKnowProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call CLI_MGR_AllCscsKnowProvisionComplete through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    *state --  current ssh status.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void CLI_PMGR_AllCscsKnowProvisionComplete(void);

void CLI_PMGR_Notify_EnterTransitionMode(void *prov_buf);

UI32_T CLI_PMGR_Get_RunningCfg(void *buffer, UI32_T buffer_size);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_IncreaseRemoteSession
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will increase counter of remote seesion
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : BOOL_T : TRUE  -- success.
 *                     FLASE -- fail.
 * NOTE     : Maximux of total session is 4 include telnet and ssh
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_PMGR_IncreaseRemoteSession(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_DecreaseRemoteSession
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will decrease counter of remote seesion
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : BOOL_T : TRUE  -- success.
 *                     FLASE -- fail.
 * NOTE     : Maximux of total session is 4 include telnet and ssh
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_PMGR_DecreaseRemoteSession(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_SetKillWorkingSpaceFlag
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the kill_itself flag to TRUE for
 *              specified sessid.
 * INPUT    : sessid
 * OUTPUT   : None
 * RETURN   : BOOL_T : TRUE /FALSE
 * NOTE     : Maximux of total session is 4 include telnet and ssh
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_PMGR_SetKillWorkingSpaceFlag(UI32_T sessid);

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_HandleChangedIpMgmtFilter
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will handle changed IP mgmt filter
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_PMGR_HandleChangedIpMgmtFilter();
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_PartialProvision
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to do partial provision.
 * INPUT    : prov_buf  -- provision buffer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void
CLI_PMGR_PartialProvision(
    void *prov_buf
);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_ADDRUNCONFIG_CreateEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to create addrunconfig entry
 * INPUT    : commands -- commands for updating running-config on the device
 * OUTPUT   : id       -- entry id
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_ERROR
 *            CLI_OM_EXP_ADDRUNCONFIG_NO_MORE_EMPTY_ENTRY
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : If no empty entry, delete entry: (done + expired)
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_PMGR_ADDRUNCONFIG_CreateEntry(const char *commands, UI32_T *id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_ADDRUNCONFIG_DeleteEntryById
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to delete addrunconfig entry by id
 * INPUT    : id -- entry id
 * OUTPUT   : None
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND
 *            CLI_OM_EXP_ADDRUNCONFIG_NOT_ALLOW
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : Not allow to delete the running entry
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_PMGR_ADDRUNCONFIG_DeleteEntryById(UI32_T id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_ADDRUNCONFIG_GetEntryById
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get addrunconfig entry by id,
 *            and if its status is done, recover it
 * INPUT    : id    -- entry id
 * OUTPUT   : entry -- whole entry of the specified id
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_ERROR
 *            CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_PMGR_ADDRUNCONFIG_GetEntryById(UI32_T id, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);
#endif  /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - CLI_PMGR_Get_Max_Command_Privilege
 * -------------------------------------------------------------------------
 * FUNCTION: Find max privilege of input command tree
 * INPUT   : UI16_T        cmd_idx : input command index
 *           UI8_T         privilege_mode : command mode
 * RETURN  : max_privilege
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI16_T CLI_PMGR_Get_Max_Command_Privilege(
    UI16_T  cmd_idx,
    UI8_T  privilege_mode
);

#endif    /* End of CSCA_PMGR_H */

