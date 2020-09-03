#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "fs_om.h"
#include "sysrsc_mgr.h"
#include "string.h"
#include "fs_type.h"
#include "l_cvrt.h"
#include "l_pt.h"
#include "fs.h"

#ifdef INCLUDE_DIAG
#undef SYS_CPNT_STACKING
#endif

#define FS_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(fs_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define FS_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(fs_om_sem_id)
#define ATOM_EXPRESSION(exp) { FS_OM_ENTER_CRITICAL_SECTION();\
                               exp;\
                               FS_OM_LEAVE_CRITICAL_SECTION();\
                             }

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
    I32_T                   fs_file_header_head_offset;
    L_PT_ShMem_Descriptor_T fs_file_header_pool_desc;
    FS_FileHeader_T         fs_file_header_buffer[FS_MAX_FILE_NUMBER];
    
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  FS_Communication_Sem;
    FS_Control_T            fs_control;
#endif
}  FS_Shmem_Data_T;

#ifdef INCLUDE_DIAG
static FS_Shmem_Data_T fs_shmem_data_global;
static FS_Shmem_Data_T *fs_shmem_data_p = &fs_shmem_data_global;
#else
static FS_Shmem_Data_T *fs_shmem_data_p;
#endif

static UI32_T fs_om_sem_id;

#ifndef INCLUDE_DIAG

/* FUNCTION NAME: FS_OM_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for FLASHDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void FS_OM_InitiateSystemResources(void)
{
    fs_shmem_data_p = (FS_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_FLASHDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(fs_shmem_data_p);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_FS_OM, &fs_om_sem_id);
    return;

}  /* End of FS_InitiateSystemResources() */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FS_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for FLASHDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void FS_OM_AttachSystemResources(void)
{
    fs_shmem_data_p = (FS_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_FLASHDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_FS_OM, &fs_om_sem_id);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FS_OM_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for FLASHDRV.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------
 */

void FS_OM_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_FLASHDRV_SHMEM_SEGID;
    *seglen_p = sizeof(FS_Shmem_Data_T);
    return;
}
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : FS_OM_InitateVariables
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in this process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T FS_OM_InitateVariables(void)
{
    UI32_T  i;        
#if (SYS_CPNT_STACKING == TRUE)
        /*
         * Init FS control block,
         * SYS_TYPE_STACKING_TRANSITION_MODE = 0
         */
        memset(&fs_shmem_data_p->fs_control, 0, sizeof(fs_shmem_data_p->fs_control));
        
        /*
         * Create a semaphore for remote access
         * Remote operation must be carried out in sequence
         */
        if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &fs_shmem_data_p->FS_Communication_Sem) != SYSFUN_OK)
        {
            ERRORSTR("\r\n%s:ERROR, SYSFUN_CreateSem() fail",__FUNCTION__);
            return FALSE;
        }
#endif
    
        fs_shmem_data_p->FS_TaskId          = 0;
        fs_shmem_data_p->fs_all_file_checksum_verified   = FALSE;
        fs_shmem_data_p->FS_InitStatus      = FS_RETURN_OK;
//        fs_shmem_data_p->FS_FileHeaderList  = NULL;
        fs_shmem_data_p->FS_FileHeaderList_offset = 0;
    fs_shmem_data_p->fs_shutdown_flag   = FALSE;
        
    fs_shmem_data_p->fs_file_header_head_offset = 0;
    fs_shmem_data_p->fs_file_header_pool_desc.buffer_offset = sizeof(L_PT_ShMem_Descriptor_T);
    fs_shmem_data_p->fs_file_header_pool_desc.buffer_len    = sizeof(fs_shmem_data_p->fs_file_header_buffer);
    fs_shmem_data_p->fs_file_header_pool_desc.partition_len = sizeof(FS_FileHeader_T);
    if (FALSE == L_PT_ShMem_Create(&fs_shmem_data_p->fs_file_header_pool_desc))
    {
        printf("%s: L_PT_ShMem_Create return FALSE\n",__FUNCTION__);
        return FALSE;
    }

    memset(fs_shmem_data_p->fs_startup_offset, 0, sizeof(fs_shmem_data_p->fs_startup_offset));
        
        /* create semaphore. Wait forever if fail */
        if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &fs_shmem_data_p->FS_Semaphore) != SYSFUN_OK)
        {
            ERRORSTR("\r\n%s:ERROR, SYSFUN_CreateSem() fail",__FUNCTION__);
            return FALSE;
        }    

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

BOOL_T FS_OM_FreeFileHeaderBuffer(FS_FileHeader_T *buf_p)
{
    BOOL_T ret;

    FS_OM_ENTER_CRITICAL_SECTION();
    ret = L_PT_ShMem_Free(&fs_shmem_data_p->fs_file_header_pool_desc, buf_p);
    FS_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}


UI32_T FS_OM_GetTaskId(void)
{
    return fs_shmem_data_p->FS_TaskId;
}

void FS_OM_SetTaskId(UI32_T taskId)
{
    fs_shmem_data_p->FS_TaskId = taskId;
}

void FS_OM_SetNextFileBuffer(FS_FileHeader_T *header, FS_FileHeader_T *next_ptr)
{
    if( next_ptr == NULL ) 
        header->next_offset = 0;
    else
        header->next_offset = L_CVRT_GET_OFFSET(fs_shmem_data_p, next_ptr);
//    fs_shmem_data_p->FS_TaskId = taskId;
}

FS_FileHeader_T *FS_OM_GetNextFileBuffer(FS_FileHeader_T *header)
{
    if( header->next_offset == 0 ) 
        return NULL;
    else
        return L_CVRT_GET_PTR(fs_shmem_data_p, header->next_offset);
//    fs_shmem_data_p->FS_TaskId = taskId;
}


UI32_T FS_OM_GetInitStatus(void)
{
//    DBG_DumpHex("", 80, (char*)fs_shmem_data_p);
    
    return fs_shmem_data_p->FS_InitStatus;
}

void FS_OM_SetInitStatus(UI32_T istatus)
{
    fs_shmem_data_p->FS_InitStatus = istatus;
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


BOOL_T FS_OM_GetNextFileHeaderByName(FS_FileHeader_T *file_header_p, UI32_T type_mask)
{
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;

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
        if ((FS_FILE_TYPE_MASK(this_header_p->file_type) & type_mask) &&
            (strcmp((char*)this_header_p->file_name, (char*)file_header_p->file_name) > 0))
        {
            memcpy(file_header_p,this_header_p,sizeof(FS_FileHeader_T));
            FS_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    else
        {
            this_header_offset = this_header_p->next_offset;
        }        
    }
    FS_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}


BOOL_T FS_OM_GetFirstFileHeaderByType(FS_FileHeader_T *file_header_p)
{
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;

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
        if (this_header_p->file_type == file_header_p->file_type)
        {
            memcpy(file_header_p,this_header_p,sizeof(FS_FileHeader_T));
            FS_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        };
        
        this_header_offset = this_header_p->next_offset;
    }
    FS_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

BOOL_T FS_OM_GetFirstFileHeaderByMtdNum(FS_FileHeader_T *file_header_p)
{
    UI32_T          this_header_offset;
    FS_FileHeader_T *this_header_p;

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
#if 1
       if(FS_FILE_TYPE_RUNTIME == this_header_p->file_type)
        {
            memcpy(file_header_p,this_header_p,sizeof(FS_FileHeader_T));
            FS_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
	
	}
#endif

       if (this_header_p->mtd_num == file_header_p->mtd_num)
        {
            memcpy(file_header_p,this_header_p,sizeof(FS_FileHeader_T));
            FS_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        };
        
        this_header_offset = this_header_p->next_offset;
    }
    FS_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

UI32_T FS_OM_GetSemaphore(void)
{
    return fs_shmem_data_p->FS_Semaphore;
}

BOOL_T FS_OM_GetShutdownFlag(void)
{
    return fs_shmem_data_p->fs_shutdown_flag;
}

void FS_OM_SetShutdownFlag(BOOL_T sflag)
{
    fs_shmem_data_p->fs_shutdown_flag = sflag;
}


BOOL_T FS_OM_GetFileChecksum(void)
{
    return fs_shmem_data_p->fs_all_file_checksum_verified;
}

void FS_OM_SetFileChecksum(BOOL_T sflag)
{
    fs_shmem_data_p->fs_all_file_checksum_verified = sflag;
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
        if (strncmp((char*)name,(char*)tmp_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1)==0)
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
        compare_result1 = strcmp((char*)this_header_p->file_name, (char*)name);
        compare_result2 = strcmp((char*)this_header_p->file_name, (char*)tmp_name);
        if (compare_result1 == 0)
        {
            new_header_p = this_header_p;
}
        else if (compare_result2 == 0)
        {
            old_header_p = this_header_p;
        };

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

#if (SYS_CPNT_STACKING == TRUE)
UI32_T FS_OM_GetCommSemaphore(void)
{
    return fs_shmem_data_p->FS_Communication_Sem;
}


void FS_OM_SetControlDriverId(UI32_T driverid)
{
    fs_shmem_data_p->fs_control.my_drive_id = driverid;
    return;
}

UI32_T FS_OM_GetControlDriverId(void)
{
    return fs_shmem_data_p->fs_control.my_drive_id;
}

UI16_T FS_OM_GetControlSeqNo(void)
{
    return fs_shmem_data_p->fs_control.seq_no;
}
void FS_OM_AddControlSeqNo(void)
{
    fs_shmem_data_p->fs_control.seq_no++;
    return;
}

void FS_OM_SetControlStackingMode(UI8_T mode)
{
    fs_shmem_data_p->fs_control.stacking_mode = mode;
    return;
}
UI8_T FS_OM_GetControlStackingMode(void)
{
    return fs_shmem_data_p->fs_control.stacking_mode;
}

void FS_OM_SetControlState(UI8_T state)
{
    fs_shmem_data_p->fs_control.state = state;
    return;
}
UI8_T FS_OM_GetControlState(void)
{
    return fs_shmem_data_p->fs_control.state;
}

void FS_OM_SetControlFileBuffer(UI8_T *buf)
{
    fs_shmem_data_p->fs_control.file_buffer = buf;
    return;
}
UI8_T* FS_OM_GetControlFileBuffer(void)
{
    return fs_shmem_data_p->fs_control.file_buffer;
}

void FS_OM_SetControlFilePtr(UI8_T *buf)
{
    fs_shmem_data_p->fs_control.file_ptr = buf;
    return;
}
UI8_T* FS_OM_GetControlFilePtr(void)
{
    return fs_shmem_data_p->fs_control.file_ptr;
}
void FS_OM_AddControlFilePtr(UI32_T size)
{
    fs_shmem_data_p->fs_control.file_ptr += size;
    return;
}

void FS_OM_SetControlFlashOperation(UI32_T op)
{
    fs_shmem_data_p->fs_control.flash_operation = op;
    return;
}
UI32_T FS_OM_GetControlFlashOperation(void)
{
    return fs_shmem_data_p->fs_control.flash_operation;
}

void FS_OM_SetControlFlashResult(UI32_T op)
{
    fs_shmem_data_p->fs_control.flash_result = op;
    return;
}
UI32_T FS_OM_GetControlFlashResult(void)
{
    return fs_shmem_data_p->fs_control.flash_result;
}

void FS_OM_ClearControlAttribution(void)
{
    memset(&(fs_shmem_data_p->fs_control.file_attr), 0, sizeof(FS_File_Attr_T));
    return;
}
void FS_OM_GetControlAttribution(FS_File_Attr_T *attr)
{
    memcpy(attr, &(fs_shmem_data_p->fs_control.file_attr), sizeof(FS_File_Attr_T));
    return;
}
void FS_OM_SetControlAttribution(FS_File_Attr_T *attr)
{
    memcpy(&(fs_shmem_data_p->fs_control.file_attr), attr, sizeof(FS_File_Attr_T));
    return;
}

UI32_T FS_OM_GetControlAttrFileSize(void)
{
    return fs_shmem_data_p->fs_control.file_attr.file_size;
}
void FS_OM_MinusControlAttrFileSize(UI32_T size)
{
    fs_shmem_data_p->fs_control.file_attr.file_size -= size;
    return;
}
UI8_T FS_OM_GetControlAttrCheckSum(void)
{
    return fs_shmem_data_p->fs_control.file_attr.check_sum;
}

UI32_T FS_OM_GetControlAttrFileType(void)
{
    return fs_shmem_data_p->fs_control.file_attr.file_type;
}
void FS_OM_GetControlAttrFileName(UI8_T name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1])
{
    memcpy(name, fs_shmem_data_p->fs_control.file_attr.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);
    return;
}

BOOL_T FS_OM_GetControlAttrPrivilage(void)
{
    return fs_shmem_data_p->fs_control.file_attr.privilage;
}
#endif


