/* ------------------------------------------------------------------------
 * FILE NAME - DOS_PMGR.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             24/01/2011      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2011
 * ------------------------------------------------------------------------
 */
#ifndef DOS_PMGR_H
#define DOS_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "dos_type.h"
#include "dos_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_PMGR_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_SetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set data by field
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value to set
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 *                      specifies NULL to set to default value
 * OUTPUT : None.
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_PMGR_SetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_GetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_PMGR_GetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_GetRunningDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : SYS_TYPE_Get_Running_Cfg_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T DOS_PMGR_GetRunningDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

#endif /* DOS_PMGR_H */

