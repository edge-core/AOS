/* MODULE NAME: DEBUG_MGR.H
 * PURPOSE:
 *   Definitions for the Debug function.
 * NOTES:
 *
 * History:
 *   02/25/2008 -- Duo Chen, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 */

#ifndef _DEBUG_MGR_H
#define _DEBUG_MGR_H
#include "sysrsc_mgr.h"
#include "debug_type.h"



/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#define DEBUG_MGR_PRINTF_DFLT_WAIT_TIME     SYS_BLD_TICKS_PER_SECOND

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - DEBUG_MGR_InitiateSystemResources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            debug module.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_InitiateSystemResources(void);

 /*FUNCTION NAME - DEBUG_MGR_AttachSystemResources
 * PURPOSE  : Attach system resource for DEBUG in the context of the
 *            calling process.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 **/
void DEBUG_MGR_AttachSystemResources(void);


 /*FUNCTION NAME - DEBUG_MGR_GetShMemInfo
 * PURPOSE  : Provide shared memory information of DEBUG for SYSRSC.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 **/
void DEBUG_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*FUNCTION NAME - DEBUG_MGR_EnterMasterMode
 * PURPOSE  : This function will sets debug to enter master mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 **/
void DEBUG_MGR_EnterMasterMode(void);

/*FUNCTION NAME - DEBUG_MGR_EnterTransitionMode
 * PURPOSE  : This function sets debug to enter transition mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_EnterTransitionMode(void);

/*FUNCTION NAME - DEBUG_MGR_EnterSlaveMode
 * PURPOSE  : This function sets debug to enter slave mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void DEBUG_MGR_EnterSlaveMode(void);


/* FUNCTION NAME - DEBUG_MGR_SetTransitionMode
 * PURPOSE  : This function sets debug to temporary transition mode.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 */
void  DEBUG_MGR_SetTransitionMode(void);

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
    UI32_T class,
    UI32_T feature,
    const char *formatstring,
    ...);

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
    UI32_T flag);

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
    UI32_T flag);

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
    UI32_T *flag);

/* FUNCTION NAME: DEBUG_MGR_Get_Session_Bmp
 * PURPOSE: Get session bitmap for every session
 * INPUT  : csc_id   -- setting CSC name, defined in debug_type.h.
 *          session_id  -- which session would be printed out.
 *          matched_way -- the way to match Debug om value when CSCs want to
 *          print.
 *          class  -- setting flag value.
 *          feature  -- setting flag value, it's null when no use.
 *          session_bmp --
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_MGR_GetSessionBitMap(
    DEBUG_TYPE_CscId_E csc_id,
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class,
    UI32_T feature,
    UI32_T * session_bmp_p);


#endif /* End of _DEBUG_MGR.H */
