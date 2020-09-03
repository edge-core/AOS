/*-----------------------------------------------------------------------------
 * Module Name: CFGDB_UTIL.H
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
#ifndef _CFGDB_UTIL_
#define _CFGDB_UTIL_

#define CFGDB_UTIL_SET_UNIT_EXISTING(UNIT_ID, EXISTING, DB)          (EXISTING? (DB |= (1<<((UNIT_ID)-1))) : (DB &= (~(1<<((UNIT_ID)-1)))))
#define CFGDB_UTIL_GET_UNIT_EXISTING(UNIT_ID, DB)                    (((DB) & (1<<((UNIT_ID)-1))) ? TRUE : FALSE)


#define CFGDB_UTIL_WRITE_FLASH_NONE                                 1
#define CFGDB_UTIL_WRITE_FLASH_SUCCESS                              2
#define CFGDB_UTIL_WRITE_FLASH_FAIL                                 3

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
void CFGDB_UTIL_SetDebugFlags(CFGDB_TYPE_BackdoorFlags_T *backdoor_flags_ptr);

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
void CFGDB_UTIL_SetSemID(UI32_T sem_id);

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
BOOL_T CFGDB_UTIL_Check32BitChecksum(UI8_T *data, UI32_T length, UI32_T checksum);

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
void CFGDB_UTIL_Generate32BitChecksum(UI8_T *data, UI32_T length, UI32_T *checksum);


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
BOOL_T CFGDB_UTIL_WriteDummyBinConfigToFlash(UI32_T version, UI8_T *filename, CFGDB_TYPE_ConfigFile_T *showdow_buf_p);


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
                                           SYS_TYPE_Stacking_Mode_T       mode);

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
                                        SYS_TYPE_Stacking_Mode_T    mode);

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
                                BOOL_T                          *need_default_table);

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
                           UI8_T                            *data_buffer);

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
                                 UI32_T                         *available_offset);

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
                                UI32_T                          my_unit_id);

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
                                 UI8_T                           *filename);

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
                                 UI8_T                           *filename);

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
                           UI32_T block_id,    UI8_T *buffer,       UI32_T is_external);
                                 

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
                                    UI32_T                          is_external);



#endif
