/* ------------------------------------------------------------------------
 * FILE NAME - DOS_OM.C
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
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysrsc_mgr.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_DOS == TRUE)
#include "dos_type.h"
#include "dos_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */
enum DOS_OM_FIELD_STORAGE_E {
    DOS_OM_FIELD_STORAGE__DOS_OM_SystemInfo_T,
};


/* TYPE DEFINITIONS
 */
typedef struct {
    UI8_T type;     /* DOS_OM_FIELD_STORAGE_E */
    UI8_T offset;
    UI8_T length;
} DOS_OM_FieldInfo_T;

typedef struct
{
    UI32_T echo_chargen_ratelimit;  /* in kbits/s */
    UI32_T tcp_flooding_ratelimit;  /* in kbits/s */
    UI32_T udp_flooding_ratelimit;  /* in kbits/s */
    UI32_T win_nuke_ratelimit;      /* in kbits/s */

    UI8_T echo_chargen_status;
    UI8_T land_status;
    UI8_T smurf_status;
    UI8_T tcp_flooding_status;
    UI8_T tcp_null_scan_status;
    UI8_T tcp_scan_status;
    UI8_T tcp_syn_fin_scan_status;
    UI8_T tcp_udp_port_zero_status;
    UI8_T tcp_xmas_scan_status;
    UI8_T udp_flooding_status;
    UI8_T win_nuke_status;
} DOS_OM_SystemInfo_T;

typedef struct
{
    DOS_OM_FieldInfo_T field_info[DOS_TYPE_FLD_NUM];
    DOS_OM_SystemInfo_T system_info;
    UI32_T debug_flags;
} DOS_OM_Shmem_Data_T;

/* MACRO DEFINITIONS
 */

/* LOCAL FUNCTIONS DECLARATIONS
 */

/* LOCAL VARIABLES DECLARATIONS
 */
static DOS_OM_Shmem_Data_T *dos_shmem_data_p;

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
void DOS_OM_InitiateSystemResources(void)
{
    dos_shmem_data_p = (DOS_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_DOS_SHMEM_SEGID);
    DOS_OM_Init();
}

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
void DOS_OM_AttachSystemResources(void)
{
    dos_shmem_data_p = (DOS_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_DOS_SHMEM_SEGID);
}

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
void DOS_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_DOS_SHMEM_SEGID;
    *seglen_p = sizeof(DOS_OM_Shmem_Data_T);
}

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
BOOL_T DOS_OM_Init(void)
{
#define INIT_OM_FIELD(fld_id, t, m) do { \
        dos_shmem_data_p->field_info[fld_id].type = DOS_OM_FIELD_STORAGE__##t;  \
        dos_shmem_data_p->field_info[fld_id].offset = (UI8_T)(UI32_T)&(((t *)0)->m);    \
        dos_shmem_data_p->field_info[fld_id].length = sizeof(((t *)0)->m);      \
    } while (0)

    memset(dos_shmem_data_p, 0, sizeof(*dos_shmem_data_p));

    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS,      DOS_OM_SystemInfo_T, echo_chargen_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT,   DOS_OM_SystemInfo_T, echo_chargen_ratelimit);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_LAND_STATUS,              DOS_OM_SystemInfo_T, land_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_SMURF_STATUS,             DOS_OM_SystemInfo_T, smurf_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS,      DOS_OM_SystemInfo_T, tcp_flooding_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT,   DOS_OM_SystemInfo_T, tcp_flooding_ratelimit);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS,     DOS_OM_SystemInfo_T, tcp_null_scan_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS,          DOS_OM_SystemInfo_T, tcp_scan_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS,  DOS_OM_SystemInfo_T, tcp_syn_fin_scan_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS, DOS_OM_SystemInfo_T, tcp_udp_port_zero_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS,     DOS_OM_SystemInfo_T, tcp_xmas_scan_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS,      DOS_OM_SystemInfo_T, udp_flooding_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT,   DOS_OM_SystemInfo_T, udp_flooding_ratelimit);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS,          DOS_OM_SystemInfo_T, win_nuke_status);
    INIT_OM_FIELD(DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT,       DOS_OM_SystemInfo_T, win_nuke_ratelimit);

    return DOS_OM_Reset();
}

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
BOOL_T DOS_OM_Reset(void)
{
    memset(&dos_shmem_data_p->system_info, 0, sizeof(dos_shmem_data_p->system_info));

    return TRUE;
}

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
DOS_TYPE_FieldType_T DOS_OM_GetDataTypeByField(DOS_TYPE_FieldId_T field_id)
{
    return DOS_TYPE_FTYPE_UI32;
}

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
BOOL_T DOS_OM_SetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    void *om_data_p;
    void *type_base_p = NULL;
    size_t type_size = 0;
    size_t om_data_size;

    if (field_id >= DOS_TYPE_FLD_NUM)
    {
        return FALSE;
    }

    switch (dos_shmem_data_p->field_info[field_id].type)
    {
        case DOS_OM_FIELD_STORAGE__DOS_OM_SystemInfo_T:
            type_base_p = &dos_shmem_data_p->system_info;
            type_size = sizeof(dos_shmem_data_p->system_info);
            break;

        default:
            return FALSE;
    }

    if (type_base_p == NULL ||
        type_size < dos_shmem_data_p->field_info[field_id].offset +
                    dos_shmem_data_p->field_info[field_id].length)
    {
        return FALSE;
    }

    om_data_p = (UI8_T *)type_base_p + dos_shmem_data_p->field_info[field_id].offset;
    om_data_size = dos_shmem_data_p->field_info[field_id].length;


    switch (DOS_OM_GetDataTypeByField(field_id))
    {
        case DOS_TYPE_FTYPE_UI32:
            switch (om_data_size)
            {
                case 1:
                    *(UI8_T *)om_data_p = (UI8_T)*(UI32_T *)data_p;
                    break;
                case 2:
                    *(UI16_T *)om_data_p = (UI16_T)*(UI32_T *)data_p;
                    break;
                case 4:
                default:
                    *(UI32_T *)om_data_p = *(UI32_T *)data_p;
                    break;
            }
            break;

        default:
            memcpy(om_data_p, data_p, om_data_size);
    }

    return TRUE;
}

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
BOOL_T DOS_OM_GetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    void *om_data_p;
    void *type_base_p = NULL;
    size_t type_size = 0;
    size_t om_data_size;

    if (data_p == NULL)
    {
        return FALSE;
    }

    if (field_id >= DOS_TYPE_FLD_NUM)
    {
        return FALSE;
    }

    switch (dos_shmem_data_p->field_info[field_id].type)
    {
        case DOS_OM_FIELD_STORAGE__DOS_OM_SystemInfo_T:
            type_base_p = &dos_shmem_data_p->system_info;
            type_size = sizeof(dos_shmem_data_p->system_info);
            break;

        default:
            return FALSE;
    }

    if (type_base_p == NULL ||
        type_size < dos_shmem_data_p->field_info[field_id].offset +
                    dos_shmem_data_p->field_info[field_id].length)
    {
        return FALSE;
    }

    om_data_p = (UI8_T *)type_base_p + dos_shmem_data_p->field_info[field_id].offset;
    om_data_size = dos_shmem_data_p->field_info[field_id].length;

    switch (DOS_OM_GetDataTypeByField(field_id))
    {
        case DOS_TYPE_FTYPE_UI32:
            switch (om_data_size)
            {
                case 1:
                    *(UI32_T *)data_p = *(UI8_T *)om_data_p;
                    break;
                case 2:
                    *(UI32_T *)data_p = *(UI16_T *)om_data_p;
                    break;
                case 4:
                default:
                    *(UI32_T *)data_p = *(UI32_T *)om_data_p;
                    break;
            }
            break;

        default:
            memcpy(data_p, om_data_p, om_data_size);
    }

    return TRUE;
}

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
void DOS_OM_SetDebugFlag(DOS_TYPE_DbgFlag_T flag, BOOL_T is_enable)
{
    if (is_enable)
    {
        dos_shmem_data_p->debug_flags |= BIT_VALUE(flag);
    }
    else
    {
        dos_shmem_data_p->debug_flags &= ~ BIT_VALUE(flag);
    }
}

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
BOOL_T DOS_OM_IsDebugFlagOn(DOS_TYPE_DbgFlag_T flag)
{
    return !!(dos_shmem_data_p->debug_flags & BIT_VALUE(flag));
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

#endif /* (SYS_CPNT_DOS == TRUE) */


