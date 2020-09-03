/* MODULE NAME: pfc_pmgr.h
 *   Declarations of MGR IPC APIs for PFC
 *   (Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _PFC_PMGR_H
#define _PFC_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "pfc_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PFC_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for PFC_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T PFC_PMGR_InitiateProcessResource(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_PMGR_GetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get the data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to output data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all output data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_PMGR_GetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetRunningDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 * OUTPUT : data_p   - pointer to output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : 1. all output data are represented in UI32_T
 *--------------------------------------------------------------------------
 */
UI32_T PFC_PMGR_GetRunningDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_PMGR_SetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_PMGR_SetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

#endif /* #ifndef _PFC_PMGR_H */

