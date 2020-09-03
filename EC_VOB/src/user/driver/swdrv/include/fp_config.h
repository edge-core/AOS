/* FUNCTION NAME: FP_CONFIG.H
* PURPOSE:
*   1. Provide basic uniform interfaces for different slice distribution.
*
* NOTES:
*   {Something must be known or noticed}
*   {1. How to use these functions - Give an example.}
*   {2. Sequence of messages if applicable.}
*   {3. Any design limitation}
*   {4. Any performance limitation}
*   {5. Is it a reusable component}
*
*REASON:
*    DESCRIPTION: separate function information and group information.
*    CREATOR:   Jinfeng Chen
*    Date:      2008-10-08
*
* Copyright(C)      Accton Corporation, 2008
*/

#ifndef _FP_CONFIG_H
#define _FP_CONFIG_H

#include "rule_type.h"
#ifdef MARVELL_CPSS /* Yongxin.Zhao added, 2009-05-13, 15:22:55 */
#include <marvell/dev_rm.h>
#else
#include "dev_rm.h"
#endif


#define FP_CONFIG_MAX_DESC_STR_LEN                  48
#define FP_CONFIG_FLAG_GROUP_DELAY_CREATE           0x00000001
#define FP_CONFIG_FLAG_GROUP_AUTO_FREE              0x00000002

typedef struct
{
    DEVRM_FIELD_QUALIFY_T               fq;
    char                                fq_str[FP_CONFIG_MAX_DESC_STR_LEN+1];
} FP_CONFIG_FieldQualifyDescriptor_T;

typedef struct
{
    UI32_T                              count;
    FP_CONFIG_FieldQualifyDescriptor_T  *fq;
} FP_CONFIG_FieldQualifyVector_T;

typedef struct
{
    UI32_T                              chunk;
    UI32_T                              flags;
} FP_CONFIG_FieldUDFDescriptor_T;

typedef struct
{
    UI32_T                              count;
    FP_CONFIG_FieldUDFDescriptor_T      *udf;
} FP_CONFIG_FieldUDFVector_T;

typedef struct
{
    RULE_TYPE_FunctionType_T            func_type;
    int                                 rule_pri;
    int                                 rule_quota;
    char                                func_type_str[FP_CONFIG_MAX_DESC_STR_LEN+1];
    char                                rule_pri_str[FP_CONFIG_MAX_DESC_STR_LEN+1];
} FP_CONFIG_FunctionDescriptor_T;

typedef struct
{
    UI32_T                              count;
    FP_CONFIG_FunctionDescriptor_T      *fd;
} FP_CONFIG_FunctionVector_T;

typedef struct
{
    UI32_T                              group_id;
    UI32_T                              group_mode;
    UI32_T                              selector_stage;
    UI32_T                              first_selector_num;
    UI32_T                              flags;
    FP_CONFIG_FieldQualifyVector_T      fq_vector;
    FP_CONFIG_FieldUDFVector_T          udf_vector;
    FP_CONFIG_FunctionVector_T          func_vector;

    char                                group_mode_str[FP_CONFIG_MAX_DESC_STR_LEN+1];
    char                                selector_stage_str[FP_CONFIG_MAX_DESC_STR_LEN+1];
    char                                first_selector_num_str[FP_CONFIG_MAX_DESC_STR_LEN+1];
} FP_CONFIG_STRUCT_T;

typedef struct FP_CONFIG_FunctionInfo_S
{
    RULE_TYPE_FunctionType_T            function_type;
    UI32_T                              group_id;           /* array index */
    int                                 rule_pri;           /* rule priority:low, middle, high */
    UI32_T                              rule_quota;
} FP_CONFIG_FunctionInfo_T;

typedef struct FP_CONFIG_GroupInfo_S
{
    UI32_T                              group_id;           /* array index */
    UI32_T                              group_mode;         /* single wide, double wide */
    UI32_T                              selector_stage;     /* VFP, IFP, EFP */
    UI32_T                              selector_bitmap;    /* a group may include more than one slice. */
    UI32_T                              selector_count;     /* number of selectors within the group. */
    UI32_T                              flags;
    UI32_T                              w[DEVRM_SHR_BITDCLSIZE(DEVRM_FIELD_QUALIFY_Count)];/* field qualify bitmap */
    DEVRM_UDF_T                         udf[DEVRM_FIELD_USER_QUALIFY_Count];

    //
    // FIXME: need this flag ? or replace by created[] ?
    //
    BOOL_T                              is_created;

    //
    // group created status per unit/device
    // FIXME: move this variable into rule_ctrl/shmem
    //
    BOOL_T                              created[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]
                                               [SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT];
} FP_CONFIG_GroupInfo_T;

/*------------------------------------------------------------------------------
 * FUNCTION NAME: FP_CONFIG_GetShMemDataSize
 *------------------------------------------------------------------------------
 * PURPOSE: Get shared memory size of fp_config
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *------------------------------------------------------------------------------
 */
UI32_T FP_CONFIG_GetShMemDataSize();


/*------------------------------------------------------------------------------
 * FUNCTION NAME: FP_CONFIG_AttachSystemResources
 *------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for FP_CONFIG in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *------------------------------------------------------------------------------
 */
void FP_CONFIG_AttachSystemResources(void *shmem_start_p);


/*------------------------------------------------------------------------------
 * FUNCTION NAME: FP_CONFIG_Init
 *------------------------------------------------------------------------------
 * PURPOSE: Set OM of FP_CONFIG to default value.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *------------------------------------------------------------------------------
 */
void FP_CONFIG_Init();


/*------------------------------------------------------------------------------
 * FUNCTION NAME: FP_CONFIG_Dump
 *------------------------------------------------------------------------------
 * PURPOSE: Dump fp config information
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *------------------------------------------------------------------------------
 */
void FP_CONFIG_Dump();


UI32_T FP_CONFIG_NumberOfFunctionType();


/*------------------------------------------------------------------------------
 * FUNCTION NAME: FP_CONFIG_GetFunctionInfo
 *------------------------------------------------------------------------------
 * PURPOSE: Get function information by array index
 * INPUT:   index   - array index
 * OUTPUT:  None
 * RETUEN:  function infomation pointer
 * NOTES:
 *------------------------------------------------------------------------------
 */
FP_CONFIG_FunctionInfo_T * FP_CONFIG_GetFunctionInfo(UI32_T index);


/*------------------------------------------------------------------------------
 * FUNCTION NAME: FP_CONFIG_get_function_info_by_type
 *------------------------------------------------------------------------------
 * PURPOSE: Get function information by type
 * INPUT:   fun_type   - function type
 * OUTPUT:  None
 * RETUEN:  function infomation pointer
 * NOTES:
 *------------------------------------------------------------------------------
 */
FP_CONFIG_FunctionInfo_T * FP_CONFIG_get_function_info_by_type(RULE_TYPE_FunctionType_T fun_type);


UI32_T FP_CONFIG_NumberOfGroup();
FP_CONFIG_GroupInfo_T * FP_CONFIG_get_group_info_by_id(UI32_T group_id);

UI32_T FP_CONFIG_GetConfigSize();

FP_CONFIG_STRUCT_T * FP_CONFIG_GetConfig(UI32_T idx);

#endif  /* _FP_CONFIG_H */

