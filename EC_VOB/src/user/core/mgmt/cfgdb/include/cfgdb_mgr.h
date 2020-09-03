/* #define SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS        500  */
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
#ifndef _CFGDB_MGR_H
#define _CFGDB_MGR_H

#include "sys_type.h"
#include "sys_cpnt.h"
#include "isc.h"
#include "sysrsc_mgr.h"
#include "sysfun.h"
#include "cfgdb_type.h"

/* NAMING CONSTANT DECLARATIONS 
 */
/* value shall not be changed
 */
enum
{
    CFGDB_MGR_SECTION_ID_NETIF_1 = 1,
    CFGDB_MGR_SECTION_ID_NETCFG_1,
    CFGDB_MGR_SECTION_ID_NETCFG_2,
        
    CFGDB_MGR_SECTION_ID_USERAUTH_1,
    CFGDB_MGR_SECTION_ID_USERAUTH_2,
    CFGDB_MGR_SECTION_ID_USERAUTH_3,
    
    CFGDB_MGR_SECTION_ID_RADIUS_1,
    
    CFGDB_MGR_SECTION_ID_XFER_1,
    
    CFGDB_MGR_SECTION_ID_NETCFG_RIP_1,
    
    CFGDB_MGR_SECTION_ID_SNMP_1,
    CFGDB_MGR_SECTION_ID_SNMP_2,
    
    CFGDB_MGR_SECTION_ID_ALERT_LED_1,
    
    CFGDB_MGR_SECTION_ID_SNMP_3,
    
    CFGDB_MGR_SECTION_ID_SSHD_1,
    CFGDB_MGR_SECTION_ID_DHCPSNP_1,
    CFGDB_MGR_SECTION_ID_DHCPv6_1,

    CFGDB_MGR_SECTION_ID_VOICE_VLAN_1,

    CFGDB_MGR_SECTION_ID_BOOT_REASON
};

enum
{
    CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL  = 1,
    CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL = 2,    
    CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL  = 3,
    CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL = 4,
};

#define CFGDB_MGR_SECTION_TYPE_TO_BMP(TYPE)    (1<<((TYPE)-1))

typedef struct 
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    UI32_T                              cfgdb_mgr_my_unit_id;
    UI32_T                              cfgdb_mgr_existing_units;
    UI32_T                              cfgdb_mgr_available_section_handler;      /*master*/
    BOOL_T                              cfgdb_mgr_allow_to_open;                  /*master*/
    BOOL_T                              cfgdb_mgr_run_timer;                      /*master/slave*/
    BOOL_T                              cfgdb_mgr_shutdown_write;                 /*master*/
    UI32_T                              cfgdb_mgr_wait_for_sync_section_handler;
    UI32_T                              cfgdb_mgr_section_id_table[SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION]; /*master*/
    UI32_T                              cfgdb_mgr_section_type_table[SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION];
    CFGDB_TYPE_ConfigFile_T             config_file_shadow;
    CFGDB_TYPE_ConfigFile_T             internal_config_file_shadow;
    CFGDB_TYPE_LocalSyncWorkArea_T      local_sync_work_area;
    CFGDB_TYPE_LocalSyncWorkArea_T      internal_local_sync_work_area;  
    CFGDB_TYPE_RemoteSyncWorkArea_T     remote_sync_work_area; 
    CFGDB_TYPE_RemoteSyncWorkArea_T     internal_remote_sync_work_area;
    CFGDB_TYPE_SectionDescriptor_T      init_time_descriptor[SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION];
    CFGDB_TYPE_SectionDescriptor_T      internal_init_time_descriptor[SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION];
    CFGDB_TYPE_BackdoorFlags_T          cfgdb_backdoor_flags;
    UI32_T   cfgdb_mgr_task_id;
} CFGDB_MGR_Shmem_Data_T;

/* DATA TYPE DECLARACTION
 */

/* EXPORTED FUNCTIONS
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
void CFGDB_MGR_Initiate_System_Resources(void);

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
void CFGDB_MGR_AttachSystemResources(void);

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
void CFGDB_MGR_Create_InterCSC_Relation(void);

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
void CFGDB_MGR_EnterMasterMode();

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
void CFGDB_MGR_EnterSlaveMode();





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
void CFGDB_MGR_EnterTransitionMode();

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
void CFGDB_MGR_SetTransitionMode();

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
void CFGDB_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


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
void CFGDB_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


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
SYS_TYPE_Stacking_Mode_T CFGDB_MGR_GetOperationMode(void);


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
                       UI32_T section_type ,    BOOL_T *need_to_sync);

 
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
void CFGDB_MGR_EndOfOpen(void);


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
BOOL_T CFGDB_MGR_SyncSection(UI32_T section_handler, void *data_buffer);

 
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
void CFGDB_MGR_EndOfOpen(void);

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
BOOL_T CFGDB_MGR_ReadSection ( UI32_T section_handler, void  *buf);

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
BOOL_T CFGDB_MGR_Read (UI32_T section_handler, void  *buf, UI32_T offset, UI32_T count );

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
BOOL_T CFGDB_MGR_WriteSection ( UI32_T section_handler, void *buf);


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
BOOL_T CFGDB_MGR_Write( UI32_T section_handler, void  *buf, UI32_T offset, UI32_T len);

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
BOOL_T CFGDB_MGR_Flush();


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
BOOL_T CFGDB_MGR_Shutdown (void);


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_ResetConfiguration
 *----------------------------------------------------------------------------------
 * Purpose: To reset the next boot configuration in FLASH.
 * Input:   reset_type_bmp -- bit map of CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_LOCAL
 *                                       CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL
 *                                       CFGDB_MGR_SECTION_TYPE_INTRENAL_AND_LOCAL
 *                                       CFGDB_MGR_SECTION_TYPE_INTERNAL_AND_GLOBAL
 *          shutdown -- To shutdown the service at the same time.
 * Output:  None.
 * Return:  TRUE/FALSE.
 *----------------------------------------------------------------------------------
 */
BOOL_T CFGDB_MGR_ResetConfiguration(UI32_T reset_type_bmp, BOOL_T shutdown);


/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_SetTaskId
 *----------------------------------------------------------------------------------
 * Purpose: TASK tell MGR the task ID, for MGR to send event to TASK.
 * Input:   cfgdb_task_id - Task ID of CFGDB_TASK.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_SetTaskId(UI32_T cfgdb_task_id);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_GetTaskId
 *----------------------------------------------------------------------------------
 * Purpose: Get the task id
 * Input:   cfgdb_task_id - Task ID of CFGDB_TASK.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_GetTaskId(UI32_T *cfgdb_task_id);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_MGR_ShutdownHandler
 *----------------------------------------------------------------------------------
 * Purpose: Timer handler shutdown event
 * Input:   None.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void CFGDB_MGR_ShutdownHandler(void);


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
void CFGDB_MGR_TimerHandler(void);

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
BOOL_T CFGDB_MGR_ReceivePacket_CallBack(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id);

#endif
