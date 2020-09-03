/* MODULE NAME:  dbsync_txt_mgr.c
* PURPOSE:
*   Initialize the resource and provide some functions for the dbsync_txt module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-2-10       -- poli , created.
*
* Copyright(C)      Accton Corporation, 2002
*/


/* INCLUDE FILE DECLARATIONS
 */

#if (SYS_CPNT_DBSYNC_TXT == TRUE)

#include <string.h>
#include <sys_type.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbsync_txt_mgr.h"
#include "sysfun.h"
#include "fs.h"
#include "fs_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "cli_def.h"
//rich#include "xfer_mgr.h"
#include "sys_time.h"
#include "stktplg_mgr.h"
//rich#include "Xfer_buf_mgr.h"
#include "leaf_es3626a.h"
#include "backdoor_mgr.h"
#include "stktplg_pom.h"
#include "xfer_mgr.h"
#include "xfer_pmgr.h"
#include "xfer_buf_pmgr.h"

/*------------------------------------------------------------------------
 * GLOBAL VARIABLES
 *-----------------------------------------------------------------------*/
#define DBSYNC_TXT_CHECK_OPER_MODE(RET_VAL)                             \
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)  {   \
     return (RET_VAL);                                                      \
  }

#define DBSYNC_TXT_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE()               \
  if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)  {   \
    return;                                                                 \
  }

#define DBSYNC_TXT_RETURN_AND_RELEASE_CSC(RET_VAL)  {               \
    return (RET_VAL);                                           \
  }

#define DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE()  { \
    return;                                                     \
  }

#define DBSYNC_TXT_CHECK_OPER_MODE_WITH_RETURN_VALUE(RET_VAL)               \
 if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)  {   \
    return RET_VAL;                                                                 \
  }

#define DBSYNC_TXT_RETURN_AND_RELEASE_CSC_WITH_RETURN_VALUE(RET_VAL)  { \
    return RET_VAL;                                                     \
  }

#define  DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id)  SYSFUN_OM_ENTER_CRITICAL_SECTION(dbsync_txt_mgr_sem_id);
#define  DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id, orig_priority)  SYSFUN_OM_LEAVE_CRITICAL_SECTION(dbsync_txt_mgr_sem_id, orig_priority);
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/*  LOCAL SUBPROGRAM DECLARATIONS
 */

#if 0
static void     DBSYNC_TXT_MGR_PrintDirtyBit( void );
#endif

static BOOL_T   DBSYNC_TXT_MGR_DeleteNonStdCfg( void );
static void     DBSYNC_TXT_MGR_SetSyncSlaveDirty( UI32_T unit_id );
static BOOL_T   DBSYNC_TXT_MGR_ClearSyncSlaveDirty( void );
static UI32_T   DBSYNC_TXT_MGR_GetNextSlaveDirtyAndClear( UI32_T * now_unit );
static UI32_T   DBSYNC_TXT_MGR_GetDirtyAndClear( void );

/*  STATIC VARIABLE DECLARATIONS
 */
static BOOL_T  autosave_dirtybit = FALSE;
static UI32_T  autosync_dirtybit[ SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ];
static UI32_T  autosync_lastunit = 0;
static UI32_T  dbsync_txt_mgr_sem_id;
static BOOL_T  dbsync_txt_mgr_provision_complete;
static UI32_T  is_doingautosave = FALSE;
static UI32_T  autosave_disable = FALSE;
static UI32_T  force_save_disable = FALSE;
static UI32_T  autosave_timer = 0;
static BOOL_T  is_delete_other_cfg_file = FALSE;
#if 0
static void    DBSYNC_TXT_MGR_SaveRunningCfg_Callback(UI32_T cookie, UI32_T status);
#endif

//rich
#ifndef SYS_DFLT_DBSYNC_TXT_CONFIG_AUTOSAVE_ENABLE_STATUS
#define SYS_DFLT_DBSYNC_TXT_CONFIG_AUTOSAVE_ENABLE_STATUS VAL_switchConfigAutosaveEnableStatus_enabled
#endif

#ifndef SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME
#define SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME               "startup%d.cfg" /* "%d" = "1" or "2" */
#endif

#ifndef SYS_DFLT_DBSYNC_TXT_INTERVAL
#define SYS_DFLT_DBSYNC_TXT_INTERVAL                    1 /*30 based on the suggestion of SJ, and is same as Hagrid*//* seconds */
#endif

static UI32_T  DBSYNC_TXT_MGR_GetDirtyAndClear(void);
static UI32_T  SwitchConfigAutosaveStatus = SYS_DFLT_DBSYNC_TXT_CONFIG_AUTOSAVE_ENABLE_STATUS;
#if 0 /*maggie liu remove warning, should be open SYS_CPNT_DBSYNC_TXT == TRUE*/
static void    DBSYNC_TXT_MGR_Backdoor_CallBack(void);
#endif
SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  DBSYNC_TXT_MGR_Init
 * PURPOSE:
 *          Initiate the semaphore for DBSYNC_TXT objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DBSYNC_TXT_INIT_Initiate_System_Resources.
 */
BOOL_T DBSYNC_TXT_MGR_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &dbsync_txt_mgr_sem_id) !=SYSFUN_OK)        
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DBSYNC_TXT_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Create_InterCSC_Relation(void)
{
    /* Register the callback function of backdoor.
     */

    /*maggie liu remove warning, should use API BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack later*/
    /*BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("dbsync", DBSYNC_TXT_MGR_Backdoor_CallBack);*/ 

} /* end of DBSYNC_TXT_MGR_Create_InterCSC_Relation */


/* FUNCTION NAME:  DBSYNC_TXT_MGR_EnterMasterMode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SSHD subsystem will enter the
 *          Master Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *              switch will be initiated to the factory default value.
 *          2. DBSYNC_TXT will handle network requests only when this subsystem
 *              is in the Master Operation mode
 *          3. This function is invoked in DBSYNC_TXT_INIT_EnterMasterMode.
 */
BOOL_T DBSYNC_TXT_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    SYSFUN_ENTER_MASTER_MODE();

    return TRUE;

}



/* FUNCTION NAME:  DBSYNC_TXT_MGR_EnterTransitionMode
 * PURPOSE:
 *          This function forces this subsystem enter the Transition Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T DBSYNC_TXT_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    autosave_dirtybit = FALSE;
    DBSYNC_TXT_MGR_ClearSyncSlaveDirty( );
    SwitchConfigAutosaveStatus = SYS_DFLT_DBSYNC_TXT_CONFIG_AUTOSAVE_ENABLE_STATUS;
    dbsync_txt_mgr_provision_complete = FALSE;
    is_doingautosave = FALSE;
    autosave_disable = FALSE;
    force_save_disable = FALSE;
    autosave_timer = 0;
    SYSFUN_ENTER_TRANSITION_MODE();

    return TRUE;
}



/* FUNCTION NAME:  DBSYNC_TXT_MGR_EnterSlaveMode
 * PURPOSE:
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests
 *          will be ignored.
 */
void DBSYNC_TXT_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_ENTER_SLAVE_MODE();

    return;

}



/* FUNCTION NAME : DBSYNC_TXT_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DBSYNC_TXT_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    SYSFUN_SET_TRANSITION_MODE();

    return;
}



/* FUNCTION NAME : DBSYNC_TXT_MGR_GetOperationMode
 * PURPOSE:
 *      Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *      None.
 */
SYS_TYPE_Stacking_Mode_T DBSYNC_TXT_MGR_GetOperationMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}       
            
            
/*  FUNCTION NAME : DBSYNC_TXT_MGR_SyncSlaveCfg
 *  PURPOSE :   Check dirty bit if sync running-config.
 *  INPUT   :   None.
 *  OUTPUT  :   None.
 *  RETURN  :   None.
 *  NOTES   :   None.
 *  HISTORY :
 *          2004/07/09  --  Reginald Ian    Create  Sync the config file to the slave unit
 */
BOOL_T DBSYNC_TXT_MGR_SyncSlaveCfg( void )
{

    /*  Local variables declaration
     */
    XFER_MGR_UserInfo_T  user_info;
    UI32_T  unit_id         = 0;
    UI32_T  master_unitid   = 0;
    UI32_T  return_state    = 4;
    UI32_T  num_unit        = 0;

    UI8_T   src_filename[ SYS_ADPT_FILE_SYSTEM_NAME_LEN ] = { 0 };
    UI8_T   dest_filename[ SYS_ADPT_FILE_SYSTEM_NAME_LEN ] = { 0 };

    /*  Entering CSC...
     */
    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );

    /*  Check if the switch is in stacking state by get the number of unit.
     */
     
    if ( STKTPLG_POM_GetNumberOfUnit( &num_unit ) == FALSE )
    {
        SYSFUN_Debug_Printf( "DBSYNC_TXT: Stacking mode error!!!\r\n" );
    }

    /*  Get the Master unit ID
     */
    if( STKTPLG_POM_GetMyUnitID( &master_unitid ) == FALSE )
    {
        SYSFUN_Debug_Printf( "DBSYNC_TXT: Stacking mode error!!!\r\n" );
    }

    /*  Exit if there is no stacking
     */
    if ( num_unit <= 1 )
    {
         
        return FALSE;
    }

    /*  Get the slave dirty bit, clear it,
     *   return the cooperate unit_id, and ask for the current state.
     */
    return_state = DBSYNC_TXT_MGR_GetNextSlaveDirtyAndClear( &unit_id );

    if ( return_state == DBSYNC_RETURN_ACTIVE || return_state == DBSYNC_RETURN_FLUSH )
    {

        /*  Get the filename of Master Startup Config
         */
        if ( FS_GetStartupFilename( master_unitid, FS_FILE_TYPE_CONFIG, src_filename ) != FS_RETURN_OK )
        {
            SYSFUN_Debug_Printf("For %ld : Get master startup config file failed in FS.\r\n", unit_id );
            DBSYNC_TXT_MGR_SetSyncSlaveDirty( unit_id );
            is_doingautosave = FALSE;
             
            return FALSE;
        }

        /*  Get the dual filename of Slave startup config file
         */
#if ( SYS_CPNT_DBSYNC_TXT == TRUE ) 
        if( XFER_MGR_Get_Slave_Unit_Dual_Startup_Cfg_FileName( unit_id, dest_filename ) == FALSE )
        {
            SYSFUN_Debug_Printf( "For %ld : Can't get slave startup cfg dual.\r\n", unit_id );
            DBSYNC_TXT_MGR_SetSyncSlaveDirty( unit_id );
            is_doingautosave = FALSE;
             
            return FALSE;
        }
#endif
        /*  Copy the master startup config to the slave with the dual filename of the slave startup file
         */
        memset(&user_info, 0, sizeof(user_info));

        if (XFER_MGR_LocalToUnit(&user_info,
                                 unit_id, dest_filename,
                                 src_filename,
                                 VAL_fileCopyAction_copy,0 ,/*maggie liu just for remove warning, need update SYS_CPNT_DBSYNC_TXT == TRUE*/
                                 0, 0 ) == FALSE)
        {
            SYSFUN_Debug_Printf( "DBSync: Delay to sync running config to unit %ld because XFER is busy.\r\n", unit_id );
            DBSYNC_TXT_MGR_SetSyncSlaveDirty( unit_id );
            is_doingautosave = FALSE;
             
            return FALSE;
        }

        /*  Set the dual filename of slave startup config file as startup.
         */
        if( FS_SetStartupFilename( unit_id, FS_FILE_TYPE_CONFIG, dest_filename ) != FS_RETURN_OK )
        {
            SYSFUN_Debug_Printf( "For %ld : Can't set slave startup cfg dual as startup is FS.\r\n", unit_id );
            DBSYNC_TXT_MGR_SetSyncSlaveDirty( unit_id );
            is_doingautosave = FALSE;
             
            return FALSE;
        }

#if 0
        SYSFUN_Debug_Printf( "Save slave %ld CFG successful...\r\n", unit_id );
#endif

        is_doingautosave = FALSE;
    }
    else
    {
        is_doingautosave = FALSE;
         
        return FALSE;
    }

    /*  Exit CSC...
     */
     
    return TRUE;
}

/* FUNCTION NAME : DBSYNC_TXT_MGR_SaveRunningCfg
 * PURPOSE:
 *      Check dirty bit if save running-config.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    :   successful
 *      FALSE   :   failed to save running config
 *
 * NOTES:
 *      None.
 */
BOOL_T DBSYNC_TXT_MGR_SaveRunningCfg( void )
{
    UI32_T  num_unit = 0;
    UI32_T  ret_state = 0;
    UI8_T   startup_filename[ SYS_ADPT_FILE_SYSTEM_NAME_LEN + 1 ] = { 0 };

    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );

    /*  Sleep 1 second for other dirty in
     */
    while ( autosave_timer != 0 )
    {
        autosave_timer --;
        SYSFUN_Sleep(1);
    }

    /*  EXIT when FLUSH or RELOAD
     */
    if ( force_save_disable == TRUE )
    {
        autosave_dirtybit = FALSE;
        is_doingautosave  = FALSE;
         
        return FALSE;
    }

    ret_state = DBSYNC_TXT_MGR_GetDirtyAndClear( );

    if ( ret_state == DBSYNC_RETURN_ACTIVE || ret_state == DBSYNC_RETURN_FLUSH )
    {

        /*  if dirtybit = true need to set another config file
         */
        /*  Change for ES4649-32-00994:
         *  DBSync shall write config to FLASH first then to set this config file as startup.
         */
#if 0//rich
        if ( XFER_MGR_GetDualStartupCfgFileName(startup_filename) == FALSE)
        {
            autosave_dirtybit = TRUE;
            is_doingautosave = FALSE;
             
            return FALSE;
        }

        if ( XFER_MGR_StreamToLocal (startup_filename, FS_FILE_TYPE_CONFIG, 0, 0 ) == FALSE)
        {
            SYSFUN_Debug_Printf( "DBSync: Delay to save running config to master because XFER is busy.\r\n" );
            autosave_dirtybit= TRUE;
            is_doingautosave = FALSE;
             
            return FALSE;
        }
#endif
        /*  Add for ES4649-32-00994:
         *  DBSync shall write config to FLASH first then to set this config file as startup.
         */
        if( FS_SetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, startup_filename) != FS_RETURN_OK )
        {
            autosave_dirtybit= TRUE;
            is_doingautosave = FALSE;
             
            return FALSE;
        }

        is_doingautosave = FALSE;

#if 0
        SYSFUN_Debug_Printf( "Master dirty saved.\r\n" );
#endif

        if ( STKTPLG_POM_GetNumberOfUnit( &num_unit ) == FALSE )
        {
            SYSFUN_Debug_Printf( "Stacking mode error!!!\r\n" );
        }
        if ( num_unit > 1 )
        {
            DBSYNC_TXT_MGR_SetSyncSlaveDirty( 0 );
        }
    }
    else
    {
         
        return FALSE;
    }

    /*  delete non standard config file at first time
     */
    if ( is_delete_other_cfg_file == FALSE )
    {
        if ( DBSYNC_TXT_MGR_DeleteNonStdCfg( ) == TRUE )
        {
            is_delete_other_cfg_file = TRUE;
        }
    }

     
    return TRUE;
}

/* FUNCTION NAME : DBSYNC_TXT_MGR_DeleteNonStdCfg
 * PURPOSE  :   Delete the config files that the filename are not normal on master unit
 * INPUT    :
 * OUTPUT   :   None.
 * RETURN   :   TRUE    :   Action success
                FALSE.  :   failed
 * NOTES    :
 * HISTORY  :
 *      2004/07/09  --  Reginald Ian    Create
 */
static BOOL_T DBSYNC_TXT_MGR_DeleteNonStdCfg( void )
{
/* Local Variables Definition
 */
    FS_File_Attr_T file_attr;
    UI32_T my_unit = 0, now_unit = 0;
    char  standard_file1[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1] = {0};
    char  standard_file2[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1] = {0};

/*  Init file_attr variable
 */
    memset(&file_attr, 0, sizeof(FS_File_Attr_T));

/*  Get the master unit ID
 */
    STKTPLG_POM_GetMyUnitID(&my_unit);

/*  Get the two standard config file name
 */
    SYSFUN_Sprintf(standard_file1, SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME, 1);
    SYSFUN_Sprintf(standard_file2, SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME, 2);

/*  Set the file type as config file
 */
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_CONFIG);

/*  scan all files
 */
    while(FS_GetNextFileInfo(&now_unit,&file_attr) == FS_RETURN_OK)
    {

/*  if the file is in the master unit
 */
        if (now_unit == my_unit)
        {

/*  Compare with the following filename:
 *  Factory_Default_Config.cfg
 *  startup1.cfg
 *  startup2.cfg
 */
            if (strcmp((char *)file_attr.file_name, SYS_DFLT_restartConfigFile) == 0)
            {
                continue;
            }
            if (strcmp((char *)file_attr.file_name, standard_file1) == 0)
            {
                continue;
            }
            if (strcmp((char *)file_attr.file_name, standard_file2) == 0)
            {
                continue;
            }

/*  Return FALSE( Fail ) if fail to delete the non-standard file
 */
            if (FS_DeleteFile(DUMMY_DRIVE ,file_attr.file_name) != FS_RETURN_OK)
            {
                SYSFUN_Debug_Printf("Error! Failed to delete non-standard config file\r\n");
                return FALSE;
            }
        }
    }

    return TRUE;
}

/* FUNCTION NAME : DBSYNC_TXT_MGR_SetSyncSlaveDirty
 * PURPOSE:
 *      Set autosync running-config dirty bit.
 *
 * INPUT:
 *      unit_id
 *      0       : set all slave dirty
 *      other   : set the unit_id slave dirty
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *              None.
 *
 * NOTES:
 *      None.
 *
 * HISTORY:
 *      2004/07/09  --  Reginald Ian, Created
 */
static void DBSYNC_TXT_MGR_SetSyncSlaveDirty( UI32_T unit_id )
{
    UI32_T num_unit = 0, my_unit = 0;
    UI32_T i        = 0;
    UI32_T    orig_priority;
    
    DBSYNC_TXT_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE( );
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection( dbsync_txt_mgr_sem_id );

    if( unit_id != 0 )
    {
        autosync_dirtybit[ unit_id - 1 ] = 1;
        DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
        DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE( );
    }

    STKTPLG_POM_GetNumberOfUnit( & num_unit );

    while( STKTPLG_POM_GetNextUnit( &i ) == TRUE )
    {
        autosync_dirtybit[ i - 1 ] = 1;
    }

    STKTPLG_POM_GetMyUnitID( & my_unit );
    autosync_dirtybit[ my_unit - 1 ] = 0;

    DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
    DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE( );
}


/* FUNCTION NAME : DBSYNC_TXT_MGR_SetDirty
 * PURPOSE  :   Set autosave running-config dirty bit.
 * INPUT    :   TRUE/FALSE.
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   set the autosave_timer to 1 second
 * HISTORY  :
 *      2004/07/09  --  Reginald Ian    Modify    Disable set dirty when flush
 */
void DBSYNC_TXT_MGR_SetDirty(UI32_T status)
{
    UI32_T    orig_priority;
    
    DBSYNC_TXT_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE( );
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection( dbsync_txt_mgr_sem_id );

    if( DBSYNC_TXT_MGR_FlushAndDisable( ) == FALSE && SwitchConfigAutosaveStatus == VAL_switchConfigAutosaveEnableStatus_enabled )
    {
        /*prevent when provision dirty bit will set to true.
        */
        if ( dbsync_txt_mgr_provision_complete == TRUE )
        {
            if ( status == TRUE )
            {
                autosave_dirtybit = TRUE;
                autosave_timer = SYS_DFLT_DBSYNC_TXT_INTERVAL * SYS_BLD_TICKS_PER_SECOND;
            }
            else
            {
                autosave_dirtybit = FALSE;
            }
        }
    }

    DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
    DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE( );
}

/* FUNCTION NAME : DBSYNC_TXT_MGR_SetStartupConfigAsDefault
 * PURPOSE:
 *      Set startup-config file as default config file.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *              TRUE/FALSE.
 *
 * NOTES:
 *      None.
 */
BOOL_T DBSYNC_TXT_MGR_SetStartupConfigAsDefault(UI8_T *filename)
{
    XFER_MGR_UserInfo_T  user_info;
    UI8_T  *xfer_buf;
    UI32_T xbuf_length;
    char startup_filename[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1] = {0};
#if (SYS_CPNT_DBSYNC_TXT == TRUE)
    FS_File_Attr_T file_attr;
    UI32_T my_unit = 0, now_unit = 0;
#endif
    BOOL_T is_find_file = FALSE;

#if (SYS_CPNT_DBSYNC_TXT == TRUE)
    if (TRUE == XFER_MGR_GetDualStartupCfgFileName((UI8_T *)startup_filename))
    {
        memset(&file_attr, 0, sizeof(FS_File_Attr_T));
        STKTPLG_POM_GetMyUnitID(&my_unit);
        file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_CONFIG);
        while(FS_GetNextFileInfo(&now_unit,&file_attr) == FS_RETURN_OK)
        {
            if (now_unit == my_unit)
            {
               /*for factory default*/
                if (strcmp((char *)file_attr.file_name, startup_filename) == 0)
                {
                    is_find_file = TRUE;
                    if(FS_SetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)startup_filename)!=FS_RETURN_OK)
                    {
                        return FALSE;
                    }
                    break;
                }
            }
        }/*end of while*/
    }
    else
#endif
    {
        sprintf(startup_filename, SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME, 1);
        is_find_file = FALSE;
    }
    /*file is not existed*/
    if (FALSE == is_find_file)
    {
        if( (xfer_buf=(UI8_T *)XFER_BUF_PMGR_Allocate()) == NULL)
        {
            return FALSE;
        }

        memset(&user_info, 0, sizeof(user_info));

        if (XFER_PMGR_LocalToStream(&user_info,
                                    (UI8_T *)SYS_DFLT_restartConfigFile,
                                    xfer_buf,
                                    &xbuf_length,
                                    SYS_ADPT_MAX_FILE_SIZE) != FS_RETURN_OK)
        {

             XFER_BUF_MGR_Free(xfer_buf);
              return FALSE;
        }


        if(FS_WriteFile(DUMMY_DRIVE,(UI8_T *)startup_filename, (UI8_T *)"Xfer", FS_FILE_TYPE_CONFIG, xfer_buf, strlen((char *)xfer_buf),0)!=FS_RETURN_OK)
        {
             XFER_BUF_MGR_Free(xfer_buf);
            return FALSE;
        }
        else
        {
             XFER_BUF_MGR_Free(xfer_buf);
        }

        if(FS_SetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)startup_filename)!=FS_RETURN_OK)
        {
            return FALSE;
        }
    }
    strcpy((char *)filename, startup_filename);
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_NotifyProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the autosave module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_NotifyProvisionComplete(void)
{
    dbsync_txt_mgr_provision_complete = TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_IsProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the autosave module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DBSYNC_TXT_MGR_IsProvisionComplete(void)
{
    return dbsync_txt_mgr_provision_complete;
}


#if 0
static void DBSYNC_TXT_MGR_SaveRunningCfg_Callback(UI32_T cookie, UI32_T status)
{
   if(status == XFER_MGR_FILE_COPY_COMPLETED)
   {
      is_doingautosave = FALSE;
   }

   if(!(status ==XFER_MGR_FILE_COPY_TFTP_COMPLETED||
      status ==XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH||
      status ==XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING||
      status ==XFER_MGR_FILE_COPY_SUCCESS||
      status ==XFER_MGR_FILE_COPY_COMPLETED))
   {
      autosave_dirtybit = TRUE;
   }
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Get_IsDoingAutosave
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell if is doing autosave
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DBSYNC_TXT_MGR_Get_IsDoingAutosave(void)
{
    DBSYNC_TXT_CHECK_OPER_MODE(FALSE);
    //DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id);
    //DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id);
    DBSYNC_TXT_RETURN_AND_RELEASE_CSC(is_doingautosave);
}

/*  FUNCTION NAME:  DBSYNC_TXT_MGR_Get_IsDoingAutosave
 *  PURPOSE     :   This function will tell if all dirty are clean.
 *  INPUT       :   None
 *  OUTPUT      :   None
 *  RETURN      :   TRUE/FALSE( success/fail )
 *  NOTE        :   None
 */
BOOL_T DBSYNC_TXT_MGR_Get_IsAllDirtyClear( void )
{
    UI32_T i = 0;

    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );

    if ( autosave_dirtybit == TRUE )
    {

#if 0
        DBSYNC_TXT_MGR_PrintDirtyBit( );
#endif

         
        return FALSE;
    }
    else if ( autosave_dirtybit == FALSE )
    {
        for ( i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i ++ )
        {
            if ( autosync_dirtybit[ i ] == 1 )
            {

#if 0
                DBSYNC_TXT_MGR_PrintDirtyBit( );
#endif

                 
                return FALSE;
            }
        }
    }
    else
    {

#if 0
        DBSYNC_TXT_MGR_PrintDirtyBit( );
#endif

         
        return TRUE;
    }

     
    return TRUE;
}

#if 0
static void DBSYNC_TXT_MGR_PrintDirtyBit( void )
{
    UI32_T i = 0;

    if( autosave_dirtybit == TRUE )
        SYSFUN_Debug_Printf( "Master is not clear.\r\n" );
    for( i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i ++ )
        if( autosync_dirtybit[ i ] == 1 )
            SYSFUN_Debug_Printf( "%ld is not clear.\r\n", i + 1 );

    return;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Set_IsDoingAutosave
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set if is doing autosave
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Set_IsDoingAutosave(BOOL_T status)
{
    UI32_T    orig_priority;
    DBSYNC_TXT_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id);
    if(status==TRUE)
    {
       is_doingautosave = TRUE;
    }
    else
    {
       is_doingautosave = FALSE;
    }
    DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id, orig_priority);
    DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE();
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_FlushAndDisable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return autosave_disable flag
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DBSYNC_TXT_MGR_FlushAndDisable(void)
{
    DBSYNC_TXT_CHECK_OPER_MODE(FALSE);
    //DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id);
    //DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id);
    DBSYNC_TXT_RETURN_AND_RELEASE_CSC(autosave_disable);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Set_FlushAndDisable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set autosave_disable flag
 *
 * INPUT   : TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Set_FlushAndDisable(BOOL_T status)
{
    DBSYNC_TXT_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();
    //DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id);

    if(status==TRUE)
    {
       autosave_disable = TRUE;
    }
    else
    {
       autosave_disable = FALSE;
    }

    //DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id);
    DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE( );
}

static UI32_T DBSYNC_TXT_MGR_GetNextSlaveDirtyAndClear( UI32_T * now_unit )
{
    UI32_T  my_unit = 0, num_unit = 0;
    UI32_T  return_state = 3;
    UI32_T  checktime = 0;
    UI32_T    orig_priority;
    
    DBSYNC_TXT_CHECK_OPER_MODE_WITH_RETURN_VALUE( FALSE );
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection( dbsync_txt_mgr_sem_id );

    STKTPLG_POM_GetMyUnitID( & my_unit );
    STKTPLG_POM_GetNumberOfUnit( & num_unit );

    /*  Get the nearest unit after the last sync that is not the master.
     *  Condition 1 :   autosync_dirtybit[ autosync_lastunit - 1 ] == 0
     *                  overleap the zero block
     *  Condition 2 :   autosync_lastunit == my_unit
     *                  check if it is master ( my_unit got above by STKTPLG lib )
     *  Condition 3 :   checktime <= num_unit
     *                  check all slave if it is dirty.
     *                  + 1 for the loop start when exceed
     *                  + 1 for the master jump over
     */
    if( STKTPLG_POM_GetNextUnit( &autosync_lastunit ) != TRUE )
    {
        autosync_lastunit = 0;
        STKTPLG_POM_GetNextUnit( &autosync_lastunit );
    }

    /*  Get the next dirty unit
     */
    while(  ( autosync_lastunit == my_unit )
            || ( autosync_dirtybit[ autosync_lastunit - 1 ] == 0 && checktime <= num_unit )
         )
    {
        if( STKTPLG_POM_GetNextUnit( &autosync_lastunit ) != TRUE )
        {
            autosync_lastunit = 0;
            STKTPLG_POM_GetNextUnit( &autosync_lastunit );
        }
        checktime ++;
    }

    if( autosync_dirtybit[ autosync_lastunit - 1 ] == 1 )
    {
        is_doingautosave = TRUE;

        if ( checktime > num_unit )
        {
            autosync_dirtybit[ autosync_lastunit - 1 ] = 0;
            *now_unit = 0;
        }
        else
        {

            /*  Clear the dirty bit
             */
            autosync_dirtybit[ autosync_lastunit - 1 ] = 0;
            *now_unit = autosync_lastunit;
        }

        if( autosave_disable == FALSE )
        {
            return_state = DBSYNC_RETURN_ACTIVE;
        }
        else
        {
            return_state = DBSYNC_RETURN_FLUSH;
        }
    }
    else
    {
        if( autosave_disable == FALSE )
        {
            return_state = DBSYNC_RETURN_CLOSE;
        }
        else
        {
            return_state = DBSYNC_RETURN_NOTHING;
        }
    }


    DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
     
    return return_state;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_GetDirtyAndClear
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get and clear autosave_dirtybit
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : DBSYNC_RETURN_CODE_E
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static UI32_T DBSYNC_TXT_MGR_GetDirtyAndClear(void)
{
    UI32_T    orig_priority;
    
    DBSYNC_TXT_CHECK_OPER_MODE_WITH_RETURN_VALUE(FALSE);
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id);
    if (autosave_dirtybit == TRUE && autosave_disable == FALSE)
    {
        is_doingautosave = TRUE;
        autosave_dirtybit = FALSE;
        DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id,orig_priority);
         
        return DBSYNC_RETURN_ACTIVE;
    }
    else if(autosave_dirtybit == TRUE && autosave_disable == TRUE)
    {
        is_doingautosave = TRUE;
        autosave_dirtybit = FALSE;
        DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id, orig_priority);
         
        return DBSYNC_RETURN_FLUSH;
    }
    else if(autosave_dirtybit == FALSE && autosave_disable == FALSE)
    {
        DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id, orig_priority);
         
        return DBSYNC_RETURN_CLOSE;
    }
    else if(autosave_dirtybit == FALSE && autosave_disable == TRUE)
    {
        DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id, orig_priority);
         
        return DBSYNC_RETURN_NOTHING;
    }
    else
    {
        DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id, orig_priority);
         
        return DBSYNC_RETURN_NOTHING;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Set_FlushAndDisable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set force_save_disable flag
 *
 * INPUT   : TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Set_ForceSaveDisable(BOOL_T status)
{
    DBSYNC_TXT_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();
    //DBSYNC_TXT_MGR_EnterCriticalSection(dbsync_txt_mgr_sem_id);
    if(status==TRUE)
    {
       force_save_disable = TRUE;
    }
    else
    {
       force_save_disable = FALSE;
    }
    //DBSYNC_TXT_MGR_LeaveCriticalSection(dbsync_txt_mgr_sem_id);
    DBSYNC_TXT_RETURN_WITHOUT_RETURN_VALUE();
}

/* Function : DBSYNC_TXT_MGR_ClearAllDirty
 * Purpose  : This function will clear all dirty
 * Input    : None
 * Output   : None
 * Return   :
              TRUE    :   success
              FALSE   :   fail
 * Note     : None
 */
BOOL_T DBSYNC_TXT_MGR_ClearAllDirty( void )
{
    UI32_T    orig_priority;
    
    if ( DBSYNC_TXT_MGR_ClearSyncSlaveDirty( ) == FALSE )
    {
        return FALSE;
    }

    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection( dbsync_txt_mgr_sem_id );

    autosave_dirtybit   = FALSE;

    DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
     
    return TRUE;
}

/* ROUTINE NAME - DBSYNC_TXT_MGR_ClearSyncSlaveDirty
 * FUNCTION: This function will clear all the slave dirty
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  :
            TRUE    :   success
            FALSE   :   fail
 * NOTE    : None
 */
static BOOL_T DBSYNC_TXT_MGR_ClearSyncSlaveDirty( void )
{
    UI32_T  i = 0;
    UI32_T    orig_priority;
    
    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection( dbsync_txt_mgr_sem_id );

    for( i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i ++ )
    {
        autosync_dirtybit[ i ] = 0;
    }

    DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
     
    return TRUE;
}
/* FUNCTION NAME : DBSYNC_TXT_SetSwitchConfigAutosaveEnableStatus
 * PURPOSE:
 *      Set the autosave status as enable.
 * INPUT:
 *      The status we want to set the autosave mechanism as.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE : Success
 *      FALSE : Failed.
 *
 * NOTES:
 *     None.
 */
BOOL_T DBSYNC_TXT_MGR_SetSwitchConfigAutosaveEnableStatus ( UI32_T status )
{
    UI32_T    orig_priority;
    
    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );
    orig_priority = DBSYNC_TXT_MGR_EnterCriticalSection( dbsync_txt_mgr_sem_id );

    if( status == 1 )
    {
        SwitchConfigAutosaveStatus = VAL_switchConfigAutosaveEnableStatus_enabled;
        DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
         
        return TRUE;
    }
    else if( status == 2 )
    {
        SwitchConfigAutosaveStatus = VAL_switchConfigAutosaveEnableStatus_disabled;
        DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
         
        return TRUE;
    }
    else
    {
        SwitchConfigAutosaveStatus = 0;
        DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
         
        return FALSE;
    }

    DBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
     
    return TRUE;
}

/* FUNCTION NAME : DBSYNC_TXT_GetSwitchConfigAutosaveEnableStatus
 * PURPOSE:
 *      Set the autosave status as enable
 *
 * INPUT:
 *      status : the buffer of for the current status return
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE : Success
 *      FALSE : Failed.
 *
 * NOTES:
 *     None.
 */
BOOL_T DBSYNC_TXT_MGR_GetSwitchConfigAutosaveEnableStatus ( UI32_T * status )
{
    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );

    if( SwitchConfigAutosaveStatus == VAL_switchConfigAutosaveEnableStatus_enabled )
    {
        *status = 1;
        //richDBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
         
        return TRUE;
    }
    else if( SwitchConfigAutosaveStatus == VAL_switchConfigAutosaveEnableStatus_disabled )
    {
        *status = 2;
         
        return TRUE;
    }
    else
    {
        *status = 0;
         
        return FALSE;
    }

     
    return TRUE;
}

/* FUNCTION NAME : DBSYNC_TXT_GetSwitchConfigAutosaveBusyStatus
 * PURPOSE:
 *      Get the status that if the autosave is busy
 *
 * INPUT:
 *      status : the buffer of for the current status return
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE : Success
 *      FALSE : Failed.
 *
 * NOTES:
 *     None.
 */
BOOL_T DBSYNC_TXT_MGR_GetSwitchConfigAutosaveBusyStatus ( UI32_T * status )
{
    DBSYNC_TXT_CHECK_OPER_MODE( FALSE );

    if( DBSYNC_TXT_MGR_Get_IsAllDirtyClear( ) == TRUE && DBSYNC_TXT_MGR_Get_IsDoingAutosave( ) == FALSE )
    {
        *status = VAL_switchConfigAutosaveBusyStatus_notBusy;
        //richDBSYNC_TXT_MGR_LeaveCriticalSection( dbsync_txt_mgr_sem_id, orig_priority );
         
        return TRUE;
    }
    else if( DBSYNC_TXT_MGR_Get_IsAllDirtyClear( ) == FALSE || DBSYNC_TXT_MGR_Get_IsDoingAutosave( ) == TRUE )
    {
        *status = VAL_switchConfigAutosaveBusyStatus_busy;
         
        return TRUE;
    }
    else
    {
        *status = 0;
         
        return FALSE;
    }

     
    return TRUE;
}

#if 0 /*maggie liu remove warning, should be open SYS_CPNT_DBSYNC_TXT == TRUE*/
/* ------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Backdoor_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION : This function supports a backdoor for the DBSync.
 * INPUT    : None
 * OUTPUT   : Node
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void DBSYNC_TXT_MGR_Backdoor_CallBack(void)
{
    UI8_T  key_in_string[16];

    while (TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("----------------------------------------------------------\r\n");
        BACKDOOR_MGR_Printf("Main menu of DBSync backdoor:\r\n\r\n");
        BACKDOOR_MGR_Printf("   1. Disable/Enable auto save running config. [%s]\r\n", force_save_disable ? "Disable" : "Enable");

        BACKDOOR_MGR_Printf("\r\n   0. To exit from backdoor.\r\n");
        BACKDOOR_MGR_Printf("----------------------------------------------------------\r\n");
        BACKDOOR_MGR_Printf("Key in the choice: ");
        memset(key_in_string, 0, sizeof(key_in_string));
        BACKDOOR_MGR_RequestKeyIn(key_in_string, 2);
        BACKDOOR_MGR_Printf("\r\n");

        switch(atoi((char *)key_in_string))
        {
        case 0:
            return;

        case 1:
            force_save_disable = !force_save_disable;
            break;

        default:
            BACKDOOR_MGR_Printf("Invalid choice.\r\n");
            break;
        }
        BACKDOOR_MGR_Printf("\r\nPress ENTER to continue ");
        BACKDOOR_MGR_RequestKeyIn(key_in_string, 0);
    }
    return;
}
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DBSYNC_TXT_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for SYSLOG MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    DBSYNC_TXT_MGR_IPCMsg_T *msg_p;
    BOOL_T need_resp = TRUE;
    
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding VLAN_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case DBSYNC_TXT_MGR_IPC_GETSWITCHCONFIGAUTOSAVEBUSYSTATUS:
        	msg_p->type.ret_bool =
                DBSYNC_TXT_MGR_GetSwitchConfigAutosaveBusyStatus(&msg_p->data.dbsync_txt_data.status);
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            break;

        case DBSYNC_TXT_MGR_IPC_GETSWITCHCONFIGAUTOSAVEENABLESTATUS:
        	msg_p->type.ret_bool =
                DBSYNC_TXT_MGR_GetSwitchConfigAutosaveEnableStatus(&msg_p->data.dbsync_txt_data.status);
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            break;
             
        case DBSYNC_TXT_MGR_IPC_GET_ISALLDIRTYCLEAR:
        	msg_p->type.ret_bool =
                DBSYNC_TXT_MGR_Get_IsDoingAutosave();
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            break;
            
            
        case DBSYNC_TXT_MGR_IPC_SETDIRTY:
            DBSYNC_TXT_MGR_SetDirty(msg_p->data.dbsync_txt_data.status);
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            need_resp = FALSE;
            break;        
        
        case DBSYNC_TXT_MGR_IPC_SETSWITCHCONFIGAUTOSAVEENABLESTATUS:
        	msg_p->type.ret_bool =
                DBSYNC_TXT_MGR_SetSwitchConfigAutosaveEnableStatus(msg_p->data.dbsync_txt_data.status);
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            break;
        
        case DBSYNC_TXT_MGR_IPC_SET_FLUSHANDDISABLE:
        	DBSYNC_TXT_MGR_Set_FlushAndDisable(msg_p->data.dbsync_txt_data.status);
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            need_resp = FALSE;
            break;
        
        case DBSYNC_TXT_MGR_IPC_SET_ISDOINGAUTOSAVE:
        	DBSYNC_TXT_MGR_Set_IsDoingAutosave(msg_p->data.dbsync_txt_data.status);
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
            need_resp = FALSE;
            break;
            
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE;
    }

    return need_resp;
} /* End of DBSYNC_TXT_MGR_HandleIPCReqMsg */

#endif /* End of #if (SYS_CPNT_DBSYNC_TXT == TRUE) */
