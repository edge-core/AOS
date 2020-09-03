/*-----------------------------------------------------------------------------
 * Module Name: CFGDB_UTIL.C
 *-----------------------------------------------------------------------------
 * PURPOSE: Utilities of this computer software component.
 *-----------------------------------------------------------------------------
 * NOTES:   None.
 *-----------------------------------------------------------------------------
 * HISTORY:
 *      07/17/2003 -- Charles Cheng, Created for change version to 0x01000002 for keep IP
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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "string.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_module.h"
#include "cfgdb_type.h"
#include "cfgdb_util.h"
#include "cfgdb_mgr.h"
#include "fs.h"
#include "fs_type.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "isc.h"
#include "iuc.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CFGDB_UTIL_POOL_ID_DEFAULT   0

/* STATIC VARIABLE DECLARATIONS
 */
static CFGDB_TYPE_BackdoorFlags_T   cfgdb_util_backdoor_flags;

static UI32_T                       cfgdb_util_sem_id;
static UI32_T                       original_priority;

#define CFGDB_UTIL_EnterCriticalSection()   SYSFUN_TakeSem(cfgdb_util_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define CFGDB_UTIL_LeaveCriticalSection()   SYSFUN_GiveSem(cfgdb_util_sem_id)

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_SetDebugFlags
 *-------------------------------------------------------------------------
 * PURPOSE: This function is used tell CFGDB_UTIL about the pointer of the
 *          backdoor flags.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE      -- Data is correct.
 *          FALSE     -- Data is not correct.
 * NOTES:   If the length is multiple of 4, stub viewed as 0.
 *-------------------------------------------------------------------------
 */
void CFGDB_UTIL_SetDebugFlags(CFGDB_TYPE_BackdoorFlags_T *backdoor_flags_ptr)
{
    memcpy(&cfgdb_util_backdoor_flags, backdoor_flags_ptr, sizeof(CFGDB_TYPE_BackdoorFlags_T));
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_SetSemID
 *-------------------------------------------------------------------------
 * PURPOSE: This function is used tell CFGDB_UTIL semaphore id
 * INPUT:   sem_id
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:   None
 *-------------------------------------------------------------------------
 */
void CFGDB_UTIL_SetSemID(UI32_T sem_id)
{
    cfgdb_util_sem_id = sem_id;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_Check32BitChecksum
 *-------------------------------------------------------------------------
 * PURPOSE: This function is used to do 32-bit check sum
 * INPUT:   data      -- Data to check.
 *          length       -- Length of data to check.
 *          checksum -- Checksum used to check data.
 * OUTPUT:  None.
 * RETUEN:  TRUE      -- Data is correct.
 *          FALSE     -- Data is not correct.
 * NOTES:   If the length is multiple of 4, stub viewed as 0.
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_Check32BitChecksum(UI8_T *data, UI32_T length, UI32_T checksum)
{
    UI8_T  *run_data;
    UI32_T run_len;
    UI32_T i;
    UI32_T sumation;
    BOOL_T retval;

    if (0 == length)
    {
        return FALSE;
    }

    if (length%4 != 0)
    {
        run_len = length + (4 - (length%4));
    }
    else
    {
        run_len = length;
    }

    if (NULL == (run_data = calloc(run_len, sizeof(UI8_T))))
    {
        return FALSE;
    }

    memcpy(run_data, data, length);

    sumation = 0;
    for(i=0; i<(run_len/4); i++)
    {
        sumation += *((UI32_T *)(run_data + (i*4)));
    }

    /* if sumation of all 32-bit values are zero, TRUE
     */
    if (0==(sumation + checksum))
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    free(run_data);
    return retval;

} /* CFGDB_MGR_Check32BitChecksum */

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_Generate32BitChecksum
 *-------------------------------------------------------------------------
 * PURPOSE: This function is used to generate 32-bit check sum
 * INPUT:   data      -- Data to generate.
 *          length       -- Length of data to generate.
 * OUTPUT:  checksum -- Generated Checksum from data.
 * RETUEN:  TRUE      -- Data is correct.
 *          FALSE     -- Data is not correct.
 * NOTES:   If the length is multiple of 4, stub viewed as 0.
 *-------------------------------------------------------------------------
 */
void CFGDB_UTIL_Generate32BitChecksum(UI8_T *data, UI32_T length, UI32_T *checksum)
{
    UI8_T  *run_data;
    UI32_T run_len;
    UI32_T i;

    *checksum = 0;

    if (0 == length)
    {
        return;
    }

    if (length%4 != 0)
    {
        run_len = length + (4 - (length%4));
    }
    else
    {
        run_len = length;
    }

    if (NULL == (run_data = calloc(run_len, sizeof(UI8_T))))
    {
        return;
    }

    memcpy(run_data, data, length);

    for(i=0; i<(run_len/4); i++)
    {
        *checksum += *((UI32_T *)(run_data + (i*4)));
    }

    /* to make sumation of all 32-bit values are zero.
     */
    *checksum = 0 - *checksum;

    free(run_data);
    return;

} /* CFGDB_MGR_Generate32BitChecksum */

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_WriteDummyBinConfigToFlash
 *-------------------------------------------------------------------------
 * PURPOSE: Write a dummy binary configuration file to FLASH, copy to
 *          shadow buffer.
 * INPUT:   version       -- version of this binary configuration file.
 *          filename      -- filename to write to FLASH.
 *          showdow_buf_p -- shadow to write.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:   In init time, if
 *          1) no binary configuration file in FLASH,
 *          2) file level checksum error, and
 *          3) CFGDB version change.
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_WriteDummyBinConfigToFlash(UI32_T version, UI8_T *filename, CFGDB_TYPE_ConfigFile_T *showdow_buf_p)
{
    /* version of this binary configuration file
     */

    showdow_buf_p->version = version;

    /* All section_id will be set to 0, that is invalid, and could be used to indicate that no section exists.
     */

    memset(showdow_buf_p->descriptor, 0, sizeof(showdow_buf_p->descriptor));

    /* All data is clean as dummy
     */

    memset(showdow_buf_p->section_data, 0, CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE);

    /* generate file level checksum
     */

    CFGDB_UTIL_Generate32BitChecksum(((UI8_T *)showdow_buf_p + sizeof(showdow_buf_p->checksum)),
                                    SYS_ADPT_BINARY_CFG_FILE_SIZE - sizeof(showdow_buf_p->checksum),
                                    &(showdow_buf_p->checksum));

    if (FS_RETURN_OK != FS_WriteFile (DUMMY_DRIVE,
                                      filename,
                                      (UI8_T *)CFGDB_TYPE_FS_COMMENT,
                                      FS_FILE_TYPE_BINARY_CONFIG,
                                      (UI8_T *)showdow_buf_p,
                                      SYS_ADPT_BINARY_CFG_FILE_SIZE,
                                      SYS_ADPT_BINARY_CFG_FILE_SIZE))
    {

        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_InitLocalSyncWorkingArea
 *-------------------------------------------------------------------------
 * PURPOSE: To init local working area.
 * INPUT:   local_sync_work_area -- working area to init.
 *          The operation mode of this unit.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_InitLocalSyncWorkingArea(CFGDB_TYPE_LocalSyncWorkArea_T *local_sync_work_area,
                                           SYS_TYPE_Stacking_Mode_T       mode)
{
    switch(mode)
    {
    case SYS_TYPE_STACKING_MASTER_MODE:
        local_sync_work_area->dirty            = FALSE;
        local_sync_work_area->dirty_timer      = 0;
        local_sync_work_area->max_dirty_timer  = 0;
        local_sync_work_area->available_offset = 0;
        local_sync_work_area->availible_index  = 1;
        break;

    case SYS_TYPE_STACKING_SLAVE_MODE:
        local_sync_work_area->dirty           = FALSE;
        local_sync_work_area->dirty_timer     = 0;
        local_sync_work_area->max_dirty_timer = 0;

        /* the other fields don't mean any thing in slave mode
         */
        break;

    default:
        return FALSE;
    }


    return TRUE;
}



/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_InitConfigFileShadow
 *-------------------------------------------------------------------------
 * PURPOSE: To init a file shadow.
 * INPUT:   filename -- The file name in FLASH.
 *          config_file_shadow -- The shadow to init.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_InitConfigFileShadow( UI8_T                       *filename,
                                        CFGDB_TYPE_ConfigFile_T     *config_file_shadow,
                                        SYS_TYPE_Stacking_Mode_T    mode)
{
    UI32_T flash_data_count;
    UI32_T read_flash_result;
    BOOL_T file_checksum_result;
    BOOL_T is_version_changed = FALSE;
    UI32_T sec_hd;

    /* init shadow buffer of binary config file as what in FLASH
     */
    /* Read data from FLASH. The job here are for
     *  1. reading FLASH to init_descriptor (section data will sync when open)
     *  2. checking binary configuration file exists or not
     *  3. checking file level checksum
     *  4. checking version number
     */

     read_flash_result = FS_ReadFile(DUMMY_DRIVE,
                                     filename,
                                     (UI8_T *)config_file_shadow,
                                     SYS_ADPT_BINARY_CFG_FILE_SIZE,
                                     &flash_data_count);


    if (FS_RETURN_OK == read_flash_result)
    {

        file_checksum_result = CFGDB_UTIL_Check32BitChecksum(((UI8_T *)config_file_shadow + sizeof(config_file_shadow->checksum)),
                                                            SYS_ADPT_BINARY_CFG_FILE_SIZE  - sizeof(config_file_shadow->checksum),
                                                            config_file_shadow->checksum);

        if (TRUE == file_checksum_result)
        {
            is_version_changed = (config_file_shadow->version != CFGDB_TYPE_VERSION);
        }
    }


    if (FS_RETURN_OK != read_flash_result    ||
        FALSE        == file_checksum_result ||
        TRUE         == is_version_changed   )
    {
#if 0
        printf("\r\nCFGDB: \r\n");
        printf("  1. (%c) failed to read from FLASH when system init,\r\n", (read_flash_result != FS_RETURN_OK) ? 'O' : 'X');
        printf("  2. (%c) file level checksum error, or\r\n", (file_checksum_result == FALSE) ? 'O' : 'X');
        printf("  3. (%c) binary configuration file version changed,\r\n", (is_version_changed == TRUE) ? 'O' : 'X' );
        printf("  Create a dummy binary config file and write to FLASH...");
#endif


        if (TRUE == CFGDB_UTIL_WriteDummyBinConfigToFlash(CFGDB_TYPE_VERSION,
                                                          filename,
                                                          config_file_shadow))
        {

            /*
            printf("...done.\n\r");
             */
        }
        else
        {

            /*
            printf("...failed.\n\r");
             */
            return FALSE;
        }
    }



    /* keep version
     */
    config_file_shadow->version = CFGDB_TYPE_VERSION;

    switch(mode)
    {
    case SYS_TYPE_STACKING_MASTER_MODE:

        /* clean up data body, these will be read from FLASH or set default in CFGDB_MFG_Open()
         */
        memset(config_file_shadow->section_data, 0, CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE);

        /* will be generated in CFGDB_MGR_EndOfOpen()
         */
        config_file_shadow->checksum = 0;
        break;

    case SYS_TYPE_STACKING_SLAVE_MODE:

        for(sec_hd = 1; sec_hd<=SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION; sec_hd++)
        {
            if (0 != config_file_shadow->descriptor[sec_hd-1].section_id)
            {

                if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == config_file_shadow->descriptor[sec_hd-1].section_type) ||
                    (CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL == config_file_shadow->descriptor[sec_hd-1].section_type) )
                {
                    /* global sections will be synced from master, so removed here
                     */

                    memset(((UI8_T *)(config_file_shadow->section_data))+config_file_shadow->descriptor[sec_hd-1].section_start_offset,
                           0,
                           config_file_shadow->descriptor[sec_hd-1].section_record_size*config_file_shadow->descriptor[sec_hd-1].section_record_number);

                    memset((UI8_T *)(&(config_file_shadow->descriptor[sec_hd-1])), 0, sizeof(CFGDB_TYPE_SectionDescriptor_T));
                }
            }
        }



        CFGDB_UTIL_Generate32BitChecksum((UI8_T *)config_file_shadow + sizeof(config_file_shadow->checksum),
                                         SYS_ADPT_BINARY_CFG_FILE_SIZE - sizeof(config_file_shadow->checksum),
                                         &(config_file_shadow->checksum));
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_OpenInfoSync
 *-------------------------------------------------------------------------
 * PURPOSE: When a section is opened, use this routine to set related info.
 * INPUT:   section_id -- section ID.
 *          record_size  -- Record size the section want to use.
 *          record_number -- Record number the section want to use.
 *          available_section_handler -- Current available section handler.
 *          local_sync_work_area -- Local working area.
 *          init_descriptor -- section descriptor read from FLASH.
 *          config_file_shadow -- File shadow.
 *          need_default_table -- Does this section need to sync dtat in
 *          the FLASH and default table.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
UI32_T CFGDB_UTIL_OpenInfoSync (UI32_T                          section_id,
                                UI32_T                          record_size,
                                UI32_T                          record_number,
                                UI32_T                          available_section_handler,
                                UI32_T                          section_type,
                                UI8_T                           *filename,
                                CFGDB_TYPE_LocalSyncWorkArea_T  *local_sync_work_area,
                                CFGDB_TYPE_SectionDescriptor_T  init_descriptor[],
                                CFGDB_TYPE_ConfigFile_T         *config_file_shadow,
                                BOOL_T                          *need_default_table)
{
    UI32_T old_index;
    UI8_T  *flash_section_data_p;     /*using memory allocate instead of declaring array to prevent from buffer overflow*/
    BOOL_T is_section_there = FALSE;
    UI32_T read_flash_result = 0;
    BOOL_T section_checksum_result = FALSE;
    BOOL_T record_size_same = FALSE;
    UI32_T section_handler;
    UI32_T checksum;

    /* allocate maxium possible section size
     */
    if (NULL == (flash_section_data_p = calloc(SYS_ADPT_BINARY_CFG_FILE_SIZE, sizeof(UI8_T)))) /*maybe smaller*/
    {
        SYSFUN_Debug_Printf("\n\rCFGDB: Failed to allocate memory when open section.\n\r");
        return 0;
    }

    /* record_size and record_number checking for per file checking
     */
    if (record_size*record_number > (CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE - local_sync_work_area->available_offset))
    {
        SYSFUN_Debug_Printf("\n\rCFGDB: SYS_ADPT_BINARY_CFG_FILE_SIZE is too small (2).\n\r");
        free(flash_section_data_p);
        return 0;
    }

    /* init flag as FALSE
     */
    is_section_there = FALSE;
    *need_default_table = FALSE;

    /* search existed section id
     */
    for(old_index=SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION; old_index<=SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION; old_index++)
    {
        if (init_descriptor[old_index-1].section_id == section_id)
        {
            /* section is there,
             * and the old_index now could be used to access related infomation in init_descriptor,
             * e.g. offset, length, ...
             */
            is_section_there = TRUE;

            if (old_index != available_section_handler)
            {
                /* position is changed, will write to FLASH,
                 * maybe some CSC didn't enter master mode, and
                 * the data of this CSC will shift position in the FLASH
                 */
                local_sync_work_area->dirty = TRUE;
            }
            break;
        }
    }

    if (TRUE == is_section_there)
    {
        /* read ONLY section data from FLASH
         */
        if (FS_RETURN_OK == (read_flash_result = FS_CopyFileContent(DUMMY_DRIVE,
                                                                    filename,
                                                                    CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE+init_descriptor[old_index-1].section_start_offset,
                                                                    flash_section_data_p,
                                                                    init_descriptor[old_index-1].section_record_size*init_descriptor[old_index-1].section_record_number)))
        {
            /* do checksum
             */
            if (TRUE == (section_checksum_result = CFGDB_UTIL_Check32BitChecksum(flash_section_data_p,
                                                                                init_descriptor[old_index-1].section_record_size*init_descriptor[old_index-1].section_record_number,
                                                                                init_descriptor[old_index-1].section_checksum)))
            {
                if (TRUE == (record_size_same = (record_size == init_descriptor[old_index-1].section_record_size)))
                {
                    if (record_number < init_descriptor[old_index-1].section_record_number)
                    {
                        memcpy((UI8_T *)(config_file_shadow->section_data) + local_sync_work_area->available_offset,
                               flash_section_data_p,
                               record_size*record_number);

                        local_sync_work_area->dirty = TRUE;
                    }
                    else if (record_number == init_descriptor[old_index-1].section_record_number)
                    {
                        memcpy((UI8_T *)(config_file_shadow->section_data) + local_sync_work_area->available_offset,
                               flash_section_data_p,
                               record_size*record_number);
                    }
                    else /* if (record_number > init_descriptor[old_index-1].section_record_number) */
                    {
                        memcpy((UI8_T *)(config_file_shadow->section_data) + local_sync_work_area->available_offset,
                               flash_section_data_p,
                               record_size*init_descriptor[old_index-1].section_record_number);

                        /* Mercury_V2 version will not have default table concept, make utility no change, move this code in MGR
                         */
                        /*
                        memcpy((UI8_T *)(config_file_shadow->section_data) + local_sync_work_area->available_offset + record_size*init_descriptor[old_index-1].section_record_number,
                               default_table + record_size*init_descriptor[old_index-1].section_record_number,
                               record_size*(record_number - init_descriptor[old_index-1].section_record_number));
                         */
                        *need_default_table = TRUE;

                        local_sync_work_area->dirty = TRUE;
                    }
                }
            }
        }
    }
    free(flash_section_data_p);


    if (FALSE        == is_section_there        ||
        FS_RETURN_OK != read_flash_result       ||
        FALSE        == section_checksum_result ||
        FALSE        == record_size_same        )
    {
        /* 1. For new section, it's first time for CSC to use CFGDB,
         *    think that if this is the first time that CSC use CFGDB, i.e. there is no data in CFGDB.
         *
         * 2. For the old section, but the data is FLASH cannot be read out, suppose data is crashed.
         *    because data in FLASH is crashed, i.e. there is no data in CFGDB.
         *
         * 3. For the old section, and the data is FLASH can be read out, but checksum failed
         *    suppose data is crashed. because data in FLASH is crashed, i.e. there is no data in CFGDB.
         *
         * 4. For the section that record size changed, config file in FLASH don't mean any thing.
         *
         *    After CSC open section and then CSC read section from CFGDB, something will be wrong.
         *    For CSC to be generic to read CFGDB, providing default setting when section opened is
         *    always necessary.
         *
         *    Trun on dirty flag to make data write to FLASH, and write the default value provided by CSC
         *    to shadow.
         */
#if 0
        printf("\n\rCFGDB: Section ID: %lu\n\r", section_id);
        printf("  1. (%c) is a new section (or section type is changed),\n\r",  FALSE           == is_section_there ?           'O':'X');
        printf("  2. (%c) failed to read FLASH data,\n\r",                      FS_RETURN_OK    != read_flash_result ?          'O':'X');
        printf("  3. (%c) record size changed, or\n\r",                         FALSE           == record_size_same ?           'O':'X');
        printf("  4. (%c) file level checksum error.\n\r",                      FALSE           == section_checksum_result ?    'O':'X');
        printf("  Set default setting as data in shadow.\n\r");
#endif
        /* Mercury_V2 version will not have default table concept, make utility no change, move this code in MGR
         */
        /*
        memcpy((UI8_T *)(config_file_shadow->section_data) + local_sync_work_area->available_offset,
               default_table,
               record_size*record_number);
        */
        *need_default_table = TRUE;
        local_sync_work_area->dirty = TRUE;
    }


    /* assign the section handler to CSC
     */
    section_handler = available_section_handler;

    /* keep the data about descriptor in shadow buffer
     */
    /* generate section level checksum
     */
    CFGDB_UTIL_Generate32BitChecksum(((UI8_T *)(config_file_shadow->section_data) + local_sync_work_area->available_offset),
                                    record_size*record_number,
                                    &checksum);

    config_file_shadow->descriptor[section_handler-1].section_id            = section_id;
    config_file_shadow->descriptor[section_handler-1].section_start_offset  = local_sync_work_area->available_offset;
    config_file_shadow->descriptor[section_handler-1].section_checksum      = checksum; /*section level*/
    config_file_shadow->descriptor[section_handler-1].section_record_size   = record_size;
    config_file_shadow->descriptor[section_handler-1].section_record_number = record_number;
    config_file_shadow->descriptor[section_handler-1].section_type          = section_type;

    local_sync_work_area->available_offset += record_size*record_number;
    local_sync_work_area->availible_index++;

    return section_handler;
}




/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_SyncData
 *-------------------------------------------------------------------------
 * PURPOSE: The Sync the data between CSC default table and data in the FLASH.
 * INPUT:   section_handler -- section handler.
 *          init_time_descriptor -- The descriptior of this section read from FLASH.
 *          config_file_shadow --- File shadow.
 *          data_buffer -- The buffer to sync.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:   Need to sync only when
 *          1. new section:                 whole
 *          2. failed to read from FALSH:   whole
 *          3. checksum error:              whole
 *          4. record size changed:         whole
 *          5. record number enlarged:      append default to the tail
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_SyncData(UI32_T                           section_handler,
                           CFGDB_TYPE_SectionDescriptor_T   *init_time_descriptor,
                           CFGDB_TYPE_ConfigFile_T          *config_file_shadow,
                           UI8_T                            *data_buffer)
{
    UI32_T sec_hd;
    BOOL_T is_found;
    UI32_T fs_rec_size;
    UI32_T fs_rec_num;
    UI32_T checksum;

    if (config_file_shadow->descriptor[section_handler-1].section_id == 0)
    {
        return FALSE;
    }

    is_found = FALSE;
    for(sec_hd = 1; sec_hd<=SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION; sec_hd++)
    {
        if (init_time_descriptor[sec_hd-1].section_id == config_file_shadow->descriptor[section_handler-1].section_id)
        {
            /* fs_rec_size and fs_rec_num are used for "append" case
             */
            fs_rec_size = init_time_descriptor[sec_hd-1].section_record_size;
            fs_rec_num  = init_time_descriptor[sec_hd-1].section_record_number;
            is_found = TRUE;
            break;
        }
    }

    if ((FALSE == is_found) ||
        (config_file_shadow->descriptor[section_handler-1].section_record_size   != fs_rec_size) ||
        (config_file_shadow->descriptor[section_handler-1].section_record_number >  fs_rec_num))
    {
        UI32_T record_size;
        UI32_T record_number;

        record_size = config_file_shadow->descriptor[section_handler-1].section_record_size;
        record_number = config_file_shadow->descriptor[section_handler-1].section_record_number;

        /* whole: use whole default table as shadow
         */
        memcpy((UI8_T *)(config_file_shadow->section_data) + config_file_shadow->descriptor[section_handler-1].section_start_offset,
               data_buffer,
               record_size*record_number);

        /* generate chacksum
         */
        CFGDB_UTIL_Generate32BitChecksum(((UI8_T *)(config_file_shadow->section_data) + config_file_shadow->descriptor[section_handler-1].section_start_offset),
                                        record_size*record_number,
                                        &checksum);
    }
    else
    {
        /* append to shadow
         */
         memcpy((UI8_T *)(config_file_shadow->section_data) + config_file_shadow->descriptor[section_handler-1].section_start_offset + fs_rec_size*fs_rec_num,
                data_buffer + fs_rec_size*fs_rec_num,
                fs_rec_size*(config_file_shadow->descriptor[section_handler-1].section_record_number - fs_rec_num));

        /* generate chacksum
         */
        CFGDB_UTIL_Generate32BitChecksum(((UI8_T *)(config_file_shadow->section_data) + config_file_shadow->descriptor[section_handler-1].section_start_offset),
                                        config_file_shadow->descriptor[section_handler-1].section_record_size*config_file_shadow->descriptor[section_handler-1].section_record_number,
                                        &checksum);
        /* also output to caller
         */
        memcpy(data_buffer,
               (UI8_T *)(config_file_shadow->section_data) + config_file_shadow->descriptor[section_handler-1].section_start_offset,
               fs_rec_size*fs_rec_num);
    }

    config_file_shadow->descriptor[section_handler-1].section_checksum = checksum;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_ReOrgSections
 *-------------------------------------------------------------------------
 * PURPOSE: Re-orgnize the sections
 * INPUT:   config_file_shadow --- File shadow.
 *          available_section_handler -- how much section need to parse.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_ReOrgSections( CFGDB_TYPE_ConfigFile_T        *config_file_shadow,
                                 UI32_T                         available_section_handler,
                                 BOOL_T                         filter_out_global,
                                 BOOL_T                         *content_changed,
                                 UI32_T                         *available_offset)
{
    CFGDB_TYPE_ConfigFile_T  *swap_config_file;
    UI32_T section_handler;
    UI32_T global_offset;
    UI32_T local_offset;


    if (NULL == (swap_config_file = (CFGDB_TYPE_ConfigFile_T *)calloc(SYS_ADPT_BINARY_CFG_FILE_SIZE, sizeof(UI8_T))))
    {
        SYSFUN_Debug_Printf("\n\rCFGDB: Allocating swap config file shadow memory failed.\n\r");
        assert(0);
        return FALSE;
    }

    /* to move "local" section prior to "global" section
     */
    local_offset  = CFGDB_TYPE_LOCAL_SECTION_STARTING_OFFSET;
    global_offset = CFGDB_TYPE_GLOBAL_SECTION_STARTING_OFFSET;

    for(section_handler=SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION;
        section_handler<=available_section_handler-1;
        section_handler++)
    {
        if (0 == config_file_shadow->descriptor[section_handler-1].section_id)
        {
            continue;
        }

        if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL == config_file_shadow->descriptor[section_handler-1].section_type) ||
            (CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL == config_file_shadow->descriptor[section_handler-1].section_type))
        {
            if (local_offset>=CFGDB_TYPE_GLOBAL_SECTION_STARTING_OFFSET)
            {
                free(swap_config_file);
                return FALSE;
            }

            /* descriptor
             */
            swap_config_file->descriptor[section_handler-1] = config_file_shadow->descriptor[section_handler-1];
            swap_config_file->descriptor[section_handler-1].section_start_offset = local_offset;

            /* data
             */
            memcpy( swap_config_file->section_data+local_offset,
                    config_file_shadow->section_data+config_file_shadow->descriptor[section_handler-1].section_start_offset,
                    (config_file_shadow->descriptor[section_handler-1].section_record_size)*(config_file_shadow->descriptor[section_handler-1].section_record_number));

            local_offset += (config_file_shadow->descriptor[section_handler-1].section_record_size)*(config_file_shadow->descriptor[section_handler-1].section_record_number);
        }
        else
        {
            if (TRUE == filter_out_global)
            {
                continue;
            }

            if (global_offset>=SYS_ADPT_BINARY_CFG_FILE_SIZE)
            {
                free(swap_config_file);
                return FALSE;
            }

            /* descriptor
             */
            swap_config_file->descriptor[section_handler-1] = config_file_shadow->descriptor[section_handler-1];
            swap_config_file->descriptor[section_handler-1].section_start_offset = global_offset;

            /* data
             */
            memcpy( swap_config_file->section_data+global_offset,
                    config_file_shadow->section_data+config_file_shadow->descriptor[section_handler-1].section_start_offset,
                    (config_file_shadow->descriptor[section_handler-1].section_record_size)*(config_file_shadow->descriptor[section_handler-1].section_record_number));

            global_offset += (config_file_shadow->descriptor[section_handler-1].section_record_size)*(config_file_shadow->descriptor[section_handler-1].section_record_number);
        }
    }

    *available_offset = global_offset;

    /* 1. version
     */
    swap_config_file->version = CFGDB_TYPE_VERSION;

    /* 2. checksum
     */
    CFGDB_UTIL_Generate32BitChecksum((UI8_T *)swap_config_file + sizeof(swap_config_file->checksum),
                                    SYS_ADPT_BINARY_CFG_FILE_SIZE - sizeof(swap_config_file->checksum),
                                    &(swap_config_file->checksum));



    if (0 != memcmp(config_file_shadow, swap_config_file, SYS_ADPT_BINARY_CFG_FILE_SIZE))
    {
        *content_changed = TRUE;
    }

    memcpy(config_file_shadow, swap_config_file, SYS_ADPT_BINARY_CFG_FILE_SIZE);
    free(swap_config_file);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_EndOfOpenFile
 *-------------------------------------------------------------------------
 * PURPOSE: To handle a file when "end of open".
 * INPUT:   local_sync_work_area -- Local sync working area.
 *          config_file_shadow --- File shadow.
 *          remote_sync_work_area --- Remote sync working area.
 *          existing_units --- Units in system.
 *          my_unit_id --- My unit ID.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 *-------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_EndOfOpenFile(CFGDB_TYPE_LocalSyncWorkArea_T  *local_sync_work_area,
                                CFGDB_TYPE_ConfigFile_T         *config_file_shadow,
                                CFGDB_TYPE_RemoteSyncWorkArea_T *remote_sync_work_area,
                                UI32_T                          available_section_handler,
                                UI32_T                          existing_units,
                                UI32_T                          my_unit_id)
{
    UI32_T start;
    UI32_T end;
    UI32_T unit_id;
    UI32_T block_id;
    BOOL_T content_changed;
    UI32_T section_handler;

    if (FALSE == CFGDB_UTIL_ReOrgSections(  config_file_shadow,
                                            available_section_handler,
                                            FALSE,
                                            &content_changed,
                                            &(local_sync_work_area->available_offset)))
    {
        return FALSE;
    }

    if (TRUE == content_changed)
    {
        /* note: this is not the only place to make this dirty
         */
        local_sync_work_area->dirty = TRUE;
    }

    if (TRUE == local_sync_work_area->dirty)
    {
        local_sync_work_area->dirty_timer     = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_HOLD_TIME;
        local_sync_work_area->max_dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_MAX_HOLD_TIME;
    }

    /* first time must sync all global blocks to all slaves.
     * only *global* need to sync
     */
    start = CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE + CFGDB_TYPE_GLOBAL_SECTION_STARTING_OFFSET;
    end   = CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE + local_sync_work_area->available_offset - 1;

    for(unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, existing_units))
        {
            continue;
        }

        if (unit_id == my_unit_id)
        {
            continue;
        }

        for(block_id = start/CFGDB_TYPE_DIRTY_BLOCK_SIZE; block_id <= end/CFGDB_TYPE_DIRTY_BLOCK_SIZE; block_id++)
        {
            remote_sync_work_area->dirty[unit_id-1][block_id] = TRUE;
        }

        for(section_handler=SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION;
            section_handler<=available_section_handler-1;
            section_handler++)
        {
            if (0 == config_file_shadow->descriptor[section_handler-1].section_id)
            {
                continue;
            }

            if ((CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL == config_file_shadow->descriptor[section_handler-1].section_type) ||
                (CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL == config_file_shadow->descriptor[section_handler-1].section_type))
            {
                remote_sync_work_area->global_desc_dirty[unit_id-1][section_handler-1] = TRUE;
            }
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_TimerEventSync
 *-------------------------------------------------------------------------
 * PURPOSE: When timer event occures, use this routine.
 * INPUT:   local_sync_work_area -- Local sync working area.
 *          config_file_shadow_p -- File shadow.
 *          filename -- Filename in the FLASH.
 * OUTPUT:  None.
 * RETUEN:  CFGDB_UTIL_WRITE_FLASH_FAIL/CFGDB_UTIL_WRITE_FLASH_NONE/CFGDB_UTIL_WRITE_FLASH_SUCCESS
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
UI32_T CFGDB_UTIL_TimerEventSync(CFGDB_TYPE_LocalSyncWorkArea_T  *local_sync_work_area,
                                 CFGDB_TYPE_ConfigFile_T         *config_file_shadow_p,
                                 UI8_T                           *filename)
{
    BOOL_T is_timeout;
    UI8_T  *shadow_clone_p = NULL;
    UI32_T old_dirty_timer = 0;
    UI32_T old_max_dirty_timer = 0;
    UI32_T retval;

    /* make a clone from shadow to prevent from race condition
     */
    //original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cfgdb_util_sem_id);
    CFGDB_UTIL_EnterCriticalSection();
    {
        if (local_sync_work_area->dirty_timer  > SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS)
        {
            local_sync_work_area->dirty_timer -= SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS;
        }
        else
        {
            local_sync_work_area->dirty_timer = 0;
        }

        if (local_sync_work_area->max_dirty_timer  > SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS)
        {
            local_sync_work_area->max_dirty_timer -= SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS;
        }
        else
        {
            local_sync_work_area->max_dirty_timer = 0;
        }

        if ( (TRUE == local_sync_work_area->dirty) &&
             ((0   == local_sync_work_area->dirty_timer) || (0 == local_sync_work_area->max_dirty_timer)) )
        {
            is_timeout = TRUE;

            if (NULL == (shadow_clone_p = (UI8_T *)calloc(SYS_ADPT_BINARY_CFG_FILE_SIZE, sizeof(UI8_T))))
            {
                SYSFUN_Debug_Printf("\n\rCFGDB: Allocating config file shadow clone memory in timer event failed.\n\r");
                //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);
                CFGDB_UTIL_LeaveCriticalSection();
                return CFGDB_UTIL_WRITE_FLASH_FAIL;
            }
            memcpy(shadow_clone_p, config_file_shadow_p, SYS_ADPT_BINARY_CFG_FILE_SIZE);

            old_dirty_timer = local_sync_work_area->dirty_timer;
            old_max_dirty_timer = local_sync_work_area->max_dirty_timer;
        }
        else
        {
            is_timeout = FALSE;;
        }
    }
    CFGDB_UTIL_LeaveCriticalSection();
    //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);

    if (TRUE == is_timeout)
    {
        UI32_T fs_result;

        fs_result = FS_WriteFile (DUMMY_DRIVE,
                                  filename,
                                  (UI8_T *)CFGDB_TYPE_FS_COMMENT,
                                  FS_FILE_TYPE_BINARY_CONFIG,
                                  shadow_clone_p,
                                  SYS_ADPT_BINARY_CFG_FILE_SIZE,
                                  SYS_ADPT_BINARY_CFG_FILE_SIZE);

        //original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cfgdb_util_sem_id);
        CFGDB_UTIL_EnterCriticalSection();
        {
            if (FS_RETURN_OK == fs_result)
            {
                /* sync shadow to FLASH sucessfully.
                 */

                if ((old_dirty_timer == local_sync_work_area->dirty_timer) &&
                    (old_max_dirty_timer == local_sync_work_area->max_dirty_timer))
                {
                    /* if this 2 variables was *not* changed, then clear the dirty flag
                     * is necessary.
                     */

                    local_sync_work_area->dirty           = FALSE;
                    local_sync_work_area->dirty_timer     = 0;
                    local_sync_work_area->max_dirty_timer = 0;
                }
                else
                {

                    /* if this 2 variables was changed, means another writing was requested,
                     * then *don't* clear the dirty flag.
                     */
                    ;
                }

                retval = CFGDB_UTIL_WRITE_FLASH_SUCCESS;
            }
            else
            {

                /* Failed to sync shadow to FLASH.
                 */
                /* postpone the action to flush shadow to FLASH, just like a new
                 * request wrintig.
                 */
                local_sync_work_area->dirty           = TRUE ;
                local_sync_work_area->dirty_timer     = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_HOLD_TIME;
                local_sync_work_area->max_dirty_timer = SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_MAX_HOLD_TIME;


                retval = CFGDB_UTIL_WRITE_FLASH_FAIL;
            }
        }
        CFGDB_UTIL_LeaveCriticalSection();
        //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);

        /* this pointer is used to allocated if and only if is_timeout is TRUE,
         * so free this pointer here is fine
         */

        free(shadow_clone_p);

    }
    else
    {

        retval = CFGDB_UTIL_WRITE_FLASH_NONE;
    }



    if ((CFGDB_UTIL_WRITE_FLASH_SUCCESS == retval) &&
        (TRUE == cfgdb_util_backdoor_flags.display_flash))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: sync shadow to FLASH in timer event: [%lu]\r\n", SYSFUN_GetSysTick());
    }

    return retval;
}




/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_FlushEventSync
 *-------------------------------------------------------------------------
 * PURPOSE: Use this routine when flush event is there.
 * INPUT:   local_sync_work_area -- Local sync working area.
 *          config_file_shadow_p -- File shadow.
 *          filename -- Filename in the FLASH.
 * OUTPUT:  None.
 * RETUEN:  CFGDB_UTIL_WRITE_FLASH_FAIL/CFGDB_UTIL_WRITE_FLASH_NONE/CFGDB_UTIL_WRITE_FLASH_SUCCESS
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
UI32_T CFGDB_UTIL_FlushEventSync(CFGDB_TYPE_LocalSyncWorkArea_T  *local_sync_work_area,
                                 CFGDB_TYPE_ConfigFile_T         *config_file_shadow_p,
                                 UI8_T                           *filename)
{
    UI8_T  *shadow_clone_p;
    UI32_T fs_result;
    UI32_T old_max_dirty_timer;
    UI32_T old_dirty_timer;

    /* make a clone from shadow to prevent from race condition
     */
    //original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cfgdb_util_sem_id);
    CFGDB_UTIL_EnterCriticalSection();
    {
        if (FALSE == local_sync_work_area->dirty)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);
            return CFGDB_UTIL_WRITE_FLASH_NONE;
        }

        if (NULL == (shadow_clone_p = (UI8_T *)calloc(SYS_ADPT_BINARY_CFG_FILE_SIZE, sizeof(UI8_T))))
        {
            //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);
            CFGDB_UTIL_LeaveCriticalSection();
            return CFGDB_UTIL_WRITE_FLASH_FAIL;
        }
        memcpy(shadow_clone_p, config_file_shadow_p, SYS_ADPT_BINARY_CFG_FILE_SIZE);

        old_dirty_timer     = local_sync_work_area->dirty_timer;
        old_max_dirty_timer = local_sync_work_area->max_dirty_timer;

        /* suppose writing to flash will be successful
         */
        local_sync_work_area->dirty_timer     = 0;
        local_sync_work_area->max_dirty_timer = 0 ;
        local_sync_work_area->dirty           = FALSE;
    }
    CFGDB_UTIL_LeaveCriticalSection();
    //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);

    fs_result = FS_WriteFile (DUMMY_DRIVE,
                              filename,
                              (UI8_T *)CFGDB_TYPE_FS_COMMENT,
                              FS_FILE_TYPE_BINARY_CONFIG,
                              shadow_clone_p,
                              SYS_ADPT_BINARY_CFG_FILE_SIZE,
                              SYS_ADPT_BINARY_CFG_FILE_SIZE);

    free(shadow_clone_p);

    //original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cfgdb_util_sem_id);
    CFGDB_UTIL_EnterCriticalSection();
    {
        if (FS_RETURN_OK != fs_result)
        {
            /* writing to FLASH was failed.
             */
            local_sync_work_area->dirty_timer     = old_dirty_timer;
            local_sync_work_area->max_dirty_timer = old_max_dirty_timer ;
            local_sync_work_area->dirty           = TRUE;

            //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);
            CFGDB_UTIL_LeaveCriticalSection();
            return CFGDB_UTIL_WRITE_FLASH_FAIL;
        }
    }
    CFGDB_UTIL_LeaveCriticalSection();
    //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);

    if (TRUE == cfgdb_util_backdoor_flags.display_flash)
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Sync shadow to FLASH in flush.\r\n");
    }
    return CFGDB_UTIL_WRITE_FLASH_SUCCESS;
}

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_SendPacket
 *----------------------------------------------------------------------------------
 * Purpose: To send out packet receive to remote unit.
 * Input:   packet_type  -- packet type
 *          from_unit_id -- from unit id.
 *          to_unit_bmp  -- to_unit is unit bit map, defined in iuc.h, to send
 *                          unreliable packet by using ISC_Send().
 *          block_id     -- block ID for ack or to sync
 *          buffer       -- data to syn. if packet type is ack, buff is unused.
 * Output:  None.
 * Return:  The reason of always useing ISC_Send() is for non-blocking purpose.
 *----------------------------------------------------------------------------------
 */
void CFGDB_UTIL_SendPacket(UI32_T packet_type, UI32_T from_unit_id, UI32_T to_unit_bmp,
                           UI32_T block_id,    UI8_T *buffer,       UI32_T is_external)
{
    UI32_T   total_len, pdu_len;
    UI32_T   payload_len;
    CFGDB_TYPE_Packet_T   *isc_buf_p;
    L_MM_Mref_Handle_T    *mref_handle_p;

    if (CFGDB_TYPE_PACKET_TYPE_SYNC == packet_type)
    {
        /* 1. SYNC
         */
        payload_len = CFGDB_TYPE_DIRTY_BLOCK_SIZE;
    }
    else if (CFGDB_TYPE_PACKET_TYPE_DESC_SYNC == packet_type)
    {
        /* 1. DESC_SYNC
         */
        payload_len = sizeof(CFGDB_TYPE_SectionDescriptor_T);
    }
    else
    {
        /* 1. ACK
         * 2. FLASH
         * 3. FALSE_DONE
         * 4. DESC_ACK
         */
        payload_len = 0;
    }

    total_len = sizeof(CFGDB_TYPE_Packet_T) + payload_len;

    mref_handle_p = L_MM_AllocateTxBuffer(total_len, L_MM_USER_ID(SYS_MODULE_CFGDB_UTIL, CFGDB_UTIL_POOL_ID_DEFAULT, packet_type));
    isc_buf_p = (CFGDB_TYPE_Packet_T*) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if(isc_buf_p==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
        return;
    }

    isc_buf_p->packet_type   = packet_type;
    isc_buf_p->from_unit_id  = from_unit_id;
    isc_buf_p->to_unit_bmp   = to_unit_bmp;
    isc_buf_p->block_id      = block_id;
    isc_buf_p->is_external   = is_external;


    if (CFGDB_TYPE_PACKET_TYPE_SYNC == packet_type)
    {
        /* if packet type is SYNC, copy data from buffer to packet payload
         */
        memcpy (isc_buf_p->payload, buffer, CFGDB_TYPE_DIRTY_BLOCK_SIZE);
    }
    else if (CFGDB_TYPE_PACKET_TYPE_DESC_SYNC == packet_type)
    {
        /* if packet type is DESC_SYNC, copy data from buffer to packet payload
         */
        memcpy (isc_buf_p->payload, buffer, sizeof(CFGDB_TYPE_SectionDescriptor_T));
    }

    if (FALSE == ISC_Send(to_unit_bmp, ISC_CFGDB_SID, mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY))
    {
        if (TRUE == cfgdb_util_backdoor_flags.display_tx)
        {
            SYSFUN_Debug_Printf("\r\nCFGDB: Failed to send packet via ISC.\r\n");
        }
    }
    else
    {
        if (TRUE == cfgdb_util_backdoor_flags.display_tx)
        {
            SYSFUN_Debug_Printf("\r\nCFGDB: TX packet. Type [%lu]. From [%lu]. To [0x%lx]. Block ID [%lu]. Is_ext [%lu]. [%lu] ticks\r\n",
                                                            packet_type,
                                                            from_unit_id,
                                                            to_unit_bmp,
                                                            block_id,
                                                            is_external,
                                                            SYSFUN_GetSysTick());
        }
    }
    return;

} /*CFGDB_UTIL_SendPacket*/


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_UTIL_SendSyncToSlave
 *----------------------------------------------------------------------------------
 * Purpose: To send SYNC packet to remote unit.
 * Input:   config_file_shadow_p -- config shadow.
 *          remote_sync_work_area -- remote sync working area.
 *          remote_sync_work_area -- existing units.
 *          my_unit_id -- my unit ID.
 * Output:  None.
 * Return:  The reason of always useing ISC_Send() is for non-blocking purpose.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_UTIL_SendSyncToSlave(  CFGDB_TYPE_ConfigFile_T         *config_file_shadow_p,
                                    CFGDB_TYPE_RemoteSyncWorkArea_T *remote_sync_work_area,
                                    UI32_T                          existing_units,
                                    UI32_T                          my_unit_id,
                                    UI32_T                          is_external)
{
    UI32_T  block_id;
    UI32_T  unit_id;
    UI32_T  to_unit_bmp;
    UI8_T   *block_clone_p;
    UI8_T   *section_desc_clone_p;
    UI32_T  sent_packet_number;
    UI32_T  section_handler;


    /* make a clone from shadow to prevent from race condition
     */
    sent_packet_number = 0;

    /* for section descriptor
     */
    if (NULL == (section_desc_clone_p = calloc(sizeof(CFGDB_TYPE_SectionDescriptor_T), sizeof(UI8_T))))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to allocate memory in timer handler [1].\r\n");
        return FALSE;
    }
    for (section_handler=1; section_handler<=SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION; section_handler++)
    {
        //original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cfgdb_util_sem_id);
        CFGDB_UTIL_EnterCriticalSection();
        {
            memcpy( section_desc_clone_p,
                    (UI8_T *)(&(config_file_shadow_p->descriptor[section_handler-1])),
                    sizeof(CFGDB_TYPE_SectionDescriptor_T));

            to_unit_bmp = 0;
            for(unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
            {
                if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, existing_units))
                {
                    continue;
                }

                if (unit_id == my_unit_id)
                {
                    continue;
                }

                if (TRUE == remote_sync_work_area->global_desc_dirty[unit_id-1][section_handler-1])
                {
                    to_unit_bmp |= IUC_STACK_UNIT_BMP(unit_id);
                }
            }
        }
        //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);
        CFGDB_UTIL_LeaveCriticalSection();

        if (0 != to_unit_bmp)
        {
            CFGDB_UTIL_SendPacket(CFGDB_TYPE_PACKET_TYPE_DESC_SYNC, my_unit_id, to_unit_bmp, section_handler, section_desc_clone_p, is_external);

            if (++sent_packet_number >= CFGDB_TYPE_MAX_NBR_OF_SYNC_PACKET_PER_TURN)
            {
                break;
            }
        }
    }
    free(section_desc_clone_p);

    /* for section data
     */
    if (NULL == (block_clone_p = calloc(CFGDB_TYPE_DIRTY_BLOCK_SIZE, sizeof(UI8_T))))
    {
        SYSFUN_Debug_Printf("\r\nCFGDB: Failed to allocate memory in timer handler [2].\r\n");
        return FALSE;
    }
    for (block_id=0; block_id<CFGDB_TYPE_DIRTY_BLOCK_NUMBER; block_id++)
    {
        //original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(cfgdb_util_sem_id);
        CFGDB_UTIL_EnterCriticalSection();
        {
            memcpy(block_clone_p,
                  (UI8_T *)config_file_shadow_p + block_id*CFGDB_TYPE_DIRTY_BLOCK_SIZE,
                   CFGDB_TYPE_DIRTY_BLOCK_SIZE);

            to_unit_bmp = 0;
            for(unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
            {
                if (FALSE == CFGDB_UTIL_GET_UNIT_EXISTING(unit_id, existing_units))
                {
                    continue;
                }

                if (unit_id == my_unit_id)
                {
                    continue;
                }

                if (TRUE == remote_sync_work_area->dirty[unit_id-1][block_id])
                {
                    to_unit_bmp |= IUC_STACK_UNIT_BMP(unit_id);
                }
            }
        }
        //SYSFUN_OM_LEAVE_CRITICAL_SECTION(cfgdb_util_sem_id, original_priority);
        CFGDB_UTIL_LeaveCriticalSection();

        if (0 != to_unit_bmp)
        {
            CFGDB_UTIL_SendPacket(CFGDB_TYPE_PACKET_TYPE_SYNC, my_unit_id, to_unit_bmp, block_id, block_clone_p, is_external);

            if (++sent_packet_number >= CFGDB_TYPE_MAX_NBR_OF_SYNC_PACKET_PER_TURN)
            {
                break;
            }
        }
    }
    free(block_clone_p);

    /* end of sync
     */
    return TRUE;
}
