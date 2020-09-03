/* MODULE NAME: DEBUG_OM.H
 * PURPOSE:
 *   Definitions for the Debug object manager
 * NOTES:
 *
 * History:
 *   12/12/2007 -- Kelly Chen, Create
 *   02/25/2008 -- Duo Chenn, Modify
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef _DEBUG_OM_H
#define _DEBUG_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysrsc_mgr.h"
#include "debug_type.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: DEBUG_OM_Get_Operating_Mode_On_Shmem
 * PURPOSE: DEBUG_OM_Get_Operating_Mode_On_Shmem
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
SYS_TYPE_Stacking_Mode_T DEBUG_OM_Get_Operating_Mode_On_Shmem(void);

/* FUNCTION NAME: DEBUG_OM_InitiateSystemResources
 * PURPOSE: Initialize DEBUG database
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void  DEBUG_OM_InitiateSystemResources(void);

/* FUNCTION NAME: DEBUG_OM_AttachSystemResources
 * PURPOSE : Attach system resource for DEBUG in the context of the
 *           calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */

void DEBUG_OM_AttachSystemResources(void);

/* FUNCTION NAME: DEBUG_OM_GetShMemInfo
 * PURPOSE: Provide shared memory information of DEBUG for SYSRSC.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */

void DEBUG_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* FUNCTION NAME: DEBUG_OM_SetTransitionMode
 * PURPOSE: This function will set the transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */

void DEBUG_OM_SetTransitionMode(void);

/* FUNCTION NAME: DEBUG_OM_EnterTransitionMode
 * PURPOSE: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */

 void DEBUG_OM_EnterTransitionMode(void);

/* FUNCTION NAME: DEBUG_OM_EnterMasterMode
 * PURPOSE: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */

 void DEBUG_OM_EnterMasterMode(void);

/* FUNCTION NAME: DEBUG_OM_EnterSlaveMode
 * PURPOSE: compare flag by csc.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */

void DEBUG_OM_EnterSlaveMode(void);

/* FUNCTION NAME: DEBUG_OM_Init
 * PURPOSE: Initialize DEBUG database.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void  DEBUG_OM_Init(void);

/* FUNCTION NAME: DEBUG_OM_SetModuleFlag
 * PURPOSE: Set debug flag for specified CSC module.
 * INPUT  : module_id  -- setting CSC name.
 *          session_id -- which session would be printed out.
 *          flag -- setting flag value.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : Just set flag to OM table.
 *          Don't need return value because DEBUG_MGR will check input argument
 *          is valid or not.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_OM_SetModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T flag);

/* FUNCTION NAME: DEBUG_OM_GetModuleFlag
 * PURPOSE: Get debug flag for specified CSC module.
 * INPUT  : module_id  -- setting CSC name.
 *          session_id -- which session would be printed out.
 * OUTPUT : flag -- setting flag value.
 * RETURN : None.
 * NOTES  : Just get flag from OM table.
 *          Don't need return value because DEBUG_MGR will check input argument
 *          is valid or not.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_OM_GetModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T *flag);

/* FUNCTION NAME: DEBUG_OM_IsPrintable
 * PURPOSE: Detect if flag is on or off by session.
 * INPUT  : module_id  -- setting CSC name, defined in debug_type.h.
 *          session_id -- which session would be printed out.
 *          matched_way -- the way to match Debug om value when CSCs want to
 *          print.
 *          class_mask -- setting flag value.
 *          feature_mask -- setting flag value, it's null when no use.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */

BOOL_T
DEBUG_OM_IsPrintable(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class,
    UI32_T feature);



#endif /* End of DEBUG_OM.H */
