/* MODULE NAME:  DBSYNC_TXT_pmgr.h
 * PURPOSE:
 *    PMGR implement for DBSYNC_TXT.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    07/30/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef DBSYNC_TXT_PMGR_H
#define DBSYNC_TXT_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveBusyStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_GetSwitchConfigAutosaveBusyStatus through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_MGR_GetSwitchConfigAutosaveBusyStatus ( UI32_T * status );

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveEnableStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_GetSwitchConfigAutosaveEnableStatus through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveEnableStatus ( UI32_T * status );

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Get_IsAllDirtyClear
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Get_IsAllDirtyClear through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Get_IsAllDirtyClear( void );

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Get_IsAllDirtyClear
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Get_IsAllDirtyClear through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Get_IsAllDirtyClear( void );

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Get_IsDoingAutosave
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Get_IsDoingAutosave through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Get_IsDoingAutosave(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_SetDirty
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_SetDirty through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DBSYNC_TXT_PMGR_SetDirty(UI32_T status);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_SetSwitchConfigAutosaveEnableStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_SetSwitchConfigAutosaveEnableStatus through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_MGR_SetSwitchConfigAutosaveEnableStatus ( UI32_T status );

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Set_FlushAndDisable
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Set_FlushAndDisable through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Set_FlushAndDisable(BOOL_T status);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Set_IsDoingAutosave
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Set_IsDoingAutosave through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Set_IsDoingAutosave(BOOL_T status);
#endif

