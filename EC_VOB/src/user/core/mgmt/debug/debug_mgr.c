/* MODULE NAME: DEBUG_OM.C
 * PURPOSE:
 *   Implementation for the DEBUG object manager
 * NOTES:
 *
 * HISTORY:
 *   12/12/2007 -- Kelly Chen, Create
 *   03/10/2008 -- Duo Chen, Modify
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <stdarg.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_stdlib.h"
#include "debug_type.h"
#include "debug_mgr.h"
#include "debug_om.h"
#include "l_ipcio.h"
#include "sys_bld.h"
#include "sys_module.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

#define DEBUG_MGR_EnterCriticalSection()
#define DEBUG_MGR_LeaveCriticalSection()
#define DEBUG_MGR_MAX_MSG_LEN 1500
#define DEBUG_MGR_WAITE_TIME 100

/* DATA TYPE DECLARATIONS
 */
#define DEBUG_MGR_MSG_RATE_LIMIT_PER_SEC 10

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T DEBUG_MGR_PrintRateLimit();

/* STATIC VARIABLE DEFINITIONS
 */
static SYSFUN_MsgQ_T debug_cli_msgq_handle;
static UI32_T debug_mgr_last_ticks;
static UI32_T debug_mgr_msg_count;

/* variables for semahphore */

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME - DEBUG_MGR_InitiateSystemResources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            debug module.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_InitiateSystemResources(void)
{
	DEBUG_OM_InitiateSystemResources();
}

 /*FUNCTION NAME - DEBUG_MGR_AttachSystemResources
 * PURPOSE  : Attach system resource for DEBUG in the context of the
 *            calling process.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 **/

void DEBUG_MGR_AttachSystemResources(void)
{
	DEBUG_OM_AttachSystemResources();
	SYSFUN_GetMsgQ(SYS_BLD_CLI_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
        &debug_cli_msgq_handle);
}

 /*FUNCTION NAME - DEBUG_MGR_GetShMemInfo
 * PURPOSE  : Provide shared memory information of DEBUG for SYSRSC.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 **/

void DEBUG_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
	DEBUG_OM_GetShMemInfo(segid_p, seglen_p);
}

/*FUNCTION NAME - DEBUG_MGR_EnterMasterMode
 * PURPOSE  : This function will sets debug to enter master mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_EnterMasterMode(void)
{
    DEBUG_OM_EnterMasterMode();

    DEBUG_OM_Init();
}

/*FUNCTION NAME - DEBUG_MGR_EnterTransitionMode
 * PURPOSE  : This function sets debug to enter transition mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_EnterTransitionMode(void)
{
    DEBUG_OM_EnterTransitionMode();
    SYSFUN_ENTER_TRANSITION_MODE();

}

/*FUNCTION NAME - DEBUG_MGR_EnterSlaveMode
 * PURPOSE  : This function sets debug to enter slave mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_EnterSlaveMode(void)
{
	DEBUG_OM_EnterSlaveMode();
}


/* FUNCTION NAME - DEBUG_MGR_SetTransitionMode
 * PURPOSE  : This function sets debug to temporary transition mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void  DEBUG_MGR_SetTransitionMode(void)
{
	DEBUG_OM_SetTransitionMode();
}


/* FUNCTION NAME: DEBUG_MGR_Printf
 * PURPOSE: Print a messsage for a specified CSC module.
 * INPUT  : csc_id  -- setting CSC name, defined in debug_type.h.
 *          matched_way -- the way to match Debug om value when CSCs want
 *          to print.
 *          class -- setting flag value.
 *          feature -- setting flag value, it's null when no use.
 *          formatstring -- any string would be printed out.
 *          ... -- optional args.
 * OUTPUT : None.
 * RETURN : DEBUG_TYPE_RETURN_OK.
 *          DEBUG_TYPE_RETURN_FAIL.
 *          DEBUG_TYPE_RETURN_INVALID_ARG.
 *          DEBUG_TYPE_RETURN_MASTER_MODE_ERROR.
 * NOTES  : Need to let input arguments to be a string. see "_snprintf" of c
 *          lib.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_MGR_Printf(
    DEBUG_TYPE_CscId_E csc_id,
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class, UI32_T feature,
    const char *formatstring,
    ...)
{
    UI32_T  len;
    va_list args;
    UI32_T session_bmp;
    L_IPCIO_UserInfo_T ipcio_user_info;


    if (DEBUG_OM_Get_Operating_Mode_On_Shmem() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DEBUG_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if(TRUE == DEBUG_MGR_PrintRateLimit())
    {
        return DEBUG_TYPE_RETURN_RATELIMIT;
    }
    
    DEBUG_MGR_EnterCriticalSection();

    if (DEBUG_MGR_GetSessionBitMap(csc_id, matched_way,
        class, feature, &session_bmp)== DEBUG_TYPE_RETURN_INVALID_ARG)
    {
        return DEBUG_TYPE_RETURN_INVALID_ARG;
    }

    if(session_bmp==0)
      return DEBUG_TYPE_RETURN_OK;

    ipcio_user_info.user_id = SYS_MODULE_DEBUG;
    ipcio_user_info.user_ext_id = session_bmp;
    va_start( args, formatstring );
    L_IPCIO_VPrintf(debug_cli_msgq_handle, &ipcio_user_info,
        DEBUG_MGR_WAITE_TIME, (char *)formatstring, args);
    va_end(args);
    DEBUG_MGR_LeaveCriticalSection();
    return DEBUG_TYPE_RETURN_OK;
}


/* FUNCTION NAME: DEBUG_MGR_EnableModuleFlag
 * PURPOSE: Enable debug flag for specified CSC module.
 * INPUT  : csc_id  -- setting CSC name, defined in debug_type.h.
 *          session_id -- which session would be printed out.
 *          flag -- setting flag value.
 * OUTPUT : None.
 * RETURN : DEBUG_TYPE_RETURN_OK.
 *          DEBUG_TYPE_RETURN_FAIL.
 *          DEBUG_TYPE_RETURN_INVALID_ARG.
 *          DEBUG_TYPE_RETURN_MASTER_MODE_ERROR.
 * NOTES  : Only provide for UI.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_MGR_EnableModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T flag)
{
    UI32_T tmp_flag;
    DEBUG_TYPE_ReturnValue_T ret;

    if (DEBUG_OM_Get_Operating_Mode_On_Shmem() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DEBUG_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    DEBUG_MGR_EnterCriticalSection();
    DEBUG_OM_GetModuleFlag(csc_id,session_id, &tmp_flag);
    tmp_flag|=flag;
    ret = DEBUG_OM_SetModuleFlag(csc_id,session_id,tmp_flag);
    DEBUG_MGR_LeaveCriticalSection();
    return ret;
}

/* FUNCTION NAME: DEBUG_MGR_DisableModuleFlag
 * PURPOSE: Disable debug flag for specified CSC module.
 * INPUT  : csc_id  -- setting CSC name, defined in debug_type.h.
 *          session_id -- which session would be printed out.
 *          flag -- setting flag value.
 * OUTPUT : None.
 * RETURN : DEBUG_TYPE_RETURN_OK.
 *          DEBUG_TYPE_RETURN_FAIL.
 *          DEBUG_TYPE_RETURN_INVALID_ARG.
 *          DEBUG_TYPE_RETURN_MASTER_MODE_ERROR.
 * NOTES  : Only provide for UI.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_MGR_DisableModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T flag)
{
    UI32_T tmp_flag;
    DEBUG_TYPE_ReturnValue_T ret;

    if (DEBUG_OM_Get_Operating_Mode_On_Shmem() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DEBUG_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    DEBUG_MGR_EnterCriticalSection();
    DEBUG_OM_GetModuleFlag(csc_id,session_id, &tmp_flag);
    tmp_flag&=(~flag);
    ret = DEBUG_OM_SetModuleFlag(csc_id,session_id,tmp_flag);
    DEBUG_MGR_LeaveCriticalSection();
    return ret;
}

/* FUNCTION NAME: DEBUG_MGR_GetModuleFlag
 * PURPOSE: Get debug flag for specified CSC module.
 * INPUT  : csc_id  -- setting CSC name, defined in debug_type.h.
 *          session_id -- which session would be printed out.
 * OUTPUT : flag -- setting flag value.
 * RETURN : DEBUG_TYPE_RETURN_OK.
 *          DEBUG_TYPE_RETURN_FAIL.
 *          DEBUG_TYPE_RETURN_INVALID_ARG.
 *          DEBUG_TYPE_RETURN_MASTER_MODE_ERROR.
 * NOTES  : Provide for UI.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_MGR_GetModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T *flag_p)
{
    DEBUG_TYPE_ReturnValue_T ret;

    if (DEBUG_OM_Get_Operating_Mode_On_Shmem() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DEBUG_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    DEBUG_MGR_EnterCriticalSection();
    ret = DEBUG_OM_GetModuleFlag(csc_id, session_id, flag_p);
    DEBUG_MGR_LeaveCriticalSection();
    return ret;
}

/* FUNCTION NAME: DEBUG_MGR_GetSessionBitMap
 * PURPOSE: Get session bitmap for every session
 * INPUT  : csc_id    -- setting CSC name, defined in debug_type.h.
 *          session_id   -- which session would be printed out.
 *          matched_way  -- the way to match Debug om value when CSCs want to
 *          print.
 *          class_mask   -- setting flag value.
 *          feature_mask -- setting flag value, it's null when no use.
 * OUTPUT : session_bmp -- session bitmap for every session
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_MGR_GetSessionBitMap(
    DEBUG_TYPE_CscId_E csc_id,
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class,
    UI32_T feature,
    UI32_T * session_bmp_p)
{
    UI32_T   session_id;

    if (DEBUG_TYPE_MAX_CSC_NUM <= csc_id)
    {
        return DEBUG_TYPE_RETURN_INVALID_ARG;
    }

    *session_bmp_p = 0;

    for (session_id=0; session_id<DEBUG_TYPE_MAX_SESSION_NUM; session_id++)
    {
        if (DEBUG_OM_IsPrintable(csc_id,session_id,matched_way, class,feature))
        {
            *session_bmp_p |= (1 << session_id);
        }
    }

    return DEBUG_TYPE_RETURN_OK;
}


static BOOL_T DEBUG_MGR_PrintRateLimit()
{
    UI32_T curr_ticks  = SYSFUN_GetSysTick();
    UI32_T passed_ticks=0;

    if (curr_ticks >= debug_mgr_last_ticks)
    {
        passed_ticks = (curr_ticks - debug_mgr_last_ticks);
    }
    else
    {
        passed_ticks = (0xFFFFFFFFUL - debug_mgr_last_ticks) + curr_ticks;
    }
    
    if( passed_ticks <= SYS_BLD_TICKS_PER_SECOND)
    {
        debug_mgr_msg_count++;    
        if(debug_mgr_msg_count > DEBUG_MGR_MSG_RATE_LIMIT_PER_SEC)
        {
            return TRUE;
        }
    }
    else
    {
        debug_mgr_last_ticks = curr_ticks;
        debug_mgr_msg_count  = 1;
    }

    return FALSE;
}

