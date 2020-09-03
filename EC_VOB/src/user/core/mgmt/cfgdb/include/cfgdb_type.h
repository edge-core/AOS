/*-----------------------------------------------------------------------------
 * Module Name: CFGDB_TYPE.H
 *-----------------------------------------------------------------------------
 * PURPOSE: Types of this computer software component.
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
#ifndef _CFGDB_TYPE_
#define _CFGDB_TYPE_

#define CFGDB_TYPE_VERSION                               0x01000003

#define CFGDB_TYPE_BACKDOOR_INIT_STATE                   FALSE

#define CFGDB_TYPE_DIRTY_BLOCK_SIZE                      1024 /*in byte*/
#define CFGDB_TYPE_DIRTY_BLOCK_NUMBER                    ((SYS_ADPT_BINARY_CFG_FILE_SIZE+CFGDB_TYPE_DIRTY_BLOCK_SIZE-1)/CFGDB_TYPE_DIRTY_BLOCK_SIZE)

#define CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE	         (sizeof(CFGDB_TYPE_ConfigFile_T) - sizeof(((CFGDB_TYPE_ConfigFile_T *)0)->section_data))
#define CFGDB_TYPE_BINARY_CFG_FILE_BODY_SIZE             (SYS_ADPT_BINARY_CFG_FILE_SIZE - CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE)    
#define CFGDB_TYPE_FS_COMMENT                            "CFGDB"

#define CFGDB_TYPE_HEADER_BLOCK_NUMBER                   ((CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE+CFGDB_TYPE_DIRTY_BLOCK_SIZE-1)/CFGDB_TYPE_DIRTY_BLOCK_SIZE)
#define CFGDB_TYPE_DATA_BLOCK_NUMBER                     (CFGDB_TYPE_DIRTY_BLOCK_NUMBER-CFGDB_TYPE_HEADER_BLOCK_NUMBER)

#define CFGDB_TYPE_LOCAL_SECTION_STARTING_OFFSET         (CFGDB_TYPE_HEADER_BLOCK_NUMBER*CFGDB_TYPE_DIRTY_BLOCK_SIZE-CFGDB_TYPE_BINARY_CFG_FILE_HEADER_SIZE)
#define CFGDB_TYPE_GLOBAL_SECTION_STARTING_OFFSET        (CFGDB_TYPE_LOCAL_SECTION_STARTING_OFFSET+((CFGDB_TYPE_DATA_BLOCK_NUMBER/2)*CFGDB_TYPE_DIRTY_BLOCK_SIZE))


enum
{
    CFGDB_TYPE_PACKET_TYPE_SYNC         = 1,    /*master to slave*/
    CFGDB_TYPE_PACKET_TYPE_ACK          = 2,    /*slave to master*/

    CFGDB_TYPE_PACKET_TYPE_FLASH        = 3,    /*master to slave*/
    CFGDB_TYPE_PACKET_TYPE_FLASH_DONE   = 4,    /*slave to master*/

    CFGDB_TYPE_PACKET_TYPE_DESC_SYNC    = 5,    /*master to slave*/
    CFGDB_TYPE_PACKET_TYPE_DESC_ACK     = 6,    /*slave to master*/
};


#define CFGDB_TYPE_EVENT_TIMER                      BIT_1
#define CFGDB_TYPE_EVENT_ENTER_TRANSITION           BIT_2
#define CFGDB_TYPE_EVENT_SHUTDOWN                   BIT_3


#define CFGDB_TYPE_MAX_NBR_OF_SYNC_PACKET_PER_TURN       20

/* Note: descriptor size is 32, non used field reserved later extension
 */
typedef  struct
{
    UI32_T  section_id;                /* section id SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION to SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION, 0 is invalid */
    UI32_T  section_start_offset;      /* offset from CFGDB_MGR_ConfigFile_T.section */
    UI32_T  section_record_size;       /* size of one record in byte */
    UI32_T  section_record_number;     /* total number of records */    /* section_record_size*section_recore_num is record length */
    UI32_T  section_checksum;          /* whole section data checksum, section level checksum */
    UI32_T  section_type;              /* section type of this section */
    UI8_T   reserved[8];
} CFGDB_TYPE_SectionDescriptor_T;


/* binary config file format for both master and slave unit
 */
typedef struct 
{
   UI32_T                           checksum;                   /* file level checksum, this field shall always be kept in first position*/
   UI32_T                           version;
   CFGDB_TYPE_SectionDescriptor_T   descriptor[SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION];
    /* the tall size of CFGDB_TYPE_ConfigFile_T is SYS_ADPT_BINARY_CFG_FILE_SIZE bytes
     */
   UI8_T                            section_data[SYS_ADPT_BINARY_CFG_FILE_SIZE-4-4-(sizeof(CFGDB_TYPE_SectionDescriptor_T)*SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION)];
} CFGDB_TYPE_ConfigFile_T;


/* for both master and slave unit
 */
typedef struct
{
    BOOL_T  dirty;                          /*master/slave*/
    UI32_T  dirty_timer;                    /*master/slave*/
    UI32_T  max_dirty_timer;                /*master/slave*/
    UI32_T  available_offset;               /*master*/      /* relative to CFGDB_TYPE_ConfigFile_T.section */
    UI32_T  availible_index;                /*master*/
} CFGDB_TYPE_LocalSyncWorkArea_T;


/* for master unit only
 */
typedef struct
{
    BOOL_T      dirty                   [SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][CFGDB_TYPE_DIRTY_BLOCK_NUMBER];
    BOOL_T      flash_done              [SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    BOOL_T      global_desc_dirty       [SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION];
} CFGDB_TYPE_RemoteSyncWorkArea_T;

typedef struct
{
    UI32_T   packet_type;      /* packet type: sync/ack */
    UI32_T   from_unit_id;     /* from unit             */
    UI32_T   to_unit_bmp;      /* to unit bit map       */
    UI32_T   is_external;      /* internal or external binary config file */
    UI32_T   block_id;         /* block ID to sync/ack  */
    UI8_T    payload[0];       /* if this is sync packet, the payload is data to sync  */
                               /* and length shall be CFGDB_TYPE_DIRTY_BLOCK_SIZE byte, */
                               /* if this is ack, the length is 0                      */
} CFGDB_TYPE_Packet_T;


typedef struct
{
    BOOL_T display_flash;
    BOOL_T display_tx;
    BOOL_T display_rx;
    BOOL_T display_tick;
    UI32_T display_dbg_msg_level;
    BOOL_T disable_writing;
    
}   CFGDB_TYPE_BackdoorFlags_T;

#endif
