/* ------------------------------------------------------------------------
 * FILE NAME - DOS_OM.H
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
#ifndef DOS_OM_H
#define DOS_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sysrsc_mgr.h"
#include "dos_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_InitiateSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void DOS_OM_InitiateSystemResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void DOS_OM_AttachSystemResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_GetShMemInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-----------------------------------------------------------------------------
 */
void DOS_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init OM resouce
 * INPUT  : use_default -- set with default value
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_OM_Init(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_Reset
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will reset OM
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_OM_Reset(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_GetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data type of field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 * OUTPUT : None.
 * RETURN : DOS_TYPE_FieldType_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_FieldType_T DOS_OM_GetDataTypeByField(DOS_TYPE_FieldId_T field_id);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_SetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set data by field
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- value to set
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_OM_SetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_GetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 * OUTPUT : data_p   - value of the field
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_OM_GetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_SetDebugFlag
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set debug flag
 * INPUT  : flag      - DOS_TYPE_DbgFlag_T
 *          is_enable - TRUE to set; FALSE to clear
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void DOS_OM_SetDebugFlag(DOS_TYPE_DbgFlag_T flag, BOOL_T is_enable);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_OM_IsDebugFlagOn
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set debug flag
 * INPUT  : flag      - DOS_TYPE_DbgFlag_T
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_OM_IsDebugFlagOn(DOS_TYPE_DbgFlag_T flag);

#endif /* DOS_OM_H */

