/* MODULE NAME: pfc_om.h
 * PURPOSE:
 *   Declarations of OM APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
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

#ifndef _PFC_OM_H
#define _PFC_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "pfc_type.h"

#if (PFC_TYPE_BUILD_LINUX == TRUE)
#include "sysrsc_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (PFC_TYPE_BUILD_LINUX == TRUE)
/*---------------------------------------------------------------------------------
 * FUNCTION - PFC_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for PFC OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void PFC_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  -  shared memory segment id
 *          seglen_p -  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:   This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-------------------------------------------------------------------------
 */
void PFC_OM_GetShMemInfo(
    SYSRSC_MGR_SEGID_T  *segid_p,
    UI32_T              *seglen_p);

#endif /* #if (PFC_TYPE_BUILD_LINUX == TRUE) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_ClearOM
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_OM_ClearOM(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_InitOM
 *--------------------------------------------------------------------------
 * PURPOSE: To init the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_OM_InitOM(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_CopyPortConfigTo
 *--------------------------------------------------------------------------
 * PURPOSE: To copy one config entry from specified src_lport to dst_lport.
 * INPUT  : src_lport - 1-based source      lport to copy config from
 *          dst_lport - 1-based destination lport to copy config to
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PFC_OM_CopyPortConfigTo(
    UI32_T  src_lport,
    UI32_T  dst_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get the data by specified field id and lport.
 * INPUT  : lport    - lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 * OUTPUT : data_p   - pointer to output data
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all output data are represented in UI32_T
 *          2. caller need to guarantee the data length is enough
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_OM_GetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_SetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport - lport to set
 *                       0 - global data to set
 *                      >0 - lport  data to set
 *          field_id  - field id to set (PFC_TYPE_FieldId_E)
 *          data_p    - pointer to input data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all input data are represented in UI32_T
 *          2. caller need to guarantee the data length is enough
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_OM_SetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetPortCtrlPtrByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get pointer of port control record with specified lport.
 * INPUT  : lport - input lport (1-based)
 * OUTPUT : None
 * RETURN : NULL - failed
 *          pointer to content of port contrl record
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
PFC_TYPE_PortCtrlRec_T *PFC_OM_GetPortCtrlPtrByLport(
    UI32_T  lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_IsDbgFlagOn
 *-------------------------------------------------------------------------
 * PURPOSE: To test if specified debug flag is set or not.
 * INPUT  : flag_enum - debug flag enum id to test
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_OM_IsDbgFlagOn(
    UI32_T  flag_enum);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetDbgFlag
 *-------------------------------------------------------------------------
 * PURPOSE: To get the debug flag.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : debug flag
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
UI32_T PFC_OM_GetDbgFlag(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_SetDbgFlag
 *-------------------------------------------------------------------------
 * PURPOSE: To set the debug flag.
 * INPUT  : debug_flag - debug flag to set
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
void PFC_OM_SetDbgFlag(
    UI32_T  debug_flag);

#endif /* End of _PFC_OM_H */

