/* MODULE NAME: cli_om_exp.h
 * PURPOSE:
 *  This file is a demonstration code for implementations of CLI om in shared memory.
 *
 * NOTES:
 *  None.
 *
 * HISTORY:
 *    mm/dd/yy (A.D.)
 *    05/20/15    -- Emma, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef CLI_OM_EXP_H
#define CLI_OM_EXP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "cli_type.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifndef _countof
 #define _countof(_Array) (sizeof(_Array)/sizeof(*_Array))
#endif

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
#define CLI_OM_EXP_ADDRUNCONFIG_MAX_ENTRY               1
#define CLI_OM_EXP_ADDRUNCONFIG_EXPIRE_TIME_BY_TICKS    (5 * 60 * SYS_BLD_TICKS_PER_SECOND)
#define CLI_OM_EXP_ADDRUNCONFIG_COMMANDS_MAX_LEN        SYS_TYPE_1K_BYTES

#define CLI_OM_EXP_ADDRUNCONFIG_EVENT_NONE                        0
#define CLI_OM_EXP_ADDRUNCONFIG_EVENT_EXECUTE_PARTIAL_PROVISION   (1 << 0)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    CLI_OM_EXP_ADDRUNCONFIG_ERROR,
    CLI_OM_EXP_ADDRUNCONFIG_SUCCESS,
    CLI_OM_EXP_ADDRUNCONFIG_INVALID_PARAMETER,
    CLI_OM_EXP_ADDRUNCONFIG_NO_MORE_EMPTY_ENTRY,
    CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND,
    CLI_OM_EXP_ADDRUNCONFIG_NOT_ALLOW
} CLI_OM_EXP_ADDRUNCONFIG_RETURN_T;

typedef enum
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_INVALID,
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_PENDING,
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING,
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_DONE
} CLI_OM_EXP_ADDRUNCONFIG_ENTRY_STATUS_T;

typedef struct
{
    UI32_T      id;
    char        commands[CLI_OM_EXP_ADDRUNCONFIG_COMMANDS_MAX_LEN + 1];
    UI32_T      finish_time_by_ticks;
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_STATUS_T    status;
} CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_InitiateSystemResources
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize share memory ID and OM
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_InitiateSystemResources(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_InitDb
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_InitDb(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_AttachSystemResources
 * ------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for OM in the context of the calling process
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_AttachSystemResources(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_GetShMemInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the CLI size for share memory
 * INPUT    : None
 * OUTPUT   : segid_p - shared memory segment id to be recorded by SYSRSC_MGR
 *            seglen_p - length of the shared memroy segment to be recorded by SYSRSC_MGR
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_InitOmSem
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize CLI semaphore ID
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_InitOmSem(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_GetIsReproduceDone
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get the cli reproduce config file done
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_OM_EXP_GetIsReproduceDone(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_SetIsReproduceDone
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to set the cli reproduce config file done
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_OM_EXP_SetIsReproduceDone(BOOL_T state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_GetWorkingFlag
 * ------------------------------------------------------------------------
 * PURPOSE  : Get working flag
 * INPUT    : None
 * OUTPUT   : flag_p -- flag
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_OM_EXP_GetWorkingFlag(CLI_TYPE_WorkingFlag_T *flag_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_SetStartupFactoryDefaultFlag
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to set startup factory default.
 * INPUT    : is_default -- enabled or disabled
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_SetStartupFactoryDefaultFlag(BOOL_T is_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_GetStartupFactoryDefaultFlag
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to get startup factory default.
 * INPUT    : None
 * OUTPUT   : is_default_p -- enabled or disabled
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_GetStartupFactoryDefaultFlag(BOOL_T *is_default_p);

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_CreateEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to create addrunconfig entry,
 *            if there is no empty entry, delete done expired entry
 * INPUT    : commands -- commands for updating running-config to device
 * OUTPUT   : id       -- entry id
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_NO_MORE_EMPTY_ENTRY
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_CreateEntry(const char *commands, UI32_T *id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_GetEntryById
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get addrunconfig entry by id
 * INPUT    : id    -- entry id
 * OUTPUT   : entry -- whole entry of the specified id
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_GetEntryById(UI32_T id, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_DeleteEntryById
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
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_DeleteEntryById(UI32_T id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_GetPendingEntryAndSetToRunning
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get pending entry and set its status to running
 * INPUT    : None
 * OUTPUT   : entry -- the pending entry
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : CLI addrunconfig task will call this API to get the pending entry
 *            to do partial provision
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_GetPendingEntryAndSetToRunning(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_HasRunningEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to check if has running entry
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_OM_EXP_ADDRUNCONFIG_HasRunningEntry();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_Done
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to update the status and finish time
 * INPUT    : finish_time_by_ticks -- finish time by systicks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_ADDRUNCONFIG_Done(UI32_T finish_time_by_ticks);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_SetTaskID
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to save CLI addrunconfig task ID
 * INPUT    : tid -- CLI addrunconfig task ID
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_ADDRUNCONFIG_SetTaskID(UI32_T tid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_SetTaskID
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get CLI addrunconfig task ID
 * INPUT    : None
 * OUTPUT   : tid -- CLI addrunconfig task ID
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_OM_EXP_ADDRUNCONFIG_GetTaskID(UI32_T *tid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_GetNextEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get next entry
 * INPUT    : index -- entry index
 * OUTPUT   : entry -- entry
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_ERROR
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_GetNextEntry(UI32_T *index, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);

#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

#endif /* CLI_OM_EXP_H */
