/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_bld.h"

#include "sysfun.h"
#include "l_cvrt.h"
#include "l_pt.h"

#include "sysrsc_mgr.h"
#include "backdoor_mgr.h"

#include "fs_om.h"
#include "fs_type.h"
#include "fs.h"

#define FS_DEBUG

/* NAMING CONSTANT DECLARATIONS
 */
#define FS_GET_MTD_ID_FROM_SCRIPT_CMD_BUF_SIZE 80
/* FS_GET_MTD_ID_FROM_SCRIPT_FILENAME
 *     The script file name of the script used to get the mtd id by its
 *     mtd name. This script is used when SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT is
 *     defined as TRUE
 */
#define FS_GET_MTD_ID_FROM_SCRIPT_FILENAME "/etc/get_mtd_id_by_name.sh"

#ifdef INCLUDE_DIAG
#undef SYS_CPNT_STACKING
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define FS_OM_ENTER_CRITICAL_SECTION() 
#define FS_OM_LEAVE_CRITICAL_SECTION() 
#define ATOM_EXPRESSION(exp) 

#define L_PT_ShMem_Allocate(desc)            malloc(sizeof(FS_FileHeader_T))
#define L_PT_ShMem_Free(desc, header)    (free(header),TRUE)

/* DATA TYPE DECLARATIONS
 */
typedef struct 
{
#ifndef INCLUDE_DIAG
    SYSFUN_DECLARE_CSC_ON_SHMEM
#endif
    UI32_T                  FS_InitStatus;
    UI32_T                  FS_Semaphore;
    UI32_T                  FS_TaskId; //  = 0
    
//    UI32_T                  FS_TASK_MsgqId;
    I32_T                   FS_FileHeaderList_offset;
//    FS_FileHeader_T         *FS_FileHeaderList; //  = NULL
    UI32_T                  fs_startup_offset[FS_FILE_TYPE_TOTAL];
    BOOL_T                  fs_shutdown_flag;
    BOOL_T                  fs_all_file_checksum_verified; // = FALSE
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    BOOL_T                  onie_installer_debug_flag;
#endif
    BOOL_T                  fs_debug_flag;
    I32_T                   fs_file_header_head_offset;
    L_PT_ShMem_Descriptor_T fs_file_header_pool_desc;
    FS_FileHeader_T         fs_file_header_buffer[FS_MAX_FILE_NUMBER];
    
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  FS_Communication_Sem;
    FS_Control_T            fs_control;
#endif
#if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT==TRUE)
    UI8_T                   fs_mtd_type_to_mtd_id_ar[FS_MTD_PART_MAX_NUMBER]; /* invalid entry is fileed with FS_TYPE_INVALID_MTD_ID */
#endif
#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
    FS_DataStorage_Control_T fs_data_storage_ctl;
#endif
}  FS_Shmem_Data_T;

static FS_Shmem_Data_T fs_shmem_data_global;
static FS_Shmem_Data_T *fs_shmem_data_p = &fs_shmem_data_global;

BOOL_T FS_OM_GetDebugFlag(void)
{
    return TRUE;
}

FS_FileHeader_T *FS_OM_AllocateFileHeaderBuffer(void)
{
    FS_FileHeader_T *ret;

    FS_OM_ENTER_CRITICAL_SECTION();
    ret = (FS_FileHeader_T *)L_PT_ShMem_Allocate(&fs_shmem_data_p->fs_file_header_pool_desc);
    FS_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

void FS_OM_AddFileHeader(FS_FileHeader_T *file_header_p)
{
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;
    FS_FileHeader_T *prev_header_p;
    BOOL_T          stop;

    FS_OM_ENTER_CRITICAL_SECTION();
    if (fs_shmem_data_p->fs_file_header_head_offset == 0)
    {
        file_header_p->next_offset = 0;
        fs_shmem_data_p->fs_file_header_head_offset = L_CVRT_GET_OFFSET(fs_shmem_data_p,file_header_p);
    }
    else
    {
        this_header_offset = fs_shmem_data_p->fs_file_header_head_offset;
        prev_header_p = NULL;
        stop          = FALSE;
        while ( (!stop) && (this_header_offset != 0) )
        {
            this_header_p = L_CVRT_GET_PTR(fs_shmem_data_p, this_header_offset);
            if (strcmp((char*)this_header_p->file_name, (char*)file_header_p->file_name) == 0)
            {
                ERRORSTR("\r\n%s:ERROR, duplicate file name add",__FUNCTION__);
                FS_OM_LEAVE_CRITICAL_SECTION();
                return;
            }
            
            if (strcmp((char*)this_header_p->file_name, (char*)file_header_p->file_name) > 0)
            {
                stop = TRUE;
            }
            else
            {
                prev_header_p      = this_header_p;
                this_header_offset = this_header_p->next_offset;
            }
        }
        if (prev_header_p == NULL)
        {
            file_header_p->next_offset = this_header_offset;
            fs_shmem_data_p->fs_file_header_head_offset = L_CVRT_GET_OFFSET(fs_shmem_data_p,file_header_p);
        }
        else
        {
            file_header_p->next_offset = this_header_offset;
            prev_header_p->next_offset = L_CVRT_GET_OFFSET(fs_shmem_data_p,file_header_p);
        }        
    }
    FS_OM_LEAVE_CRITICAL_SECTION();
    return;
}

BOOL_T FS_OM_GetFileHeaderByName(FS_FileHeader_T *file_header_p, UI32_T type_mask)
{
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;
    I32_T           compare_result;

    FS_OM_ENTER_CRITICAL_SECTION();
    if (fs_shmem_data_p->fs_file_header_head_offset == 0)
    {
        FS_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    this_header_offset = fs_shmem_data_p->fs_file_header_head_offset;
    while (this_header_offset != 0) 
    {	
        this_header_p = L_CVRT_GET_PTR(fs_shmem_data_p, this_header_offset);
        compare_result = strcmp((char*)this_header_p->file_name, (char*)file_header_p->file_name);
        
        if ((FS_FILE_TYPE_MASK(this_header_p->file_type) & type_mask) &&
            ( compare_result == 0))
        {
            memcpy(file_header_p,this_header_p,sizeof(FS_FileHeader_T));
            FS_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        };
        
        if (compare_result > 0)
        {
            break;
        }
    else
        {
            this_header_offset = this_header_p->next_offset;
        }        
    }
    FS_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

BOOL_T FS_OM_RemoveFileHeaderByName(UI8_T file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1])
{
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;
    FS_FileHeader_T *prev_header_p;
    BOOL_T          stop;
    I32_T           compare_result;
    BOOL_T          ret;

    FS_OM_ENTER_CRITICAL_SECTION();
    this_header_offset = fs_shmem_data_p->fs_file_header_head_offset;
    prev_header_p = NULL;
    stop          = FALSE;
    while ( (!stop) && (this_header_offset != 0) )
    {
        this_header_p  = L_CVRT_GET_PTR(fs_shmem_data_p, this_header_offset);
        compare_result = strcmp((char*)this_header_p->file_name, (char*)file_name); 
        if ( compare_result == 0)
        {
            if (prev_header_p == NULL)
            {
                fs_shmem_data_p->fs_file_header_head_offset = this_header_p->next_offset;
            }
            else
            {
                prev_header_p->next_offset = this_header_p->next_offset;
            }
            if (TRUE == this_header_p->startup)
            {
                /* If it is a startup file, remove it from startup file list too
                 */
                if (fs_shmem_data_p->fs_startup_offset[this_header_p->file_type] == this_header_offset)
                {
                    fs_shmem_data_p->fs_startup_offset[this_header_p->file_type] = 0;
                }
            }
            ret = L_PT_ShMem_Free(&fs_shmem_data_p->fs_file_header_pool_desc, this_header_p);
            FS_OM_LEAVE_CRITICAL_SECTION();
            return ret;
        }
            
        if (compare_result > 0)
        {
            stop = TRUE;
        }
        else
        {
            prev_header_p      = this_header_p;
            this_header_offset = this_header_p->next_offset;
        }
    }
    FS_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

BOOL_T FS_OM_GetStartupName(UI32_T file_type, UI8_T name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1])
{
    FS_FileHeader_T *file_header_p;

    if( file_type > FS_FILE_TYPE_TOTAL )
    {
        ERRORSTR("\r\n%s: file_type out of range %ld >= %d",__FUNCTION__,file_type,FS_FILE_TYPE_TOTAL);
        memset(name, 0, SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);
        return FALSE;
    }
    if (fs_shmem_data_p->fs_startup_offset[file_type] == 0)
    {
        return FALSE;
    }

    file_header_p = (FS_FileHeader_T *)L_CVRT_GET_PTR(fs_shmem_data_p, fs_shmem_data_p->fs_startup_offset[file_type]);    
    memcpy(name, file_header_p->file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

    return TRUE;
}

BOOL_T FS_OM_SetStartupName(UI32_T file_type,UI8_T name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1])
{
    UI8_T           tmp_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;
    FS_FileHeader_T *old_header_p;
    FS_FileHeader_T *new_header_p;
    I32_T           compare_result1,compare_result2;
        
    if( file_type > FS_FILE_TYPE_TOTAL ){
        ERRORSTR("\r\n%s: file_type out of range %ld >= %d",__FUNCTION__,file_type,FS_FILE_TYPE_TOTAL);
        return FALSE;
    }

    FS_OM_ENTER_CRITICAL_SECTION();
    if (TRUE == FS_OM_GetStartupName(file_type,tmp_name))
    {
        if (strncmp((char*)name,(char*)tmp_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN)==0)
        {
            FS_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    }


    if (fs_shmem_data_p->fs_file_header_head_offset == 0)
    {
        FS_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    old_header_p = NULL;
    new_header_p = NULL;
    this_header_offset = fs_shmem_data_p->fs_file_header_head_offset;
    while (this_header_offset != 0) 
    {
        this_header_p = L_CVRT_GET_PTR(fs_shmem_data_p, this_header_offset);
        compare_result1 = strncmp((char*)this_header_p->file_name, (char*)name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        compare_result2 = strncmp((char*)this_header_p->file_name, (char*)tmp_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        if (compare_result1 == 0)
        {
            new_header_p = this_header_p;
        }
        else if (compare_result2 == 0)
        {
            old_header_p = this_header_p;
        }

        if (compare_result1 > 0 &&
            compare_result2 > 0)
        {
            break;
        }
        else if (new_header_p != NULL && old_header_p != NULL)
        {
            break;
        }
        else
        {
            this_header_offset = this_header_p->next_offset;
        }
    }

    if (new_header_p != NULL)
    {
        fs_shmem_data_p->fs_startup_offset[file_type] = L_CVRT_GET_OFFSET(fs_shmem_data_p,new_header_p);
        new_header_p->startup = TRUE;
        if (old_header_p != NULL)
            old_header_p->startup = FALSE;

        FS_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    FS_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

