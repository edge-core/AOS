/*-----------------------------------------------------------------------------
 * Module Name: CFGDB_MGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE: Manage this computer software component.
 *-----------------------------------------------------------------------------
 * NOTES: 1. There are 2 kind of check sum
 *           a. Binary configuration file, i.e. File level.
 *           b. Per section, i,e, Section level.
 *           c. File level checksum is not always whole file, and must base on
 *              total length to reduce to time to calculate. ???
 *           d. total length should be reviewed by myself.
 *           e. Header always need to sync to remote unit if data changed, because
 *              checksum is changed.
 *        2. About record size and record length.
 *           a. record size change: default.
 *           b. record length extend: original + default.
 *        3. version change: default.
 *        4. How about some unit leaves from stack. ???
 *-----------------------------------------------------------------------------
 * HISTORY:
 *      03/21/2003  - Ryan,          Created
 *      04/08/2003 -- Vincent,       Modify
 *      04/30/2003 -- Charles Cheng, Take over
 *      07/17/2003 -- Charles Cheng, Change version to 0x01000002 for keep IP
 *                                   address when set to factory default.
 *-----------------------------------------------------------------------------
 *  (C) Unpublished Work of Accton Technology,  Corp.  All Rights Reserved.
 *
 *      THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
 *      PROPRIETARY AND TRADESECRET INFORMATION OF ACCTON TECHNOLOGY CORP.
 *      ACCESS TO THIS WORK IS RESTRICTED TO (I) ACCTON EMPLOYEES WHO HAVE A
 *      NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
 *      AND (II) ENTITIES OTHER THAN ACCTON WHO HAVE ENTERED INTO APPROPRIATE
 *      LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
 *      PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
 *      ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
 *      TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ACCTON.
 *      ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
 *      SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
 *-----------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "cfgdb_type.h"
#include "cfgdb_util.h"
#include "cfgdb_mgr.h"
#include "sysfun.h"
#include "sys_cpnt.h"
#include "fs.h"
#include "fs_type.h"
#include "isc.h"
#include "iuc.h"
#include "l_mm.h"
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"
#include "backdoor_mgr.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */
#define CFGDB_MGR_BINARY_CFG_FILE_NAME                  ".binary_cfg_file.bin"
#define CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME         ".binary_cfg_file_internal.bin"

#define CFGDB_MGR_EnterCriticalSection()   SYSFUN_TakeSem(cfgdb_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define CFGDB_MGR_LeaveCriticalSection()   SYSFUN_GiveSem(cfgdb_mgr_sem_id)

/* DATA TYPE DEFINITION
 */


/* MACRO FUNCTION DECLARACTION
 */
#define CFGDB_MGR_SECTION_START_OFFSET(SHADOW, SECTION_HANDLER)        (SHADOW->descriptor[(SECTION_HANDLER)-1].section_start_offset)
#define CFGDB_MGR_SECTION_RECORD_SIZE(SHADOW, SECTION_HANDLER)         (SHADOW->descriptor[(SECTION_HANDLER)-1].section_record_size)
#define CFGDB_MGR_SECTION_RECORD_NUMBER(SHADOW, SECTION_HANDLER)       (SHADOW->descriptor[(SECTION_HANDLER)-1].section_record_number)
#define CFGDB_MGR_SECTION_CHECKSUM(SHADOW, SECTION_HANDLER)            (SHADOW->descriptor[(SECTION_HANDLER)-1].section_checksum)

#define CFGDB_MGR_SECTION_LEN(SHADOW, SECTION_HANDLER)                 (CFGDB_MGR_SECTION_RECORD_SIZE(SHADOW, SECTION_HANDLER)*CFGDB_MGR_SECTION_RECORD_NUMBER(SHADOW, SECTION_HANDLER))
#define CFGDB_MGR_SECTION_DATA_ADDR_IN_SHADOW(SHADOW, SECTION_HANDLER) (SHADOW->section_data + CFGDB_MGR_SECTION_START_OFFSET(SHADOW, SECTION_HANDLER))

#define CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL)   \
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) == SYS_TYPE_STACKING_SLAVE_MODE ){  \
    return (RET_VAL);  \
    }

#define CFGDB_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE()    \
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) == SYS_TYPE_STACKING_SLAVE_MODE){  \
        return;   \
    }

#define CFGDB_MGR_RETURN_AND_RELEASE_CSC(RET_VAL)   {  \
        return (RET_VAL);  \
    }

#define CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE()  {  \
        return;  \
    }

#define CFGDB_MGR_MAX_SHUTDOWN_WAITING_TIME                 SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS



#define CFGDB_MGR_DBGMSG0(LEVEL, MSG)                           \
  if (shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level>=(LEVEL))  {     \
      printf(MSG);                                              \
  }

#define CFGDB_MGR_DBGMSG1(LEVEL, MSG, ARG0)                     \
  if (shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level>=(LEVEL))  {     \
      printf(MSG, ARG0);                                        \
  }

#define CFGDB_MGR_DBGMSG2(LEVEL, MSG, ARG0, ARG1)               \
  if (shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level>=(LEVEL))  {     \
      printf(MSG, ARG0, ARG1);                                  \
  }

#define CFGDB_MGR_DBGMSG3(LEVEL, MSG, ARG0, ARG1, ARG2)         \
  if (shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level>=(LEVEL))  {     \
      printf(MSG, ARG0, ARG1, ARG2);                            \
  }


/* LOCAL SUBPROGRAM SPECIFICATION
 */
static void CFGDB_MGR_BackdoorMain(void);
static void CFGDB_MGR_DumpSectionAttributes(void);

static CFGDB_MGR_Shmem_Data_T   *shmem_data_p;
static UI32_T                   cfgdb_mgr_sem_id;

/* EXPORTED FUNCTIONS BODY
 */
/*----------------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_Initiate_System_Resources
 *----------------------------------------------------------------------------------
 * Purpose: Initiate this package
 * Input:   None.
 * Output:  None.
 * Return:  None.
 * Notes:   This function should be called when system is initialized.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_Initiate_System_Resources (void)
{
    shmem_data_p = (CFGDB_MGR_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_CFGDB_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

/*
    if (NULL == (config_file_shadow_p = (CFGDB_TYPE_ConfigFile_T *)calloc(SYS_ADPT_BINARY_CFG_FILE_SIZE, sizeof(UI8_T))))
    {
        printf("\r\nCFGDB: Allocating config file shadow memory failed.\r\n");
        assert(0);
    }

    if (NULL == (internal_config_file_shadow_p = (CFGDB_TYPE_ConfigFile_T *)calloc(SYS_ADPT_BINARY_CFG_FILE_SIZE, sizeof(UI8_T))))
    {
        printf("\r\nCFGDB: Allocating internal config file shadow memory failed.\r\n");
        assert(0);
    }
*/



    return;

} /* CFGDB_MGR_Initiate_System_Resources */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for CFGDB in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void CFGDB_MGR_AttachSystemResources(void)
{
    shmem_data_p = (CFGDB_MGR_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_CFGDB_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_CFGDB_OM, &cfgdb_mgr_sem_id);
}


/*----------------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * Purpose: This function initializes all function pointer registration operations.
 * Input:   None.
 * Output:  None.
 * Return:  None.
 * Notes:   None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_Create_InterCSC_Relation(void)
{
    /*
    if (FALSE == ISC_Register_Service_CallBack(ISC_CFGDB_SID, CFGDB_MGR_ReceivePacket_CallBack))
    {
        printf("\r\nCFGDB: Failed to register ISC callback function.\r\n");
        assert(0);
    }
    */

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("cfgdb", SYS_BLD_CFGDB_GROUP_IPCMSGQ_KEY, CFGDB_MGR_BackdoorMain);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter master mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_MGR_EnterMasterMode(void)
{
    UI32_T unit_id;

    if ( FALSE == STKTPLG_POM_GetMyUnitID(&(shmem_data_p->cfgdb_mgr_my_unit_id)) )
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to get my unit ID from STKTPLG.\r\n");
    }


    for(unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {

        if (TRUE == STKTPLG_POM_UnitExist(unit_id))
        {

            CFGDB_UTIL_SET_UNIT_EXISTING(unit_id, TRUE, shmem_data_p->cfgdb_mgr_existing_units);
        }
        else
        {

            CFGDB_UTIL_SET_UNIT_EXISTING(unit_id, FALSE, shmem_data_p->cfgdb_mgr_existing_units);
        }
    }


    CFGDB_UTIL_InitLocalSyncWorkingArea(&shmem_data_p->local_sync_work_area,          SYS_TYPE_STACKING_MASTER_MODE);
    CFGDB_UTIL_InitLocalSyncWorkingArea(&shmem_data_p->internal_local_sync_work_area, SYS_TYPE_STACKING_MASTER_MODE);


    /* set variables in reomte sync work area
     */

    memset(&shmem_data_p->remote_sync_work_area,          0, sizeof(shmem_data_p->remote_sync_work_area));
    memset(&shmem_data_p->internal_remote_sync_work_area, 0, sizeof(shmem_data_p->internal_remote_sync_work_area));


    if ((FALSE == CFGDB_UTIL_InitConfigFileShadow((UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME, &(shmem_data_p->config_file_shadow),            SYS_TYPE_STACKING_MASTER_MODE)) ||
        (FALSE == CFGDB_UTIL_InitConfigFileShadow((UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME, &shmem_data_p->internal_config_file_shadow,   SYS_TYPE_STACKING_MASTER_MODE)))
    {

        assert(0);
    }

    /* to clean up shadow to prevent from error using,
     * only init_time_descriptor is need to keep
     */

    memcpy(shmem_data_p->init_time_descriptor, shmem_data_p->config_file_shadow.descriptor,          sizeof(shmem_data_p->init_time_descriptor));
    memcpy(shmem_data_p->internal_init_time_descriptor, shmem_data_p->internal_config_file_shadow.descriptor, sizeof(shmem_data_p->internal_init_time_descriptor));

    memset(shmem_data_p->config_file_shadow.descriptor,          0, sizeof(shmem_data_p->config_file_shadow.descriptor));
    memset(shmem_data_p->internal_config_file_shadow.descriptor, 0, sizeof(shmem_data_p->internal_config_file_shadow.descriptor));

    shmem_data_p->cfgdb_mgr_available_section_handler = 1;
    shmem_data_p->cfgdb_mgr_allow_to_open             = TRUE;
    shmem_data_p->cfgdb_mgr_run_timer                 = FALSE;
    shmem_data_p->cfgdb_mgr_shutdown_write            = FALSE;
    shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler = 0;

    memset(shmem_data_p->cfgdb_mgr_section_id_table, 0, sizeof(shmem_data_p->cfgdb_mgr_section_id_table));
    memset(shmem_data_p->cfgdb_mgr_section_type_table, 0, sizeof(shmem_data_p->cfgdb_mgr_section_type_table));
    shmem_data_p->cfgdb_backdoor_flags.display_flash          = CFGDB_TYPE_BACKDOOR_INIT_STATE;
    shmem_data_p->cfgdb_backdoor_flags.display_tx             = CFGDB_TYPE_BACKDOOR_INIT_STATE;
    shmem_data_p->cfgdb_backdoor_flags.display_rx             = CFGDB_TYPE_BACKDOOR_INIT_STATE;
    shmem_data_p->cfgdb_backdoor_flags.display_tick           = FALSE; /*CFGDB_TYPE_BACKDOOR_INIT_STATE;*/
    shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level  = 0;
    shmem_data_p->cfgdb_backdoor_flags.disable_writing        = FALSE;
    CFGDB_UTIL_SetDebugFlags(&shmem_data_p->cfgdb_backdoor_flags);
    CFGDB_UTIL_SetSemID(cfgdb_mgr_sem_id);

    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);

 } /* CFGDB_MGR_EnterMasterMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter slave mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_MGR_EnterSlaveMode(void)
{
    UI32_T  unit_id;
    BOOL_T  content_changed;
    UI32_T  dummy;

    if (FALSE == STKTPLG_POM_GetMyUnitID(&shmem_data_p->cfgdb_mgr_my_unit_id))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to get my unit ID from STKTPLG.\r\n");
    }

    for(unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        if (TRUE == STKTPLG_POM_UnitExist(unit_id))
        {
            CFGDB_UTIL_SET_UNIT_EXISTING(unit_id, TRUE, shmem_data_p->cfgdb_mgr_existing_units);
        }
        else
        {
            CFGDB_UTIL_SET_UNIT_EXISTING(unit_id, FALSE, shmem_data_p->cfgdb_mgr_existing_units);
        }
    }

    CFGDB_UTIL_InitLocalSyncWorkingArea(&shmem_data_p->local_sync_work_area,          SYS_TYPE_STACKING_SLAVE_MODE);
    CFGDB_UTIL_InitLocalSyncWorkingArea(&shmem_data_p->internal_local_sync_work_area, SYS_TYPE_STACKING_SLAVE_MODE);

    /* To get from FLASH
     */
    if ((FALSE == CFGDB_UTIL_InitConfigFileShadow((UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME, &shmem_data_p->config_file_shadow, SYS_TYPE_STACKING_SLAVE_MODE)) ||
        (FALSE == CFGDB_UTIL_InitConfigFileShadow((UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME, &shmem_data_p->internal_config_file_shadow,   SYS_TYPE_STACKING_SLAVE_MODE)))
    {
        assert(0);
    }

    /* 1) Read out local part, and
     * 2) erase global part, that is prepared for master to sync
     */
    if ((FALSE == CFGDB_UTIL_ReOrgSections(&shmem_data_p->config_file_shadow,            SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION, TRUE, &content_changed, &dummy)) ||
        (FALSE == CFGDB_UTIL_ReOrgSections(&shmem_data_p->internal_config_file_shadow,   SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION, TRUE, &content_changed, &dummy)) )
    {
        assert(0);
    }

    /* shutdown API is used when master and slave mode both
     */
    shmem_data_p->cfgdb_mgr_shutdown_write            = FALSE;

    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);

} /* CFGDB_MGR_EnterSlaveMode */


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.

 * -------------------------------------------------------------------------*/
void CFGDB_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* nothing to do.
     */
    ;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void CFGDB_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* nothing to do.
     */
    ;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
} /* CFGDB_MGR_EnterTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Set transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_MGR_SetTransitionMode(void)
{

	CFGDB_UTIL_SetSemID(cfgdb_mgr_sem_id);

    CFGDB_MGR_Shutdown();

    shmem_data_p->cfgdb_mgr_run_timer = FALSE;

    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);

} /* End of CFGDB_MGR_SetTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function will get current operation mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  Current operation mode.
 *           1. SYS_TYPE_STACKING_TRANSITION_MODE
 *           2. SYS_TYPE_STACKING_MASTER_MODE
 *           3. SYS_TYPE_STACKING_SLAVE_MODE
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T CFGDB_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);

}/* End of CFGDB_MGR_GetOperationMode() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_Open
 *-------------------------------------------------------------------------
 * PURPOSE : Called after CSC enter master mode.
 * INPUT   : section_id       -- This value is pre-defined in CFGDB.
 *                               Each CSC shall use specific naming constant
 *                               that is just belong to this CSC.
 *           record_size      -- Fixed size of record in section.
 *           record_number    -- Record number.
 *           section_type     -- The type of this section:
 *                               CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL:
 *                                    Kept in local unit and could be deleted outside CFGDB.
 *                               CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL:
 *                                    Sync to other unit and could be deleted outside CFGDB.
 *                               CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL:
 *                                    Kept in local unit and could NOT be deleted outside CFGDB.
 *                               CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL
 *                                    Sync to other unit and could NOT be deleted outside CFGDB.
 *           need_to_sync     -- To tell CSC that sync is necessary or not.
 * OUTPUT  : section_handler  -- Section handler, that could be used to
 *                               read/write this section.
 * RETURN  : TRUE/FALSE.
 * NOTE    : 1. This function will allocate buffer which length is
 *              record_size*record_number bytes and set the buffer with
 *              default value provided by CSC and then read the related
 *              section of data in binary into the buffer.
 *           2. If record size is changed (smaller or larger), we have to
 *              read the binary file record by record to in the right
 *              position in shadow.
 *           3. No semaphore is necessary, because STKCTRL call CSC enter
 *              master mode sequentially, and each CSC call this API before
 *              return.
 *           4. No need to generate file level checksum, because before
 *              provision complete, the data is changed only because calling
 *              CFGDB_MGR_Open(), and data will be firm until CFGDB_MGR_EndOfOpen().
 *           5. No need to set dirty flag in both local and remote units, because
 *              in CFGDB_MGR_EndOfOpen(), all dirty flags will be trun on.
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_Open (UI32_T section_id,       UI32_T record_size,
                       UI32_T record_number,    UI32_T *section_handler,
                       UI32_T section_type ,    BOOL_T *need_to_sync)
{
    UI8_T                           filename[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    CFGDB_TYPE_LocalSyncWorkArea_T  *sync_work_area;
    CFGDB_TYPE_SectionDescriptor_T  *init_descriptor;
    CFGDB_TYPE_ConfigFile_T         *config_shadow;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* check the flag to open section
     */
    if (FALSE == shmem_data_p->cfgdb_mgr_allow_to_open)
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Section ID: %lu, opening section is not allowed.\r\n", (unsigned long)section_id);
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* set *section_handler as an invalid value, 0, to prevent from that CSC don't check return vlaue
     * and use the *section_handler directly.
     * If this API return TRUE, a valid *section_handler is outputed.
     */
    *section_handler = 0;

    /* argument checking I: section_id
     */
    if ((section_id < SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION) ||
        (section_id > SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION) )
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* argument checking III: record_size and record_number, only check overall here,
     * for per file checking, in UTIL
     */
    SYSFUN_Debug_Printf("\r\n%s(%d):recored_size=%ld, record_number=%ld, binary_file_size=%d",__FUNCTION__, __LINE__, (long)record_size, (long)record_number, CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE);
    if (record_size*record_number > CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE)
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: SYS_ADPT_BINARY_CFG_FILE_SIZE is too small (1).\r\n");
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (0 != shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler)
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Section handler: %lu, need to sync but not sync.\r\n", (unsigned long)shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler);
        shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler = 0;
    }

    /* argument checking IV: section type check
     */
    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  != section_type) &&
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL != section_type) &&
        (CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL  != section_type) &&
        (CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL != section_type) )
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* for Beagle case here, because Beagle is urgent, and Beagle is standalone.
     */
    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == section_type) ||
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == section_type))
    {
        strcpy((char *)filename, CFGDB_MGR_BINARY_CFG_FILE_NAME);
        config_shadow = &shmem_data_p->config_file_shadow;
        sync_work_area = &shmem_data_p->local_sync_work_area;
        init_descriptor = shmem_data_p->init_time_descriptor;
    }
    else
    {
        strcpy((char *)filename, CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME);
        config_shadow = &shmem_data_p->internal_config_file_shadow;
        sync_work_area = &shmem_data_p->internal_local_sync_work_area;
        init_descriptor = shmem_data_p->internal_init_time_descriptor;
    }

    if (0 == (*section_handler = CFGDB_UTIL_OpenInfoSync (section_id,
                                                          record_size,
                                                          record_number,
                                                          shmem_data_p->cfgdb_mgr_available_section_handler,
                                                          section_type,
                                                          filename,
                                                          sync_work_area,
                                                          init_descriptor,
                                                          config_shadow,
                                                          need_to_sync)))
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    shmem_data_p->cfgdb_mgr_section_id_table[*section_handler-1] = section_id;
    shmem_data_p->cfgdb_mgr_section_type_table[*section_handler-1] = section_type;

    /* shift the value for future sectioon open
     */
    shmem_data_p->cfgdb_mgr_available_section_handler++;

    /* for Beagle case here, because Beagle is urgent. Will in CFGDB_MGR_Sync() in Mercury_V2
     */
    if (TRUE == *need_to_sync)
    {
        shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler = *section_handler;
    }

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);

} /* CFGDB_MGR_Open */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_EndOfOpen
 *-------------------------------------------------------------------------
 * PURPOSE : Called by STKCTRL after all CSCs calling CFGDB_MGR_Open.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 *         : None
 * NOTE    : 1. Generate file level checksum.
 *           2. Trun on all dirty flags in all in both local and reomte
 *              units.
 *-------------------------------------------------------------------------
 */
void CFGDB_MGR_EndOfOpen(void)
{
    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    /* set the flag to reject invalid action to open section
     */
    shmem_data_p->cfgdb_mgr_allow_to_open = FALSE;

    /* external
     */
    CFGDB_UTIL_EndOfOpenFile(&shmem_data_p->local_sync_work_area,
                             &shmem_data_p->config_file_shadow,
                             &shmem_data_p->remote_sync_work_area,
                             shmem_data_p->cfgdb_mgr_available_section_handler,
                             shmem_data_p->cfgdb_mgr_existing_units,
                             shmem_data_p->cfgdb_mgr_my_unit_id);

    /* internal
     */
    CFGDB_UTIL_EndOfOpenFile(&shmem_data_p->internal_local_sync_work_area,
                             &shmem_data_p->internal_config_file_shadow,
                             &shmem_data_p->internal_remote_sync_work_area,
                             shmem_data_p->cfgdb_mgr_available_section_handler,
                             shmem_data_p->cfgdb_mgr_existing_units,
                             shmem_data_p->cfgdb_mgr_my_unit_id);

    /* begin to run timer in master, i.e. begine write to FLASH and sync to slave
     * in this way, slaves don't write to FLASH, if master doesn't run timer
     */
    shmem_data_p->cfgdb_mgr_run_timer = TRUE;

    CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();

} /* end of CFGDB_MGR_EndOfOpen */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_ReadSection
 *-------------------------------------------------------------------------
 * Purpose: Read entire section from FLASH
 * Input:   section_handler   --- handle of section returned by CFGDB_MGR_Open
 * Output:  buf  --- data buffer to store read data
 * Return:  0:Success, other value:fail
 * Notes : called after CSC open a section
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_ReadSection (UI32_T section_handler, void *buffer)
{
    BOOL_T retval;
    CFGDB_TYPE_ConfigFile_T  *cfg_file_shadow_p;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == buffer)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ( (section_handler < SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION) ||
         (section_handler >= shmem_data_p->cfgdb_mgr_available_section_handler) )
    {
	    CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
	}

    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) )
    {
        /* external
         */
        cfg_file_shadow_p = &shmem_data_p->config_file_shadow;
    }
    else
    {
        /* internal
         */
        cfg_file_shadow_p = &shmem_data_p->internal_config_file_shadow;
    }


    retval = CFGDB_MGR_Read(section_handler,
                            buffer,
                            0,
                            CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler));

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(retval);

} /* CFGDB_MGR_ReadSection */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_SyncSection
 *-------------------------------------------------------------------------
 * PURPOSE : Used to sync data for some section between FLASH and data in CSC.
 * INPUT   : section_handler -- Section handler.
 *           data_buffer     -- The default table provide from CSC.
 * OUTPUT  : data_buffer     -- Data after sync.
 * RETURN  : None
 *         : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_SyncSection(UI32_T section_handler, void *data_buffer)
{
    UI32_T section_id;
    CFGDB_TYPE_ConfigFile_T         *config_shadow_p;
    CFGDB_TYPE_SectionDescriptor_T  *init_descriptor_p;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == data_buffer)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* In our design, Sync data must right after open, so section handler
     * shall be available handler just before now.
     */
	if (section_handler != shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler)
	{
	    SYSFUN_Debug_Printf("CFGDB: Section handler: %lu, not time to sync\r\n", (unsigned long)section_handler);
	    CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
	}
	else
	{
	    shmem_data_p->cfgdb_mgr_wait_for_sync_section_handler = 0;
	}

    if (0 == (section_id = shmem_data_p->cfgdb_mgr_section_id_table[section_handler-1]))
    {
	    CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) )
    {
        config_shadow_p = &shmem_data_p->config_file_shadow;
        init_descriptor_p = shmem_data_p->init_time_descriptor;
    }
    else
    {
        config_shadow_p = &shmem_data_p->internal_config_file_shadow;
        init_descriptor_p = shmem_data_p->internal_init_time_descriptor;
    }


    if (FALSE == CFGDB_UTIL_SyncData(section_handler,
                                     init_descriptor_p,
                                     config_shadow_p,
                                     data_buffer))
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}


/*----------------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_Read
 *----------------------------------------------------------------------------------
 * Purpose: Read data records of given section from FLASH
 * Input:   section_handler      --- Handle of section returned by CFGDB_MGR_Open
 *          offset  --- offset Number of bytes from origin of this section
 *          count   --- number of bytes to be read
 * Output:
 *          buf     --- buffer storage location for data
 * Return:  0:Success, other value:fail
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_Read (UI32_T section_handler, void *buffer, UI32_T offset, UI32_T length)
{
    CFGDB_TYPE_ConfigFile_T  *cfg_file_shadow_p;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == buffer)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ( (section_handler < SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION) ||
         (section_handler >= shmem_data_p->cfgdb_mgr_available_section_handler) )
    {
	    CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) )
    {
        /* external
         */
        cfg_file_shadow_p = &shmem_data_p->config_file_shadow;
    }
    else
    {
        /* internal
         */
        cfg_file_shadow_p = &shmem_data_p->internal_config_file_shadow;
    }


    if (offset > CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler) ||
        length > CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler) ||
        (offset+length) > CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler))
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }


    CFGDB_MGR_EnterCriticalSection();
    {
        memcpy(buffer, (UI8_T *)CFGDB_MGR_SECTION_DATA_ADDR_IN_SHADOW(cfg_file_shadow_p, section_handler)+offset, length);
    }
    CFGDB_MGR_LeaveCriticalSection();

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);

} /* CFGDB_MGR_Read */

/*----------------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_WriteSection
 *----------------------------------------------------------------------------------
 * Purpose: Write entire section data to FLASH
 * Input:   section_handler    --- handle of section
 *          buf   --- data buffer
 * Output:  none
 * Return:  0:Success, other value:fail
 * Note: Any  "need_init" section,  after a CFGDB_MGR_Write be called,
 *       the  "need_init" mark will be moved.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_WriteSection (UI32_T section_handler, void *buffer)
{
    CFGDB_TYPE_ConfigFile_T  *cfg_file_shadow_p;
    BOOL_T retval;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (NULL == buffer)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((section_handler <  SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION) ||
        (section_handler >= shmem_data_p->cfgdb_mgr_available_section_handler) )
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) )
    {
        /* external
         */
        cfg_file_shadow_p = &shmem_data_p->config_file_shadow;
    }
    else
    {
        /* internal
         */
        cfg_file_shadow_p = &shmem_data_p->internal_config_file_shadow;
    }

    retval = CFGDB_MGR_Write(section_handler,
                             buffer,
                             0,
                             CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler));

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(retval);

} /* CFGDB_MGR_WriteSection */

/*----------------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_Write
 *----------------------------------------------------------------------------------
 * Purpose: write section data to FLASH
 * Input:   section_handler      --- Handle referring to open section
 *          buf     --- buffer storage location for data
 *          offset  --- offset Number of bytes from origin of this section
 *          length  --- number of bytes to be write
 * Output:
 * Return:  0:Success, other value:fail
 * Note: Any  "need_init" section,  after a CFGDB_MGR_Write be called,
 *       the  "need_init" mark will be moved.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_Write (UI32_T section_handler, void *buffer, UI32_T offset, UI32_T length)
{
    CFGDB_TYPE_ConfigFile_T  *cfg_file_shadow_p;
    UI32_T unit_id;
    CFGDB_TYPE_RemoteSyncWorkArea_T *remote_sync_work_p;
    CFGDB_TYPE_LocalSyncWorkArea_T  *loacal_sync_work_p;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (TRUE == shmem_data_p->cfgdb_mgr_allow_to_open)
    {
        /* while opening section is valid, i.e. system is in init state,
         * writing section is invalid
         */
        SYSFUN_Debug_Printf("\r\nCFGDB: Section handler: %lu, before \"end of open\", writing is invalid.\r\n", (unsigned long)section_handler);
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (TRUE == shmem_data_p->cfgdb_mgr_shutdown_write)
    {
        /* when this flag is on, topology will be change.
         * writing section is invalid, and reject request to make database stable
         */
        SYSFUN_Debug_Printf("\r\nCFGDB: Section handler: %lu, topology will be changed, writing is invalid.\r\n", (unsigned long)section_handler);
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (TRUE == shmem_data_p->cfgdb_backdoor_flags.disable_writing)
    {
        /* when turn on this flag by backdoor, writing will be rejected.
         */
        CFGDB_MGR_DBGMSG1(1, "\r\nCFGDB: Section handler: %lu, writing is disabled by backdoor.\r\n", (unsigned long)section_handler);
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (NULL == buffer)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((section_handler <  SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION) ||
        (section_handler >= shmem_data_p->cfgdb_mgr_available_section_handler) )
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
        (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) )
    {
        /* external
         */
        cfg_file_shadow_p  = &shmem_data_p->config_file_shadow;
        remote_sync_work_p = &shmem_data_p->remote_sync_work_area;
        loacal_sync_work_p = &shmem_data_p->local_sync_work_area;
    }
    else
    {
        /* internal
         */
        cfg_file_shadow_p  = &shmem_data_p->internal_config_file_shadow;
        remote_sync_work_p = &shmem_data_p->internal_remote_sync_work_area;
        loacal_sync_work_p = &shmem_data_p->internal_local_sync_work_area;
    }

    if ( offset > CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler)     ||
         length > CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler)     ||
        (offset+length-1) > CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler) )
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* Returning FALSE is based on design philosophy. Of course, we can return TRUE and
     * do nothing. But I think we need to warn the CSC, because it's not reasonable
     */
    if (0 == length)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }


    /* check: is the writing data same as the existed content
     */
    if (memcmp((UI8_T *)CFGDB_MGR_SECTION_DATA_ADDR_IN_SHADOW(cfg_file_shadow_p, section_handler)+offset, buffer, length) == 0 )
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* mark the dirty area and fill data
     * Note: In the criticl section, can't call any function except lib
     */
    CFGDB_MGR_EnterCriticalSection();
    {
        /* offset from starting address of shadow data
         */
        UI32_T data_start;
        UI32_T data_end;
        UI32_T block_id;

        /* write to shadow
         */
        /* 1. section data to local shadow
         */
        memcpy((UI8_T *)CFGDB_MGR_SECTION_DATA_ADDR_IN_SHADOW(cfg_file_shadow_p, section_handler)+offset, buffer, length);

        /* 2. generate section level checksum to local shadow
         */
        CFGDB_UTIL_Generate32BitChecksum((UI8_T *)(CFGDB_MGR_SECTION_DATA_ADDR_IN_SHADOW(cfg_file_shadow_p, section_handler)),
                                        CFGDB_MGR_SECTION_LEN(cfg_file_shadow_p, section_handler),
                                        &CFGDB_MGR_SECTION_CHECKSUM(cfg_file_shadow_p, section_handler));

        /* 3. generate file level checksum to local shadow
         */
        CFGDB_UTIL_Generate32BitChecksum((UI8_T *)cfg_file_shadow_p + sizeof(cfg_file_shadow_p->checksum),
                                        SYS_ADPT_BINARY_CFG_FILE_SIZE - sizeof(cfg_file_shadow_p->checksum),
                                        &(cfg_file_shadow_p->checksum));

        if ((CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
            (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) )
        {
            /* turn on dirty bit to mark the remote data are dirty, and only for dirty block,
             * and then wait for timer, max_dirty_timer or dirty_timer, timeout, to send to
             * remote unit.
             */
            data_start             = CFGDB_MGR_SECTION_DATA_ADDR_IN_SHADOW(cfg_file_shadow_p, section_handler) + offset - (UI8_T*)cfg_file_shadow_p;
            data_end               = data_start             + length                                              - 1; /*length always > 0*/

            for(unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
            {
                if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, shmem_data_p->cfgdb_mgr_existing_units))
                {
                    continue;
                }

                if (unit_id == shmem_data_p->cfgdb_mgr_my_unit_id)
                {
                    continue;
                }

                /* turn on remote dirty bit of data block dirty bit
                 */
                for (block_id= data_start/CFGDB_TYPE_DIRTY_BLOCK_SIZE; block_id<= data_end/CFGDB_TYPE_DIRTY_BLOCK_SIZE; block_id++)
                {
                    remote_sync_work_p->dirty[unit_id-1][block_id]= TRUE;
                }

                remote_sync_work_p->global_desc_dirty[unit_id-1][section_handler-1] = TRUE;
            }
        }

        /* always postpone SYS_ADPT_AUTOSAVE_HOLD_T ticks to write to FLASH
         */
        if (TRUE == shmem_data_p->cfgdb_backdoor_flags.display_tick)
        {
            SYSFUN_Debug_Printf("\r\nCFGDB: Writing [%lu] ticks\r\n", (unsigned long)SYSFUN_GetSysTick());
        }

        loacal_sync_work_p->dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_HOLD_TIME;

        /* if maximum delay write time is 0, that means current data are not "dirty"
         * then only this time to postpone "maximum" SYS_ADPT_AUTOSAVE_MAX_HOLD_T
         * ticks to write to FLASH.
         * if maximum delay write time is not 0, means data are dirty already,
         * and maximum delay write timer is truned on already, don't reset again.
         */
        if (loacal_sync_work_p->max_dirty_timer == 0)
        {
            loacal_sync_work_p->max_dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_MAX_HOLD_TIME;
        }

        /* turn on dirty bit to mark the local data are dirty,
         * and wait for timer, max_dirty_timer or dirty_timer, timeout, to write
         * to FLASH.
         */
        loacal_sync_work_p->dirty = TRUE;
    }
    CFGDB_MGR_LeaveCriticalSection();

	CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);

} /* CFGDB_MGR_Write */


/*----------------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_Flush
 *----------------------------------------------------------------------------------
 * Purpose: Flush shadow data to FLASH
 * Input:   None.
 * Output:  None.
 * Return:  TRUE/FALSE
 * Note:    None.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_Flush()
{
    UI32_T ret;
    UI32_T unit_id;
    UI32_T to_unit_bmp;
    UI32_T base_time;

    /* could be used in master and slave mode both
     */

    /* I. take care of local: Master mode and slave mode both
     */
    /* external
     */
    if (CFGDB_UTIL_WRITE_FLASH_FAIL == (ret = CFGDB_UTIL_FlushEventSync(&shmem_data_p->local_sync_work_area,
                                                                    &shmem_data_p->config_file_shadow,
                                                                    (UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME)))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to flush shadow to FLASH (ext.).\r\n");
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* internal
     */
    if (CFGDB_UTIL_WRITE_FLASH_FAIL == (ret = CFGDB_UTIL_FlushEventSync(&shmem_data_p->internal_local_sync_work_area,
                                                                        &shmem_data_p->internal_config_file_shadow,
                                                                        (UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME)))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to flush shadow to FLASH (int.).\r\n");
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* II. take care of remote: master only
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    /* trun off flash done flag
     */
    to_unit_bmp = 0;
    for(unit_id = 1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, shmem_data_p->cfgdb_mgr_existing_units))
        {
            continue;
        }

        if (unit_id == shmem_data_p->cfgdb_mgr_my_unit_id)
        {
            continue;
        }
        shmem_data_p->remote_sync_work_area.flash_done[unit_id-1] = FALSE;
        to_unit_bmp |= IUC_STACK_UNIT_BMP(unit_id);
    }

    /* send FALSH to all slave units
     */
    if (0 != to_unit_bmp)
    {
        CFGDB_UTIL_SendPacket(CFGDB_TYPE_PACKET_TYPE_FLASH, shmem_data_p->cfgdb_mgr_my_unit_id, to_unit_bmp, 0, 0, 0);
    }

    /* wait for remote shadow sync to FLASH
     */
    base_time = SYSFUN_GetSysTick();
    for(unit_id = 1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; )
    {
        /* don't check master self and not-existing slave unit
         */
        if ((TRUE    == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, shmem_data_p->cfgdb_mgr_existing_units)       )&&
            (unit_id != shmem_data_p->cfgdb_mgr_my_unit_id                       )&&
            (FALSE   == shmem_data_p->remote_sync_work_area.flash_done[unit_id-1]))
        {
            /*  1. existing unit,
             *  2. slave unit
             *  3. shadow not sync FALSH
             *  check itself again
             */
            ;
        }
        else
        {
            /*  1. not-existing unit,
             *  2. master unit
             *  3. shadow sync FALSH already
             *  check next unit
             */
            unit_id++;
        }

        /* if TCN occurs, and the stacking link is broken, then response packet could not
         * received any more, so, not wait
         */
        if ((SYSFUN_GetSysTick() - base_time)>= CFGDB_MGR_MAX_SHUTDOWN_WAITING_TIME)
        {
            SYSFUN_Debug_Printf("Flush timeout.\r\n");
            break;
        }

        /* sleep to make other tasks do something
         */
        SYSFUN_Sleep(5);
    }

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_Shutdown
 *----------------------------------------------------------------------------------
 * Purpose: Shutdown FLASH, i.e. return only after sync to local and remote units
 *          is completed and then do not update FLASH from dirty area further.
 * Input:   None.
 * Output:  None.
 * Return:  TRUE  -- FLASH already shutdown.
            FALSE -- Failed.
 * Notes:   1. Call by STKCTRL when tolopogy change.
 *          2. Current writing data must write completely.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_Shutdown (void)
{
    UI32_T unit_id;
    BOOL_T *ref_off_mem;                //[CFGDB_TYPE_DIRTY_BLOCK_NUMBER]
    BOOL_T *ref_off_global_desc_mem;    //[SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION]
    UI32_T base_time;

    /* could be used in master and slave mode both
     */


    /* set the flag to prevent from CSC try to write to CFGDB
     */
    if (FALSE == shmem_data_p->cfgdb_mgr_shutdown_write)
    {
        CFGDB_MGR_DBGMSG0(3, "CFGDB: Send shutdown event and wait");

        CFGDB_MGR_EnterCriticalSection();
        shmem_data_p->cfgdb_mgr_shutdown_write = TRUE;
        CFGDB_MGR_LeaveCriticalSection();

        while(FALSE == shmem_data_p->cfgdb_mgr_shutdown_write)
        {
            CFGDB_MGR_DBGMSG0(3, ".");
            SYSFUN_Sleep(5);
        }

        CFGDB_MGR_DBGMSG0(3, "Done.\n\r");
    }
    else
    {

        /* If CFGDB was shutdowned already, nothing to do,
         * return TRUE and do tothing.
         */
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }


    if(NULL == (ref_off_mem = calloc(CFGDB_TYPE_DIRTY_BLOCK_NUMBER, sizeof(BOOL_T))))
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (NULL == (ref_off_global_desc_mem = calloc(SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION, sizeof(BOOL_T))))
    {
        free(ref_off_mem);
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* wait for data sync to remote shadow: in master mode
     */
    memset(ref_off_mem,             0, CFGDB_TYPE_DIRTY_BLOCK_NUMBER*sizeof(BOOL_T));
    memset(ref_off_global_desc_mem, 0, SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION*sizeof(BOOL_T));

    base_time = SYSFUN_GetSysTick();

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE)
    {
        for(unit_id = 1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; )
        {
            int int_data = memcmp(  shmem_data_p->internal_remote_sync_work_area.dirty[unit_id-1],
                                    ref_off_mem,
                                    CFGDB_TYPE_DIRTY_BLOCK_NUMBER*sizeof(BOOL_T));

            int int_desc = memcmp(  shmem_data_p->internal_remote_sync_work_area.global_desc_dirty[unit_id-1],
                                    ref_off_global_desc_mem,
                                    SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION*sizeof(BOOL_T));

            int ext_data = memcmp(  shmem_data_p->remote_sync_work_area.dirty[unit_id-1],
                                    ref_off_mem,
                                    CFGDB_TYPE_DIRTY_BLOCK_NUMBER*sizeof(BOOL_T));

            int ext_desc = memcmp( shmem_data_p->remote_sync_work_area.global_desc_dirty[unit_id-1],
                                   ref_off_global_desc_mem,
                                   SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION*sizeof(BOOL_T));


            /* don't check master self and not-existing slave unit
             */
            if ( ((TRUE    == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, shmem_data_p->cfgdb_mgr_existing_units))                    &&
                  (unit_id != shmem_data_p->cfgdb_mgr_my_unit_id)                                                                 ) &&
                  ( (int_data!=0) || (int_desc!=0) || (ext_data!=0) || (ext_desc!=0)))
            {
                /*  1. existing unit,
                 *  2. slave unit
                 *  3. some dirty flags in on
                 *  4. send sync again
                 *  check itself again
                 */
                if ((int_data!=0) || (int_desc!=0))
                {
                    CFGDB_UTIL_SendSyncToSlave( &shmem_data_p->internal_config_file_shadow,
                                                &shmem_data_p->internal_remote_sync_work_area,
                                                shmem_data_p->cfgdb_mgr_existing_units,
                                                shmem_data_p->cfgdb_mgr_my_unit_id,
                                                0);

                }

                if ((ext_data!=0) || (ext_desc!=0))
                {
                    CFGDB_UTIL_SendSyncToSlave( &shmem_data_p->config_file_shadow,
                                                &shmem_data_p->remote_sync_work_area,
                                                shmem_data_p->cfgdb_mgr_existing_units,
                                                shmem_data_p->cfgdb_mgr_my_unit_id,
                                                1);
                }
            }
            else
            {
                /*  1. not-existing unit,
                 *  2. master unit
                 *  3. all dirty flags are off
                 *  check next unit
                 */
                unit_id++;
            }

            /* if TCN occurs, and the stacking link is broken, then response packet could not
             * received any more, so, not wait
             */
            if ((SYSFUN_GetSysTick() - base_time)>= CFGDB_MGR_MAX_SHUTDOWN_WAITING_TIME)
            {
                SYSFUN_Debug_Printf("Shutdown timeout.\r\n");
                break;
            }

            /* sleep to make other tasks do something
             */
            SYSFUN_Sleep(5);
        }
    }



    if (FALSE == CFGDB_MGR_Flush())
    {
        SYSFUN_Debug_Printf("Failed to flush shadow to FALSH\r\n");
        free(ref_off_mem);
        free(ref_off_global_desc_mem);
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    free(ref_off_mem);
    free(ref_off_global_desc_mem);


    CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);

} /* CFGDB_MGR_ShutDown */


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_ResetConfiguration
 *----------------------------------------------------------------------------------
 * Purpose: To reset the next boot configuration in FLASH.
 * Input:   type_bmp -- bit map of  CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL
 *                                  CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL
 *                                  CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL
 *                                  CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL
 *          shutdown -- To shutdown the service at the same time.
 * Output:  None.
 * Return:  TRUE/FALSE.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_ResetConfiguration(UI32_T reset_type_bmp, BOOL_T shutdown)
{
    BOOL_T reset_internal;
    BOOL_T reset_external;

    CFGDB_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (0 == reset_type_bmp)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (reset_type_bmp & (~(CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL)  |
                            CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL) |
                            CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL)  |
                            CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL) )))
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (TRUE == shutdown)
    {
        CFGDB_MGR_Shutdown();
    }

    reset_external = ( reset_type_bmp & CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL) ||
                       reset_type_bmp & CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL) );

    reset_internal = ( reset_type_bmp & CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL) ||
                       reset_type_bmp & CFGDB_MGR_SECTION_TYPE_TO_BMP(CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL) );


    if (TRUE == reset_external)
    {
        if (FS_RETURN_OK != FS_DeleteFile(DUMMY_DRIVE, (UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME))
        {
            CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    if (TRUE == reset_internal)
    {
        if (FS_RETURN_OK != FS_DeleteFile(DUMMY_DRIVE, (UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME))
        {
            CFGDB_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    CFGDB_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_SetTaskId
 *----------------------------------------------------------------------------------
 * Purpose: TASK tell MGR the task ID, for MGR to send event to TASK.
 * Input:   cfgdb_task_id - Task ID of CFGDB_TASK.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_SetTaskId(UI32_T cfgdb_task_id)
{
    shmem_data_p->cfgdb_mgr_task_id = cfgdb_task_id;
}

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_GetTaskId
 *----------------------------------------------------------------------------------
 * Purpose: TASK tell MGR the task ID, for MGR to send event to TASK.
 * Input:   cfgdb_task_id - Task ID of CFGDB_TASK.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_GetTaskId(UI32_T *cfgdb_task_id)
{
    *cfgdb_task_id = shmem_data_p->cfgdb_mgr_task_id;
}


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_ShutdownHandler
 *----------------------------------------------------------------------------------
 * Purpose: Timer handler shutdown event
 * Input:   None.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_ShutdownHandler(void)
{
    CFGDB_MGR_EnterCriticalSection();
    shmem_data_p->cfgdb_mgr_shutdown_write = TRUE;
    CFGDB_MGR_LeaveCriticalSection();
}


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_TimerHandler
 *----------------------------------------------------------------------------------
 * Purpose: Timer handler periodically called by CFGDB_TASK to deal with database
 *          sync to binary file and all slaves.
 * Input:   None.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_TimerHandler(void)
{
    UI32_T  ret;

    if (TRUE == shmem_data_p->cfgdb_backdoor_flags.display_tick)
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Timer [%lu] ticks\r\n", (unsigned long)SYSFUN_GetSysTick());
    }

    /* check to run timer in master or not, if master doesn't run timer,
     * slaves don't receice SYNC and doesn't dirty, and doesn't write to FLASH
     */
    if (FALSE == shmem_data_p->cfgdb_mgr_run_timer)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    /* If the shutdown flag is on, that means
     * 1) Flush is on going
     * 2) Reset configuration with shutdown is on going
     * Nothing to do.
     */
    if (TRUE == shmem_data_p->cfgdb_mgr_shutdown_write)
    {
        CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    /* Local external
     */
    if (CFGDB_UTIL_WRITE_FLASH_FAIL == (ret = CFGDB_UTIL_TimerEventSync(&shmem_data_p->local_sync_work_area,
                                                                        &shmem_data_p->config_file_shadow,
                                                                        (UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME)))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to sync shadow to FLASH (ext.).\r\n");
        CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }
    /* Local internal
     */
    if ( CFGDB_UTIL_WRITE_FLASH_FAIL == (ret = CFGDB_UTIL_TimerEventSync(&shmem_data_p->internal_local_sync_work_area,
                                                                         &shmem_data_p->internal_config_file_shadow,
                                                                         (UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME)))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to sync shadow to FLASH (int.).\r\n");
        CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }
    /* send SYNC multicast packet only in master
     */
    if (SYS_TYPE_STACKING_MASTER_MODE == CFGDB_MGR_GetOperationMode())
    {
        CFGDB_UTIL_SendSyncToSlave( &shmem_data_p->config_file_shadow,
                                    &shmem_data_p->remote_sync_work_area,
                                    shmem_data_p->cfgdb_mgr_existing_units,
                                    shmem_data_p->cfgdb_mgr_my_unit_id,
                                    1);

        CFGDB_UTIL_SendSyncToSlave( &shmem_data_p->internal_config_file_shadow,
                                    &shmem_data_p->internal_remote_sync_work_area,
                                    shmem_data_p->cfgdb_mgr_existing_units,
                                    shmem_data_p->cfgdb_mgr_my_unit_id,
                                    0);
    }
    CFGDB_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();

} /* CFGDB_MGR_TimerHandler */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_MGR_ReceivePacket
 * ------------------------------------------------------------------------
 * PURPOSE  : Receive packet service routine of CFGDB.
 * INPUT    : key     -- key.
 *            mem_ref -- memory reference.
 *            svc_id  -- service id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_ReceivePacket_CallBack(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id)
{
    UI8_T  *buf;
    UI32_T packet_type;
    UI32_T from_unit_id;
    UI32_T to_unit_bmp;
    UI32_T block_id;
    UI32_T is_external;
    UI8_T *payload;

    buf = L_MM_Mref_GetPdu(mref_handle_p, &packet_type); /* packet_type is used as dummy here */

    if(buf==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
        return FALSE;
    }

    packet_type  = ((CFGDB_TYPE_Packet_T *)buf)->packet_type;
    from_unit_id = ((CFGDB_TYPE_Packet_T *)buf)->from_unit_id;
    to_unit_bmp  = ((CFGDB_TYPE_Packet_T *)buf)->to_unit_bmp;
    block_id     = ((CFGDB_TYPE_Packet_T *)buf)->block_id;
    payload      = ((CFGDB_TYPE_Packet_T *)buf)->payload;
    is_external  = ((CFGDB_TYPE_Packet_T *)buf)->is_external;

    switch(packet_type)
    {
    case CFGDB_TYPE_PACKET_TYPE_SYNC:
        if (SYS_TYPE_STACKING_SLAVE_MODE == CFGDB_MGR_GetOperationMode())
        {
            CFGDB_MGR_EnterCriticalSection();
            {
                CFGDB_TYPE_ConfigFile_T         *tmp_config_file_shadow_p;
                CFGDB_TYPE_LocalSyncWorkArea_T  *tmp_local_sync_work_area_p;

                if (1 == is_external)
                {
                    tmp_config_file_shadow_p    =   &shmem_data_p->config_file_shadow;
                    tmp_local_sync_work_area_p  =   &shmem_data_p->local_sync_work_area;
                }
                else
                {
                    tmp_config_file_shadow_p    =   &shmem_data_p->internal_config_file_shadow;
                    tmp_local_sync_work_area_p  =   &shmem_data_p->internal_local_sync_work_area;
                }

                /* 1. write to shadow
                 */
                memcpy((UI8_T *)tmp_config_file_shadow_p + block_id*CFGDB_TYPE_DIRTY_BLOCK_SIZE,
                       payload,
                       CFGDB_TYPE_DIRTY_BLOCK_SIZE);

                /* 2. generate file level checksum
                 */
                CFGDB_UTIL_Generate32BitChecksum((UI8_T *)tmp_config_file_shadow_p + sizeof(tmp_config_file_shadow_p->checksum),
                                                 SYS_ADPT_BINARY_CFG_FILE_SIZE - sizeof(tmp_config_file_shadow_p->checksum),
                                                 &(tmp_config_file_shadow_p->checksum));

                /* 3. trun on local dirty
                 */
                (*tmp_local_sync_work_area_p).dirty = TRUE;

                /* 4. set timer
                 */
                /* always postpone SYS_ADPT_AUTOSAVE_HOLD_T ticks to write to FLASH
                 */
                (*tmp_local_sync_work_area_p).dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_HOLD_TIME;

                /* if maximum delay write time is 0, that means current data are not "dirty"
                 * then only this time to postpone "maximum" SYS_ADPT_AUTOSAVE_MAX_HOLD_T
                 * ticks to write to FLASH.
                 * if maximum delay write time is not 0, means data are dirty already,
                 * and maximum delay write timer is truned on already, don't reset again.
                 */
                if ((*tmp_local_sync_work_area_p).max_dirty_timer == 0)
                {
                    (*tmp_local_sync_work_area_p).max_dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_MAX_HOLD_TIME;
                }
            }
            CFGDB_MGR_LeaveCriticalSection();

            /* 4. send back ack now (not after write to FLASH)
             */
            CFGDB_UTIL_SendPacket(CFGDB_TYPE_PACKET_TYPE_ACK, shmem_data_p->cfgdb_mgr_my_unit_id, IUC_STACK_UNIT_BMP(from_unit_id), block_id, 0, is_external);
        }
        else
        {
            CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Error, receive SYNC in master.\r\n");
        }
        break;

    case CFGDB_TYPE_PACKET_TYPE_ACK:
        if (SYS_TYPE_STACKING_MASTER_MODE == CFGDB_MGR_GetOperationMode())
        {
            /* turn off remote unit dirty bit
             */
            if (1 == is_external)
            {
                shmem_data_p->remote_sync_work_area.dirty[from_unit_id-1][block_id] = FALSE;
            }
            else
            {
                shmem_data_p->internal_remote_sync_work_area.dirty[from_unit_id-1][block_id] = FALSE;
            }
        }
        else
        {
            CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Error, receive ACK in slave.\r\n");
        }
        break;

    case CFGDB_TYPE_PACKET_TYPE_FLASH:
        if (SYS_TYPE_STACKING_SLAVE_MODE == CFGDB_MGR_GetOperationMode())
        {
            BOOL_T ret;

            /* I. take care of local
             */
            /* external
             */
            if (CFGDB_UTIL_WRITE_FLASH_FAIL == (ret = CFGDB_UTIL_FlushEventSync(&shmem_data_p->local_sync_work_area,
                                                                                &shmem_data_p->config_file_shadow,
                                                                                (UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME)))
            {
                SYSFUN_Debug_Printf("\r\nCFGDB: Failed to flush shadow to FLASH (ext.).\r\n");
            }

            /* internal
             */
           if (CFGDB_UTIL_WRITE_FLASH_FAIL == (ret = CFGDB_UTIL_FlushEventSync( &shmem_data_p->internal_local_sync_work_area,
                                                                                &shmem_data_p->internal_config_file_shadow,
                                                                                (UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME)))
            {
                SYSFUN_Debug_Printf("\r\nCFGDB: Failed to flush shadow to FLASH (int.).\r\n");
            }

            /* send back FLASH_DONE now
             */
           CFGDB_UTIL_SendPacket(CFGDB_TYPE_PACKET_TYPE_FLASH_DONE, shmem_data_p->cfgdb_mgr_my_unit_id, IUC_STACK_UNIT_BMP(from_unit_id), 0, 0, 0);

        }
        else
        {
            CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Error, receive FLASH in master.\r\n");
        }
        break;

    case CFGDB_TYPE_PACKET_TYPE_FLASH_DONE:
        if (SYS_TYPE_STACKING_MASTER_MODE == CFGDB_MGR_GetOperationMode())
        {
            /* FLASH is for both external and internal,
             * use external working area as the flag.
             */
            shmem_data_p->remote_sync_work_area.flash_done[from_unit_id-1] = TRUE;
        }
        else
        {
            CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Error, receive FLASH_DONE in slave.\r\n");
        }
        break;

    case CFGDB_TYPE_PACKET_TYPE_DESC_SYNC:
        if (SYS_TYPE_STACKING_SLAVE_MODE == CFGDB_MGR_GetOperationMode())
        {
            UI32_T available_sec_hd;
            UI32_T sec_hd;
            BOOL_T available_sec_hd_found;

            CFGDB_MGR_EnterCriticalSection();
            {
                CFGDB_TYPE_ConfigFile_T         *tmp_config_file_shadow_p;
                CFGDB_TYPE_LocalSyncWorkArea_T  *tmp_local_sync_work_area_p;

                if (1 == is_external)
                {
                    tmp_config_file_shadow_p    =   &shmem_data_p->config_file_shadow;
                    tmp_local_sync_work_area_p  =   &shmem_data_p->local_sync_work_area;
                }
                else
                {
                    tmp_config_file_shadow_p    =   &shmem_data_p->internal_config_file_shadow;
                    tmp_local_sync_work_area_p  =   &shmem_data_p->internal_local_sync_work_area;
                }

                available_sec_hd_found = FALSE;
                if ((shmem_data_p->config_file_shadow.descriptor[block_id-1].section_id == 0) &&
                    (shmem_data_p->internal_config_file_shadow.descriptor[block_id-1].section_id == 0))
                {
                    /* the most preferable handler in slave is the one that the same as master
                     */
                    available_sec_hd = block_id;
                    available_sec_hd_found =TRUE;
                }
                else if ((shmem_data_p->config_file_shadow.descriptor[block_id-1].section_id == ((CFGDB_TYPE_SectionDescriptor_T*)payload)->section_id) ||
                         (shmem_data_p->internal_config_file_shadow.descriptor[block_id-1].section_id == ((CFGDB_TYPE_SectionDescriptor_T*)payload)->section_id))
                {
                    /* section ID is the same.
                     */
                    available_sec_hd = block_id;
                    available_sec_hd_found =TRUE;
                }
                else
                {
                    for(sec_hd=1; sec_hd<=SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION; sec_hd++)
                    {
                        if ((shmem_data_p->config_file_shadow.descriptor[sec_hd-1].section_id == 0) &&
                            (shmem_data_p->internal_config_file_shadow.descriptor[sec_hd-1].section_id == 0))
                        {
                            available_sec_hd = sec_hd;
                            available_sec_hd_found = TRUE;
                            break;
                        }
                    }
                }

                if (FALSE == available_sec_hd_found)
                {
                    SYSFUN_Debug_Printf("\r\nCFGDB: No available section handler in slave.\r\n");
                }
                else
                {
                    /* 1. write to shadow,
                     */
                    memcpy((UI8_T *)(&(tmp_config_file_shadow_p->descriptor[available_sec_hd-1])),
                           payload,
                           sizeof(CFGDB_TYPE_SectionDescriptor_T));

                    /* 2. generate file level checksum
                     */
                    CFGDB_UTIL_Generate32BitChecksum((UI8_T *)tmp_config_file_shadow_p + sizeof(tmp_config_file_shadow_p->checksum),
                                                     SYS_ADPT_BINARY_CFG_FILE_SIZE - sizeof(tmp_config_file_shadow_p->checksum),
                                                     &(tmp_config_file_shadow_p->checksum));

                    /* 3. trun on local dirty
                     */
                    (*tmp_local_sync_work_area_p).dirty = TRUE;

                    /* 4. set timer
                     */
                    /* always postpone SYS_ADPT_AUTOSAVE_HOLD_T ticks to write to FLASH
                     */
                    (*tmp_local_sync_work_area_p).dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_HOLD_TIME;

                    /* if maximum delay write time is 0, that means current data are not "dirty"
                     * then only this time to postpone "maximum" SYS_ADPT_AUTOSAVE_MAX_HOLD_T
                     * ticks to write to FLASH.
                     * if maximum delay write time is not 0, means data are dirty already,
                     * and maximum delay write timer is truned on already, don't reset again.
                     */
                    if ((*tmp_local_sync_work_area_p).max_dirty_timer == 0)
                    {
                        (*tmp_local_sync_work_area_p).max_dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_MAX_HOLD_TIME;
                    }
                }
            }
            CFGDB_MGR_LeaveCriticalSection();

            /* 4. send back ack now (not after write to FLASH)
             */
            CFGDB_UTIL_SendPacket(CFGDB_TYPE_PACKET_TYPE_DESC_ACK, shmem_data_p->cfgdb_mgr_my_unit_id, IUC_STACK_UNIT_BMP(from_unit_id), block_id, 0, is_external);
        }
        else
        {
            CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Error, receive DESC_SYNC in master.\r\n");
        }
        break;

    case CFGDB_TYPE_PACKET_TYPE_DESC_ACK:
        if (SYS_TYPE_STACKING_MASTER_MODE == CFGDB_MGR_GetOperationMode())
        {
            /* turn off remote unit dirty bit, here block_id is used to be "section handler"
             */
            if (1 == is_external)
            {
                shmem_data_p->remote_sync_work_area.global_desc_dirty[from_unit_id-1][block_id-1] = FALSE;
            }
            else
            {
                shmem_data_p->internal_remote_sync_work_area.global_desc_dirty[from_unit_id-1][block_id-1] = FALSE;
            }
        }
        else
        {
            CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Error, receive ACK in slave.\r\n");
        }
        break;


    default:
        CFGDB_MGR_DBGMSG0(3, "\r\nCFGDB: Invalid packet type.\r\n");
        break;
    }

    if (TRUE == shmem_data_p->cfgdb_backdoor_flags.display_rx)
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: RX packet. Type [%lu]. From [%lu]. To [%lx]. Block ID [%lu]. Is_ext [%lu]. [%lu] ticks\r\n\r\n",
                                                                                                            (unsigned long)packet_type,
                                                                                                            (unsigned long)from_unit_id,
                                                                                                            (unsigned long)to_unit_bmp,
                                                                                                            (unsigned long)block_id,
                                                                                                            (unsigned long)is_external,
                                                                                                            (unsigned long)SYSFUN_GetSysTick());
    }

    L_MM_Mref_Release(&mref_handle_p);
    return TRUE;
    
    to_unit_bmp; /* workaround for compiler warning:variable 'to_unit_bmp' set but not used */
}

/* LOCAL SUBPROGRAM BODIES
 */
static void CFGDB_MGR_DumpSectionAttributes(void)
{
    UI32_T hd;
    char   *type_string[] = {"external local",
                             "external global",
                             "internal local",
                             "internal global"
                            };


    printf("\n\r");
    printf("========[External]=========\n\r");
    for (hd=1; hd<=SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION; hd++)
    {
        if (0 == shmem_data_p->config_file_shadow.descriptor[hd-1].section_id)
        {
            continue;
        }

        printf("Handler:    %lu\n\r",   (unsigned long)hd);
        printf("   ID:      %lu\n\r",   (unsigned long)shmem_data_p->config_file_shadow.descriptor[hd-1].section_id);
        printf("   Type:    %s\n\r",    type_string[shmem_data_p->config_file_shadow.descriptor[hd-1].section_type - 1]);
        printf("   Offset:  %lu\n\r",   (unsigned long)shmem_data_p->config_file_shadow.descriptor[hd-1].section_start_offset);
        printf("   Size:    %lu\n\r",   (unsigned long)shmem_data_p->config_file_shadow.descriptor[hd-1].section_record_size);
        printf("   Number:  %lu\n\r",   (unsigned long)shmem_data_p->config_file_shadow.descriptor[hd-1].section_record_number);

        if (shmem_data_p->cfgdb_mgr_section_id_table[hd-1] != shmem_data_p->config_file_shadow.descriptor[hd-1].section_id)
        {
            printf("Not the same: %lu/%lu\n\r", (unsigned long)shmem_data_p->cfgdb_mgr_section_id_table[hd-1], (unsigned long)shmem_data_p->config_file_shadow.descriptor[hd-1].section_id);
        }
        else
        {
            printf("The same\n\r");
        }

        printf("\n\r");
    }

    printf("========[Internal]=========\n\r");
    for (hd=1; hd<=SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION; hd++)
    {
        if (0 == shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_id)
        {
            continue;
        }

        printf("Handler:    %lu\n\r",   (unsigned long)hd);
        printf("   ID:      %lu\n\r",   (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_id);
        printf("   Type:    %s\n\r",    type_string[shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_type - 1]);
        printf("   Offset:  %lu\n\r",   (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_start_offset);
        printf("   Size:    %lu\n\r",   (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_record_size);
        printf("   Number:  %lu\n\r",   (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_record_number);

        if (shmem_data_p->cfgdb_mgr_section_id_table[hd-1] != shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_id)
        {
            printf("Not the same: %lu/%lu\n\r", (unsigned long)shmem_data_p->cfgdb_mgr_section_id_table[hd-1], (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[hd-1].section_id);
        }
        else
        {
            printf("The same\n\r");
        }


        printf("\n\r");
    }

    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_BackdoorMain
 *-------------------------------------------------------------------------
 * PURPOSE: Backdoor of CFGDB.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
static void CFGDB_MGR_BackdoorMain(void)
{
    UI8_T  key_in_string[16];
    UI32_T i;
    UI32_T section_handler;
    UI32_T section_id;
    UI32_T start_offset;
    UI32_T length;
    UI32_T time;
    UI32_T unit;
    UI32_T block;
    BOOL_T is_got;
    BOOL_T is_external;
    BOOL_T debug_flag_changed = FALSE;

    while(1)
    {
        for(i=0; i<=9; i++)
            BACKDOOR_MGR_Printf("\r\n");

        BACKDOOR_MGR_Printf("----------------------------------------\r\n");
        BACKDOOR_MGR_Printf("Main menu of CFGDB backdoor:\r\n\r\n");
        BACKDOOR_MGR_Printf("   1. Toggle state to display message after writing to FLASH. [%s]\r\n", shmem_data_p->cfgdb_backdoor_flags.display_flash ?  "TRUE" : "FALSE");
        BACKDOOR_MGR_Printf("   2. Toggle state to display message when TX packet. [%s]\r\n",         shmem_data_p->cfgdb_backdoor_flags.display_tx ?     "TRUE" : "FALSE");
        BACKDOOR_MGR_Printf("   3. Toggle state to display message when RX packet. [%s]\r\n",         shmem_data_p->cfgdb_backdoor_flags.display_rx ?     "TRUE" : "FALSE");
        BACKDOOR_MGR_Printf("   4. Toggle state to display message of timer. [%s]\r\n",               shmem_data_p->cfgdb_backdoor_flags.display_tick ?   "TRUE" : "FALSE");
        BACKDOOR_MGR_Printf("   5. Dump external header shadow memory.\r\n");
        BACKDOOR_MGR_Printf("   6. Dump external body shadow memory.\r\n");
        BACKDOOR_MGR_Printf("   7. Dump external local sync work area.\r\n");
        BACKDOOR_MGR_Printf("   8. Dump external remote sync work area.\r\n");
        BACKDOOR_MGR_Printf("   9. Dump internal header shadow memory.\r\n");
        BACKDOOR_MGR_Printf("  10. Dump internal body shadow memory.\r\n");
        BACKDOOR_MGR_Printf("  11. Dump internal local sync work area.\r\n");
        BACKDOOR_MGR_Printf("  12. Dump internal remote sync work area.\r\n");
        BACKDOOR_MGR_Printf("  13. Section ID to section handler.\r\n");
        BACKDOOR_MGR_Printf("  14. Section handler to section ID.\r\n");
        BACKDOOR_MGR_Printf("  15. Delete local unit, external CFGDB file.\r\n");
        BACKDOOR_MGR_Printf("  16. Delete local unit, internal CFGDB file.\r\n");
        BACKDOOR_MGR_Printf("  17. API test: CFGDB_MGR_Read().\r\n");
        BACKDOOR_MGR_Printf("  18. API test: CFGDB_MGR_Write().\r\n");
        BACKDOOR_MGR_Printf("  19. API test: CFGDB_MGR_Flush().\r\n");
        BACKDOOR_MGR_Printf("  20. Dump attributes of sections.\r\n");
        BACKDOOR_MGR_Printf("  21. Set debug message level. Now: [%lu]\r\n", (unsigned long)shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level);
        BACKDOOR_MGR_Printf("  22. Disable/Enable writing to FLASH. [%s]\r\n", shmem_data_p->cfgdb_backdoor_flags.disable_writing ? "Disable" : "Enable");

        BACKDOOR_MGR_Printf("\r\n  99. To exit from backdoor.\r\n\r\n");

        BACKDOOR_MGR_Printf("Key in the choice: ");
        memset(key_in_string, 0, sizeof(key_in_string));
        BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 2);
        BACKDOOR_MGR_Printf("\r\n");

        switch(atoi((char *)key_in_string))
        {
        case 1:
            shmem_data_p->cfgdb_backdoor_flags.display_flash = !shmem_data_p->cfgdb_backdoor_flags.display_flash;
            debug_flag_changed = TRUE;
            break;

        case 2:
            shmem_data_p->cfgdb_backdoor_flags.display_tx = !shmem_data_p->cfgdb_backdoor_flags.display_tx;
            debug_flag_changed = TRUE;
            break;

        case 3:
            shmem_data_p->cfgdb_backdoor_flags.display_rx = !shmem_data_p->cfgdb_backdoor_flags.display_rx;
            debug_flag_changed = TRUE;
            break;

        case 4:
            shmem_data_p->cfgdb_backdoor_flags.display_tick = !shmem_data_p->cfgdb_backdoor_flags.display_tick;
            debug_flag_changed = TRUE;
            break;

        case 5:
            BACKDOOR_MGR_Printf("Shadow memory starting address in DRAM: %p\r\n", &shmem_data_p->config_file_shadow);
            BACKDOOR_MGR_Printf("File level checksum: 0x%lx\r\n", (unsigned long)shmem_data_p->config_file_shadow.checksum);
            BACKDOOR_MGR_Printf("Configuration file version: 0x%lx\r\n", (unsigned long)shmem_data_p->config_file_shadow.version);
            BACKDOOR_MGR_Printf("Offset from the data to the head: %lu\r\n", (unsigned long)((UI8_T *)(shmem_data_p->config_file_shadow.section_data) - (UI8_T *)&shmem_data_p->config_file_shadow));

            BACKDOOR_MGR_Printf("Key in section handler <1-%lu>: ", (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler-1);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            section_handler = atoi((char *)key_in_string);

            BACKDOOR_MGR_Printf("Section ID: %lu\r\n",                 (unsigned long)shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_id);
            BACKDOOR_MGR_Printf("Section start offset: %lu\r\n",       (unsigned long)shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_start_offset);
            BACKDOOR_MGR_Printf("Section record size: %lu\r\n",        (unsigned long)shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_size);
            BACKDOOR_MGR_Printf("Section record number: %lu\r\n",      (unsigned long)shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_number);
            BACKDOOR_MGR_Printf("Section level checksum: 0x%08lx\r\n", (unsigned long)shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_checksum);
            break;

        case 6:
            BACKDOOR_MGR_Printf("Key in offset address <0-%lu>: ", (unsigned long)CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            start_offset = atoi((char *)key_in_string);

            BACKDOOR_MGR_Printf("Key in length <1-%lu>: ", (unsigned long)CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE-start_offset);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            length = atoi((char *)key_in_string);

            for(i=start_offset, time = 1; i<=start_offset+length-1; i++, time++)
            {
                BACKDOOR_MGR_Printf("%02x ", *(shmem_data_p->config_file_shadow.section_data+i) );

                if (time%16 == 0)
                {
                    BACKDOOR_MGR_Printf("\r\n");
                }
            }
            break;

        case 7:
            BACKDOOR_MGR_Printf("Dirty: %s\r\n",                         shmem_data_p->local_sync_work_area.dirty ?            "TRUE" : "FALSE");
            BACKDOOR_MGR_Printf("Dirty timer: %lu\r\n",                  (unsigned long)shmem_data_p->local_sync_work_area.dirty_timer);
            BACKDOOR_MGR_Printf("Max dirty timer: %lu\r\n",              (unsigned long)shmem_data_p->local_sync_work_area.max_dirty_timer);
            BACKDOOR_MGR_Printf("Allow to open: %s\r\n",                 shmem_data_p->cfgdb_mgr_allow_to_open ?               "TRUE" : "FALSE");
            BACKDOOR_MGR_Printf("Run timer: %s\r\n",                     shmem_data_p->cfgdb_mgr_run_timer ?                   "TRUE" : "FALSE");
            BACKDOOR_MGR_Printf("Available section handler: %lu\r\n",    (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler);
            BACKDOOR_MGR_Printf("Available offset: %lu\r\n",             (unsigned long)shmem_data_p->local_sync_work_area.available_offset);
            break;

        case 8:
            for(unit=1; unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
            {
                if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit, shmem_data_p->cfgdb_mgr_existing_units))
                {
                    continue;
                }

                if (unit == shmem_data_p->cfgdb_mgr_my_unit_id)
                {
                    continue;
                }

                BACKDOOR_MGR_Printf("\r\nUnit %lu: \r\n", (unsigned long)unit);

                for(block=0, time = 1; block<CFGDB_TYPE_DIRTY_BLOCK_NUMBER; block++, time++)
                {
                    BACKDOOR_MGR_Printf("%d ", (int)shmem_data_p->remote_sync_work_area.dirty[unit-1][block]);

                    if (time%10 == 0)
                    {
                        BACKDOOR_MGR_Printf("\r\n");
                    }
                }
            }
            break;

        case 9:
            BACKDOOR_MGR_Printf("Shadow memory starting address in DRAM: %p\r\n", &shmem_data_p->internal_config_file_shadow);
            BACKDOOR_MGR_Printf("File level checksum: 0x%lx\r\n", (unsigned long)shmem_data_p->internal_config_file_shadow.checksum);
            BACKDOOR_MGR_Printf("Configuration file version: 0x%lx\r\n", (unsigned long)shmem_data_p->internal_config_file_shadow.version);
            BACKDOOR_MGR_Printf("Offset from the data to the head: %lu\r\n", (unsigned long)((UI8_T *)(shmem_data_p->internal_config_file_shadow.section_data) - (UI8_T *)&shmem_data_p->internal_config_file_shadow));

            BACKDOOR_MGR_Printf("Key in section handler <1-%lu>: ", (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler-1);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            section_handler = atoi((char *)key_in_string);

            BACKDOOR_MGR_Printf("Section ID: %lu\r\n",                 (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_id);
            BACKDOOR_MGR_Printf("Section start offset: %lu\r\n",       (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_start_offset);
            BACKDOOR_MGR_Printf("Section record size: %lu\r\n",        (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_size);
            BACKDOOR_MGR_Printf("Section record number: %lu\r\n",      (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_number);
            BACKDOOR_MGR_Printf("Section level checksum: 0x%08lx\r\n", (unsigned long)shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_checksum);
            break;

        case 10:
            BACKDOOR_MGR_Printf("Key in offset address <0-%lu>: ", (unsigned long)CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            start_offset = atoi((char *)key_in_string);

            BACKDOOR_MGR_Printf("Key in length <1-%lu>: ", (unsigned long)CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE-start_offset);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            length = atoi((char *)key_in_string);

            for(i=start_offset, time = 1; i<=start_offset+length-1; i++, time++)
            {
                BACKDOOR_MGR_Printf("%02x ", *(shmem_data_p->internal_config_file_shadow.section_data+i) );

                if (time%16 == 0)
                {
                    BACKDOOR_MGR_Printf("\r\n");
                }
            }
            break;




        case 11:
            BACKDOOR_MGR_Printf("Dirty: %s\r\n",                         shmem_data_p->internal_local_sync_work_area.dirty ?            "TRUE" : "FALSE");
            BACKDOOR_MGR_Printf("Dirty timer: %lu\r\n",                  (unsigned long)shmem_data_p->internal_local_sync_work_area.dirty_timer);
            BACKDOOR_MGR_Printf("Max dirty timer: %lu\r\n",              (unsigned long)shmem_data_p->internal_local_sync_work_area.max_dirty_timer);
            BACKDOOR_MGR_Printf("Allow to open: %s\r\n",                 shmem_data_p->cfgdb_mgr_allow_to_open ?               "TRUE" : "FALSE");
            BACKDOOR_MGR_Printf("Run timer: %s\r\n",                     shmem_data_p->cfgdb_mgr_run_timer ?                   "TRUE" : "FALSE");
            BACKDOOR_MGR_Printf("Available section handler: %lu\r\n",    (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler);
            BACKDOOR_MGR_Printf("Available offset: %lu\r\n",             (unsigned long)shmem_data_p->internal_local_sync_work_area.available_offset);
            break;

        case 12:
            for(unit=1; unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
            {
                if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit, shmem_data_p->cfgdb_mgr_existing_units))
                {
                    continue;
                }

                if (unit == shmem_data_p->cfgdb_mgr_my_unit_id)
                {
                    continue;
                }

                BACKDOOR_MGR_Printf("\r\nUnit %lu: \r\n", (unsigned long)unit);

                for(block=0, time = 1; block<CFGDB_TYPE_DIRTY_BLOCK_NUMBER; block++, time++)
                {
                    BACKDOOR_MGR_Printf("%d ", (int)shmem_data_p->internal_remote_sync_work_area.dirty[unit-1][block]);

                    if (time%10 == 0)
                    {
                        BACKDOOR_MGR_Printf("\r\n");
                    }
                }
            }
            break;

        case 13:
            BACKDOOR_MGR_Printf("Key in section ID <1-%lu>: ", (unsigned long)SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            section_id = atoi((char *)key_in_string);

            is_got = FALSE;
            for(section_handler=1; section_handler<=SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION; section_handler++)
            {
                if(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_id == section_id)
                {
                    is_external = TRUE;
                    is_got = TRUE;
                    break;
                }
            }

            if (FALSE == is_got)
            {
                for(section_handler=1; section_handler<=SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION; section_handler++)
                {
                    if(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_id == section_id)
                    {
                        is_external = FALSE;
                        is_got = TRUE;
                        break;
                    }
                }
            }

            if (TRUE == is_got)
            {
                BACKDOOR_MGR_Printf("Got section handler: %lu, in %s shadow.\r\n", (unsigned long)section_handler, is_external ? "external" : "internal");
            }
            else
            {
                BACKDOOR_MGR_Printf("This section ID is not opened.\r\n");
            }
            break;

        case 14:
            BACKDOOR_MGR_Printf("Key in section handler <1-%lu>: ", (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler-1);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            section_handler = atoi((char *)key_in_string);

            is_got = FALSE;
            if (0 != shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_id)
            {
                section_id = shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_id;
                is_external = TRUE;
                is_got = TRUE;
            }
            else if (0 != shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_id)
            {
                section_id = shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_id;
                is_external = FALSE;
                is_got = TRUE;
            }

            if (TRUE == is_got)
            {
                BACKDOOR_MGR_Printf("Got section ID: %lu, in %s shadow\r\n", (unsigned long)section_id, is_external ? "external" : "internal");
                BACKDOOR_MGR_Printf("Section type: %s.\r\n", shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1] == CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  ? "[External and Local]"  :
                                               (shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1] == CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL ? "[External and Global]" :
                                               (shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1] == CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL  ? "[Internal and Local]"  :
                                                                                                                                                "[Internal and Global]" )));
            }
            else
            {
                BACKDOOR_MGR_Printf("No such section handler: %lu, maybe CFGBD bug.\r\n", (unsigned long)section_handler);
            }
            break;

        case 15:
            if (FS_RETURN_OK == FS_DeleteFile(DUMMY_DRIVE, (UI8_T *)CFGDB_MGR_BINARY_CFG_FILE_NAME))
            {
                BACKDOOR_MGR_Printf("Success to delete external CFGDB file\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("Failed to delete external CFGDB file\r\n");
            }
            break;

        case 16:
            if (FS_RETURN_OK == FS_DeleteFile(DUMMY_DRIVE, (UI8_T *)CFGDB_MGR_INTERNAL_BINARY_CFG_FILE_NAME))
            {
                BACKDOOR_MGR_Printf("Success to delete internal CFGDB file\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("Failed to delete internal CFGDB file\r\n");
            }
            break;




        case 17:
            BACKDOOR_MGR_Printf("Key in section handler <1-%lu>: ", (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler-1);
            memset(key_in_string, 0, sizeof(key_in_string));
            BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
            BACKDOOR_MGR_Printf("\r\n");
            section_handler = atoi((char *)key_in_string);

            if (section_handler == 0 || section_handler >= shmem_data_p->cfgdb_mgr_available_section_handler)
            {
                BACKDOOR_MGR_Printf("Invalid section handler.\r\n");
            }
            else
            {
                UI8_T *buffer_p;

                if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
                    (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]))
                {
                    BACKDOOR_MGR_Printf("Key in start offset <0-%lu>: ", (unsigned long)(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_number)-1);
                    memset(key_in_string, 0, sizeof(key_in_string));
                    BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
                    BACKDOOR_MGR_Printf("\r\n");
                    start_offset = atoi((char *)key_in_string);

                    BACKDOOR_MGR_Printf("Key in length <1-%lu>: ",       (unsigned long)(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_number) - start_offset);
                    memset(key_in_string, 0, sizeof(key_in_string));
                    BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
                    BACKDOOR_MGR_Printf("\r\n");
                    length = atoi((char *)key_in_string);
                }
                else
                {
                    BACKDOOR_MGR_Printf("Key in start offset <0-%lu>: ", (unsigned long)(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_number)-1);
                    memset(key_in_string, 0, sizeof(key_in_string));
                    BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
                    BACKDOOR_MGR_Printf("\r\n");
                    start_offset = atoi((char *)key_in_string);

                    BACKDOOR_MGR_Printf("Key in length <1-%lu>: ",       (unsigned long)(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_number) - start_offset);
                    memset(key_in_string, 0, sizeof(key_in_string));
                    BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);
                    BACKDOOR_MGR_Printf("\r\n");
                    length = atoi((char *)key_in_string);
                }

                if (NULL == (buffer_p = calloc(length, sizeof(UI8_T))))
                {
                    BACKDOOR_MGR_Printf("Failed to allocate memory.\r\n");
                }
                else
                {
                    if (TRUE == CFGDB_MGR_Read(section_handler, buffer_p, start_offset, length))
                    {
                        for(i=0, time=1; i<=length-1; i++, time++)
                        {
                            BACKDOOR_MGR_Printf("%02x ", *(buffer_p+i) );

                            if (time%16 == 0)
                            {
                                BACKDOOR_MGR_Printf("\r\n");
                            }
                        }
                    }
                    else
                    {
                        BACKDOOR_MGR_Printf("Failed to read CFGDB.\r\n");
                    }
                    free(buffer_p);
                }
            }
            break;

        case 18:
            BACKDOOR_MGR_Printf("Key in section handler <1-%lu>: ", (unsigned long)shmem_data_p->cfgdb_mgr_available_section_handler-1);
            memset(key_in_string, 0, sizeof(key_in_string)); BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);  BACKDOOR_MGR_Printf("\r\n");
            section_handler = atoi((char *)key_in_string);

            if (section_handler == 0 || section_handler >= shmem_data_p->cfgdb_mgr_available_section_handler)
            {
                BACKDOOR_MGR_Printf("Invalid section handler.\r\n");
            }
            else
            {
                UI8_T  *buffer_p;

                if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]) ||
                    (CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == shmem_data_p->cfgdb_mgr_section_type_table[section_handler-1]))
                {
                    BACKDOOR_MGR_Printf("Key in start offset <0-%lu>: ", (unsigned long)(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_number) - 1);
                    memset(key_in_string, 0, sizeof(key_in_string)); BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);  BACKDOOR_MGR_Printf("\r\n");
                    start_offset = atoi((char *)key_in_string);

                    BACKDOOR_MGR_Printf("Key in length <1-%lu>: ",       (unsigned long)(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->config_file_shadow.descriptor[section_handler-1].section_record_number) - start_offset);
                    memset(key_in_string, 0, sizeof(key_in_string)); BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);  BACKDOOR_MGR_Printf("\r\n");
                    length = atoi((char *)key_in_string);
                }
                else
                {
                    BACKDOOR_MGR_Printf("Key in start offset <0-%lu>: ", (unsigned long)(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_number) - 1);
                    memset(key_in_string, 0, sizeof(key_in_string)); BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);  BACKDOOR_MGR_Printf("\r\n");
                    start_offset = atoi((char *)key_in_string);

                    BACKDOOR_MGR_Printf("Key in length <1-%lu>: ",       (unsigned long)(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_size)*(shmem_data_p->internal_config_file_shadow.descriptor[section_handler-1].section_record_number) - start_offset);
                    memset(key_in_string, 0, sizeof(key_in_string)); BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);  BACKDOOR_MGR_Printf("\r\n");
                    length = atoi((char *)key_in_string);
                }

                if (NULL == (buffer_p = calloc(length, sizeof(UI8_T))))
                {
                    BACKDOOR_MGR_Printf("Failed to allocate memory.\r\n");
                }
                else
                {
                    UI32_T base;

                    BACKDOOR_MGR_Printf("This test program automatically write continuously integer (0-255) to buffer.\r\n");
                    BACKDOOR_MGR_Printf("Key in the base <0-255>: ");
                    memset(key_in_string, 0, sizeof(key_in_string)); BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 5);  BACKDOOR_MGR_Printf("\r\n");
                    base = atoi((char *)key_in_string);

                    for(i=0; i<=length-1; i++)
                    {
                        *(buffer_p+i) = ((i+base)%256);
                    }

                    if (FALSE == CFGDB_MGR_Write(section_handler, buffer_p, start_offset, length))
                    {
                        BACKDOOR_MGR_Printf("Failed to write CFGDB.\r\n");
                    }
                    free(buffer_p);
                }
            }
            break;

        case 19:
            if (FALSE == CFGDB_MGR_Flush())
            {
                BACKDOOR_MGR_Printf("Failed to flush shadow to FLASH.\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("Success to flush shadow to FLASH.\r\n");
            }
            break;


        case 20:
            CFGDB_MGR_DumpSectionAttributes();
            break;

        case 21:
            {
                int ch;

                BACKDOOR_MGR_Printf("Display level <0-3>: ");
                ch = getchar();
                if ((ch < '0')||(ch>'3'))
        	    {
        	        BACKDOOR_MGR_Printf("Out of unit range\r\n");
        	    }
                else
                {
                    ch = ch - '0';
                    BACKDOOR_MGR_Printf ("%d\r\n",ch);

                    shmem_data_p->cfgdb_backdoor_flags.display_dbg_msg_level = (int)ch;
                    debug_flag_changed = TRUE;
                }

            }
            break;

        case 22:
            shmem_data_p->cfgdb_backdoor_flags.disable_writing = !shmem_data_p->cfgdb_backdoor_flags.disable_writing;
            debug_flag_changed = TRUE;
            break;

        case 99:
            return;

        default:
            BACKDOOR_MGR_Printf("Invalid choice.\r\n");
            break;
        }
        if(debug_flag_changed)
        {
            CFGDB_UTIL_SetDebugFlags(&shmem_data_p->cfgdb_backdoor_flags);
        }
        BACKDOOR_MGR_Printf("\r\nPress ENTER to continue ");
        BACKDOOR_MGR_RequestKeyIn((char *)key_in_string, 0);
    }
    return;
}

