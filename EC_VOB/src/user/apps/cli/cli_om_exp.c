/* MODULE NAME: cli_om_exp.c
 * PURPOSE:
 *  Implementation for the CLI object manager in shared memory
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

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include "cli_om_exp.h"
#include "string.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sys_module.h"

#ifndef ASSERT
#define ASSERT(eq)
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    CLI_TYPE_WorkingFlag_T   working_flag;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T cli_addrunconfig_table[CLI_OM_EXP_ADDRUNCONFIG_MAX_ENTRY];
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */
} CLI_OM_EXP_Database_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_FindEntryById
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get entry by id
 * INPUT    : id    -- entry id
 * OUTPUT   : None
 * RETURN   : entry -- the entry pointer
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *CLI_OM_EXP_ADDRUNCONFIG_FindEntryById(UI32_T id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to find entry by status
 * INPUT    : status    -- entry status
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_INVALID,
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_PENDING,
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING,
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_DONE
 * OUTPUT   : None
 * RETURN   : entry -- the entry pointer
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_STATUS_T status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_InitEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to initilize entry
 * INPUT    : id        -- entry id
 *            commands  -- commands for updating running-config on the device
 *            entry     -- the entry pointer
 * OUTPUT   : None
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_INVALID_PARAMETER
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_InitEntry(UI32_T id, const char *commands, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_ConstructEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to construct entry
 * INPUT    : entry -- the entry pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void CLI_OM_EXP_ADDRUNCONFIG_ConstructEntry(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_DestructEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to destruct entry
 * INPUT    : entry -- the entry pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void CLI_OM_EXP_ADDRUNCONFIG_DestructEntry(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry);
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */


/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T   cli_om_exp_semaphore_id;
static UI32_T   cli_om_exp_orig_priority;
static CLI_OM_EXP_Database_T *cli_om_exp_db;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
static UI32_T cli_om_exp_addrunconfig_task_id;
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */


/* MACRO FUNCTIONS DECLARACTION
 */
#define CLI_OM_EXP_LOCK() \
    cli_om_exp_orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cli_om_exp_semaphore_id);
#define CLI_OM_EXP_UNLOCK() \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(cli_om_exp_semaphore_id, cli_om_exp_orig_priority);


/* EXPORTED SUBPROGRAM BODIES
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
void CLI_OM_EXP_InitiateSystemResources(void)
{
    cli_om_exp_db = (CLI_OM_EXP_Database_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_CLI_OM_SHMEM_SEGID);
    CLI_OM_EXP_InitDb();
}

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
void CLI_OM_EXP_InitDb(void)
{
    UI32_T i;

    memset(cli_om_exp_db, 0, sizeof(*cli_om_exp_db));

    for (i = 0; i < _countof(cli_om_exp_db->cli_addrunconfig_table); ++i)
    {
        CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry = &cli_om_exp_db->cli_addrunconfig_table[i];

        CLI_OM_EXP_ADDRUNCONFIG_ConstructEntry(entry);
    }
}

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
void CLI_OM_EXP_AttachSystemResources(void)
{
    cli_om_exp_db = (CLI_OM_EXP_Database_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_CLI_OM_SHMEM_SEGID);

    if (NULL == cli_om_exp_db)
    {
        SYSFUN_Debug_Printf("\r\n CLI OM EXP: Get shared memory failed.");
    }

    if (SYSFUN_OK != SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_CLI_OM, &cli_om_exp_semaphore_id))
    {
        SYSFUN_Debug_Printf("\r\n CLI OM EXP: Get semaphore failed.");
    }
}

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
void CLI_OM_EXP_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_CLI_OM_SHMEM_SEGID;
    *seglen_p = sizeof(CLI_OM_EXP_Database_T);
}

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
void CLI_OM_EXP_InitOmSem(void)
{
    /* Semaphore for critical section, mgr and om use same sem id
     */
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_CLI_OM, &cli_om_exp_semaphore_id)!=SYSFUN_OK)
        SYSFUN_Debug_Printf("%s:get om sem id fail.\n", __FUNCTION__);
}

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
BOOL_T CLI_OM_EXP_GetIsReproduceDone(void)
{
    BOOL_T ret;

    CLI_OM_EXP_LOCK();

    ret = (1 == cli_om_exp_db->working_flag.reproduce_done) ? TRUE : FALSE;

    CLI_OM_EXP_UNLOCK();

    return ret;
}

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
BOOL_T CLI_OM_EXP_SetIsReproduceDone(BOOL_T state)
{
    CLI_OM_EXP_LOCK();

    cli_om_exp_db->working_flag.reproduce_done = (TRUE == state) ? 1 : 0;

    CLI_OM_EXP_UNLOCK();

    return TRUE;
}

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
BOOL_T CLI_OM_EXP_GetWorkingFlag(CLI_TYPE_WorkingFlag_T *flag_p)
{
    CLI_OM_EXP_LOCK();

    memcpy(flag_p, &cli_om_exp_db->working_flag, sizeof(*flag_p));

    CLI_OM_EXP_UNLOCK();
    return TRUE;
}

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
void CLI_OM_EXP_SetStartupFactoryDefaultFlag(BOOL_T is_default)
{
    CLI_OM_EXP_LOCK();

    cli_om_exp_db->working_flag.startup_by_factory_default =
        ((TRUE == is_default) ? 1 : 0);

    CLI_OM_EXP_UNLOCK();
}

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
void CLI_OM_EXP_GetStartupFactoryDefaultFlag(BOOL_T *is_default_p)
{
    CLI_OM_EXP_LOCK();

    *is_default_p = (1 == cli_om_exp_db->working_flag.startup_by_factory_default) ?
                                                                   TRUE : FALSE;
    CLI_OM_EXP_UNLOCK();
}

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_CreateEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to create addrunconfig entry
 * INPUT    : commands -- commands for updating running-config to device
 * OUTPUT   : id       -- entry id
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_ERROR
 *            CLI_OM_EXP_ADDRUNCONFIG_NO_MORE_EMPTY_ENTRY
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : If no empty entry, recover entry: (done + expired)
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_CreateEntry(const char *commands, UI32_T *id)
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *new_entry;
    CLI_OM_EXP_ADDRUNCONFIG_RETURN_T ret;

    if (commands == NULL || id == NULL)
    {
        return CLI_OM_EXP_ADDRUNCONFIG_ERROR;
    }

    CLI_OM_EXP_LOCK();

    new_entry = CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_INVALID);

    if (new_entry == NULL)
    {
        CLI_OM_EXP_UNLOCK();
        return CLI_OM_EXP_ADDRUNCONFIG_NO_MORE_EMPTY_ENTRY;
    }

    CLI_OM_EXP_ADDRUNCONFIG_ConstructEntry(new_entry);
    ret = CLI_OM_EXP_ADDRUNCONFIG_InitEntry(*id, commands, new_entry);

    if (ret != CLI_OM_EXP_ADDRUNCONFIG_SUCCESS)
    {
        CLI_OM_EXP_UNLOCK();
        return ret;
    }

    CLI_OM_EXP_UNLOCK();

    return CLI_OM_EXP_ADDRUNCONFIG_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_GetEntryById
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get addrunconfig entry by id
 * INPUT    : id    -- entry id
 * OUTPUT   : entry -- whole entry of the specified id
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_ERROR
 *            CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_GetEntryById(UI32_T id, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry)
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *find_entry;

    if (entry == NULL)
    {
        return CLI_OM_EXP_ADDRUNCONFIG_ERROR;
    }

    CLI_OM_EXP_LOCK();

    find_entry = CLI_OM_EXP_ADDRUNCONFIG_FindEntryById(id);

    if (find_entry == NULL)
    {
        CLI_OM_EXP_UNLOCK();
        return CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND;
    }

    memcpy(entry, find_entry, sizeof(*entry));

    CLI_OM_EXP_UNLOCK();

    return CLI_OM_EXP_ADDRUNCONFIG_SUCCESS;
}

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
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_DeleteEntryById(UI32_T id)
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *find_entry;

    CLI_OM_EXP_LOCK();

    find_entry = CLI_OM_EXP_ADDRUNCONFIG_FindEntryById(id);

    if (find_entry == NULL)
    {
        CLI_OM_EXP_UNLOCK();
        return CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND;
    }

    CLI_OM_EXP_ADDRUNCONFIG_DestructEntry(find_entry);

    CLI_OM_EXP_UNLOCK();

    return CLI_OM_EXP_ADDRUNCONFIG_SUCCESS;
}

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
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_GetPendingEntryAndSetToRunning(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry)
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *find_entry;

    CLI_OM_EXP_LOCK();

    find_entry = CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_PENDING);

    if (find_entry == NULL)
    {
        CLI_OM_EXP_UNLOCK();
        return CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND;
    }

    find_entry->status = CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING;

    memcpy(entry, find_entry, sizeof(*entry));

    CLI_OM_EXP_UNLOCK();

    return CLI_OM_EXP_ADDRUNCONFIG_SUCCESS;
}

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
BOOL_T CLI_OM_EXP_ADDRUNCONFIG_HasRunningEntry()
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *find_entry;
    BOOL_T ret;

    CLI_OM_EXP_LOCK();

    find_entry =  CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING);

    if (find_entry == NULL)
    {
        ret = FALSE;
    }
    else
    {
        ret = TRUE;
    }

    CLI_OM_EXP_UNLOCK();

    return ret;
}

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
void CLI_OM_EXP_ADDRUNCONFIG_Done(UI32_T finish_time_by_ticks)
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *find_entry;

    CLI_OM_EXP_LOCK();

    find_entry = CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING);
    ASSERT(find_entry != NULL);

    find_entry->status = CLI_OM_EXP_ADDRUNCONFIG_ENTRY_DONE;
    find_entry->finish_time_by_ticks = finish_time_by_ticks;

    CLI_OM_EXP_UNLOCK();
}

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
void CLI_OM_EXP_ADDRUNCONFIG_SetTaskID(UI32_T tid)
{
    CLI_OM_EXP_LOCK();

    cli_om_exp_addrunconfig_task_id = tid;

    CLI_OM_EXP_UNLOCK();
}

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
void CLI_OM_EXP_ADDRUNCONFIG_GetTaskID(UI32_T *tid)
{
    CLI_OM_EXP_LOCK();

    *tid = cli_om_exp_addrunconfig_task_id;

    CLI_OM_EXP_UNLOCK();
}

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
CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_GetNextEntry(UI32_T *index, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry)
{
    CLI_OM_EXP_LOCK();

    if ((entry == NULL) ||
        (_countof(cli_om_exp_db->cli_addrunconfig_table) <= *index))
    {
        CLI_OM_EXP_UNLOCK();
        return CLI_OM_EXP_ADDRUNCONFIG_ERROR;
    }

    memcpy(entry, &cli_om_exp_db->cli_addrunconfig_table[*index], sizeof(*entry));

    CLI_OM_EXP_UNLOCK();

    (*index)++;

    return CLI_OM_EXP_ADDRUNCONFIG_SUCCESS;
}


/* LOCAL SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_FindEntryById
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to get entry by id
 * INPUT    : id    -- entry id
 * OUTPUT   : None
 * RETURN   : entry -- the entry pointer
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *CLI_OM_EXP_ADDRUNCONFIG_FindEntryById(UI32_T id)
{
    UI32_T i;

    for (i = 0; i < _countof(cli_om_exp_db->cli_addrunconfig_table); ++i)
    {
        CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry = &cli_om_exp_db->cli_addrunconfig_table[i];

        if (entry->id == id)
        {
            return entry;
        }
    }

    return NULL;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to find entry by status
 * INPUT    : status    -- entry status
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_INVALID,
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_PENDING,
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING,
 *                          CLI_OM_EXP_ADDRUNCONFIG_ENTRY_DONE
 * OUTPUT   : None
 * RETURN   : entry -- the entry pointer
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *CLI_OM_EXP_ADDRUNCONFIG_FindEntryByStatus(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_STATUS_T status)
{
    UI32_T i;

    for (i = 0; i < _countof(cli_om_exp_db->cli_addrunconfig_table); ++i)
    {
        CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry = &cli_om_exp_db->cli_addrunconfig_table[i];

        if (entry->status == status)
        {
            return entry;
        }
    }

    return NULL;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_InitEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to initilize entry
 * INPUT    : id        -- entry id
 *            commands  -- commands for updating running-config on the device
 *            entry     -- the entry pointer
 * OUTPUT   : None
 * RETURN   : CLI_OM_EXP_ADDRUNCONFIG_INVALID_PARAMETER
 *            CLI_OM_EXP_ADDRUNCONFIG_SUCCESS
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static CLI_OM_EXP_ADDRUNCONFIG_RETURN_T CLI_OM_EXP_ADDRUNCONFIG_InitEntry(UI32_T id, const char *commands, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry)
{
    memcpy(entry->commands, commands, sizeof(entry->commands) - 1);

    if (entry->commands[sizeof(entry->commands) - 1] != '\0')
    {
        memset(entry->commands, 0, sizeof(entry->commands) - 1);
        return CLI_OM_EXP_ADDRUNCONFIG_INVALID_PARAMETER;
    }

    entry->id = id;
    entry->status = CLI_OM_EXP_ADDRUNCONFIG_ENTRY_PENDING;

    return CLI_OM_EXP_ADDRUNCONFIG_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_ConstructEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to construct entry
 * INPUT    : entry -- the entry pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void CLI_OM_EXP_ADDRUNCONFIG_ConstructEntry(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry)
{
    entry->id = 0;
    entry->commands[0] = '\0';
    entry->finish_time_by_ticks = 0;
    entry->status = CLI_OM_EXP_ADDRUNCONFIG_ENTRY_INVALID;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EXP_ADDRUNCONFIG_DestructEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to destruct entry
 * INPUT    : entry -- the entry pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void CLI_OM_EXP_ADDRUNCONFIG_DestructEntry(CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T *entry)
{
    entry->id = 0;
    entry->status = CLI_OM_EXP_ADDRUNCONFIG_ENTRY_INVALID;
}
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */
