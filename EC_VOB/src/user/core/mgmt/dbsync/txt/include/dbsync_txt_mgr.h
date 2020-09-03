/* MODULE NAME:  dbsync_txt_mgr.h
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

#ifndef DBSYNC_TXT_MGR_H

#define DBSYNC_TXT_MGR_H

#include "sysfun.h"

#define DBSYNC_TXT_MGR_GET_MSGBUFSIZE(field_name)                       \
            (DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE +                        \
            sizeof(((DBSYNC_TXT_MGR_IPCMsg_T*)0)->data.field_name))

#define DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE sizeof(union DBSYNC_TXT_MGR_IpcMsg_Type_U)


/* INCLUDE FILE DECLARATIONS
 */
#include <sys_type.h>


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

enum DBSYNC_RETURN_CODE_E
{
    DBSYNC_RETURN_ACTIVE = 0,
    DBSYNC_RETURN_FLUSH,
    DBSYNC_RETURN_CLOSE,
    DBSYNC_RETURN_NOTHING,
};

typedef struct
{
    union DBSYNC_TXT_MGR_IpcMsg_Type_U
    {
        UI32_T cmd;    
        BOOL_T ret_bool; 
        UI32_T ret_ui32;
    } type;

    union
    {
        struct DBSYNC_TXT_MGR_IPCMSG_DATA_S
        {
            UI32_T status;
        } dbsync_txt_data; 
       
    } data; /* contains the supplemntal data for the corresponding cmd */
} DBSYNC_TXT_MGR_IPCMsg_T;

/* definitions of command which will be used in ipc message
 */
enum
{
    DBSYNC_TXT_MGR_IPC_GETSWITCHCONFIGAUTOSAVEBUSYSTATUS,
    DBSYNC_TXT_MGR_IPC_GETSWITCHCONFIGAUTOSAVEENABLESTATUS,
    DBSYNC_TXT_MGR_IPC_GET_ISALLDIRTYCLEAR,
    DBSYNC_TXT_MGR_IPC_SETDIRTY,
    DBSYNC_TXT_MGR_IPC_SETSWITCHCONFIGAUTOSAVEENABLESTATUS,
    DBSYNC_TXT_MGR_IPC_SET_FLUSHANDDISABLE,
    DBSYNC_TXT_MGR_IPC_SET_ISDOINGAUTOSAVE
    
};
/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T DBSYNC_TXT_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DBSYNC_TXT_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Create_InterCSC_Relation(void);

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
BOOL_T DBSYNC_TXT_MGR_EnterMasterMode(void);



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
BOOL_T DBSYNC_TXT_MGR_EnterTransitionMode(void);



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
void DBSYNC_TXT_MGR_EnterSlaveMode(void);



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
void DBSYNC_TXT_MGR_SetTransitionMode(void);



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
SYS_TYPE_Stacking_Mode_T DBSYNC_TXT_MGR_GetOperationMode(void);

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
BOOL_T DBSYNC_TXT_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* FUNCTION NAME : DBSYNC_TXT_MGR_SyncSlaveCfg
 * PURPOSE:
 *      Check dirty bit if sync running-config.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      TRUE : success
 *      FALSE: fail for some reason
 *
 * RETURN:
 *              None.
 *
 * NOTES:
 *      None.
 */
BOOL_T DBSYNC_TXT_MGR_SyncSlaveCfg( void );

/* FUNCTION NAME : DBSYNC_TXT_MGR_SaveRunningCfg
 * PURPOSE:
 *      Check dirty bit if save running-config.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      TRUE : success
 *              FALSE: fail for some reason
 *
 * RETURN:
 *              None.
 *
 * NOTES:
 *      None.
 */
BOOL_T DBSYNC_TXT_MGR_SaveRunningCfg( void );




/* FUNCTION NAME : DBSYNC_TXT_MGR_SetDirty
 * PURPOSE:
 *      Set autosave running-config dirty bit.
 *
 * INPUT:
 *      TRUE/FALSE.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *              None.
 *
 * NOTES:
 *      None.
 */
void DBSYNC_TXT_MGR_SetDirty(UI32_T status);


/* FUNCTION NAME : DBSYNC_TXT_MGR_SetStartupConfigAsDefault
 * PURPOSE:
 *      Set startup-config file as default config file.
 *      And get startup filename
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      UI8_T *filename.
 *
 * RETURN:
 *              TRUE/FALSE.
 *
 * NOTES:
 *      Filename length SYS_ADPT_FILE_SYSTEM_NAME_LEN+1.
 */
BOOL_T DBSYNC_TXT_MGR_SetStartupConfigAsDefault(UI8_T *filename);


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
void DBSYNC_TXT_MGR_NotifyProvisionComplete(void);

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
BOOL_T DBSYNC_TXT_MGR_IsProvisionComplete(void);

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
BOOL_T DBSYNC_TXT_MGR_Get_IsDoingAutosave(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Get_IsAllDirtyClear
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell if all dirty bit is cleared
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DBSYNC_TXT_MGR_Get_IsAllDirtyClear( void );

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
void DBSYNC_TXT_MGR_Set_IsDoingAutosave(BOOL_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_FlushAndDisable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will send an event to do autosave
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DBSYNC_TXT_MGR_FlushAndDisable(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Set_FlushAndDisable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set flushanddisable flag
 *
 * INPUT   : TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Set_FlushAndDisable(BOOL_T status);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - DBSYNC_TXT_MGR_Set_ForceSaveDisable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set flushanddisable flag
 *
 * INPUT   : TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void DBSYNC_TXT_MGR_Set_ForceSaveDisable(BOOL_T status);

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
BOOL_T DBSYNC_TXT_MGR_SetSwitchConfigAutosaveEnableStatus ( UI32_T status );

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
BOOL_T DBSYNC_TXT_MGR_GetSwitchConfigAutosaveEnableStatus ( UI32_T * status );

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
BOOL_T DBSYNC_TXT_MGR_GetSwitchConfigAutosaveBusyStatus ( UI32_T * status );

/* Function : DBSYNC_TXT_MGR_ClearAllDirty
 * Purpose  : This function will clear all dirty
 * Input    : None
 * Output   : None
 * Return   :
              TRUE    :   success
              FALSE   :   fail
 * Note     : None
 */
BOOL_T DBSYNC_TXT_MGR_ClearAllDirty( void );

#endif /* #ifndef DBSYNC_TXT_MGR_H */
