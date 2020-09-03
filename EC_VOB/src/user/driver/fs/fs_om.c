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
#define FS_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(fs_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define FS_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(fs_om_sem_id)
#define ATOM_EXPRESSION(exp) { FS_OM_ENTER_CRITICAL_SECTION();\
                               exp;\
                               FS_OM_LEAVE_CRITICAL_SECTION();\
                             }

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

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT==TRUE)
static void FS_OM_InitMTDTypeToIDArray(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef INCLUDE_DIAG
static FS_Shmem_Data_T fs_shmem_data_global;
static FS_Shmem_Data_T *fs_shmem_data_p = &fs_shmem_data_global;
#else
static FS_Shmem_Data_T *fs_shmem_data_p;
#endif

static UI32_T fs_om_sem_id;

/* EXPORTED SUBPROGRAM BODIES
 */

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
//  fs_shmem_data_p->FS_FileHeaderList  = NULL;
    fs_shmem_data_p->FS_FileHeaderList_offset = 0;
    fs_shmem_data_p->fs_shutdown_flag   = FALSE;
    fs_shmem_data_p->fs_debug_flag      = FALSE;

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
#if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT==TRUE)
    FS_OM_InitMTDTypeToIDArray();
#endif
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    fs_shmem_data_p->onie_installer_debug_flag=FALSE;
#endif

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
#if 0
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
#if 0
/*partition table init , read partition table info , and save in sh mem*/


BOOL_T FS_OM_PartitionTableInit()
{
    int fd;	
    int read_len, i; 
    AMS_PARTITION_TABLE_T  *partition_table;
    
    fd = open(FS_FLASH_MTD_BLOCK_PARTITION_TABLE, O_RDONLY, 0666);    
    if( fd == -1 )    
    {        
        ERRORSTR("\r\n%s:ERROR!! Can not open %s!!", __FUNCTION__, FS_FLASH_MTD_BLOCK_PARTITION_TABLE);        
        return FALSE;    
    }   

    partition_table = (AMS_PARTITION_TABLE_T *)malloc(sizeof(AMS_PARTITION_TABLE_T));
    read_len = read(fd, (char *)partition_table, sizeof(AMS_PARTITION_TABLE_T));    
    if( read_len < sizeof(AMS_PARTITION_TABLE_T) )    
    {        
        DEBUGSTR("\r\n%s: read size %ld < %ld!!", __FUNCTION__, read_len, sizeof(AMS_PARTITION_TABLE_T));    
        while(1);
    } 
    if(partition_table->magic != PARTITION_TABLE_MAGIC)
    {
        DEBUGSTR("\r\n%s: Bad magic in partition table!!", __FUNCTION__); 
        while(1);
    }
    for(i=0; i< MAX_PARTITION_NUMBER; i++)
    {
        
        strncpy(fs_shmem_data_p->fs_partition_table[i].name, partition_table->flash_region[i].region_name, FLASH_REGION_NAMELEN);
        fs_shmem_data_p->fs_partition_table[i].mtd_num = i;
        fs_shmem_data_p->fs_partition_table[i].base = partition_table->flash_region[i].region_base;
        fs_shmem_data_p->fs_partition_table[i].size = partition_table->flash_region[i].region_size;
        fs_shmem_data_p->fs_partition_table[i].type = partition_table->flash_region[i].region_type;
    }
    free(partition_table);
    close(fd);	
    return TRUE;
}

FS_PARTITION_TABLE_T *FS_GetNextFlashPartitionEntry(FS_PARTITION_TABLE_T *flash_entry_p)
{

     
    if (flash_entry_p == NULL) /* get first entry */
    {
        DEBUGSTR("\r\n%s   first entry::!!\n", __FUNCTION__);
        return &(fs_partition_table[0]);
    }
    else
    {
        DEBUGSTR("\r\n  flash_entry_p %s, mtd num:%d, type %d::!!\n", flash_entry_p->name, flash_entry_p->mtd_num, flash_entry_p->type);
        if (((UI32_T)flash_entry_p<(UI32_T)(&(fs_partition_table[0]))) ||
            ((UI32_T)flash_entry_p>=(UI32_T)(&(fs_partition_table[MAX_PARTITION_NUMBER-1]))))
        {
            return NULL;
        }
        return (FS_PARTITION_TABLE_T*)(((UI8_T*)flash_entry_p)+sizeof(FS_PARTITION_TABLE_T));
    }
}
#endif

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
#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

#if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE)
/* FUNCTION NAME: FS_OM_GetMtdDevIdByMtdType
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the mtd device id according to the given mtd type.
 *-----------------------------------------------------------------------------
 * INPUT    : mtd_type     -  mtd type(FS_MTD_PART_XXX constants defined in fs_type.h)
 * OUTPUT   : mtd_dev_id_p -  The mtd device id of the given mtd_type.
 * RETURN   : TRUE  - The mtd device id is output successfully.
 *            FALSE - The mtd device id of the given mtd_type cannot be found.
 *-----------------------------------------------------------------------------
 * NOTES: This function is called when SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT is TRUE
 */
BOOL_T FS_OM_GetMtdDevIdByMtdType(UI32_T mtd_type, UI32_T *mtd_dev_id_p)
{
    UI32_T mtd_dev_id;

    if (mtd_type>=FS_MTD_PART_MAX_NUMBER)
    {
        printf("%s(%d)Invalid mtd_type=%lu\r\n", __FUNCTION__, __LINE__,
            mtd_type);
        return FALSE;
    }

    if (mtd_dev_id_p==NULL)
    {
        printf("%s(%d)mtd_dev_id_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    mtd_dev_id=fs_shmem_data_p->fs_mtd_type_to_mtd_id_ar[mtd_type];
    if (mtd_dev_id==FS_TYPE_INVALID_MTD_ID)
    {
        return FALSE;
    }

    *mtd_dev_id_p=mtd_dev_id;
    return TRUE;
}
#endif /* end of #if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */
#if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE)

/* FUNCTION NAME: FS_OM_InitMTDTypeToIDArray
 *-----------------------------------------------------------------------------
 * PURPOSE: Initialize the array for mapping mtd type to mtd id.
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is called when SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT is TRUE
 */
static void FS_OM_InitMTDTypeToIDArray(void)
{
    char*  script_cmd_buf_p;
    char   script_output_buf[16];
    FILE   *fp;
    UI32_T rc, mtd_type;
    const struct
    {
        UI8_T mtd_type;
        const char* mtd_name_p;
    } mtd_type_n_name_ar[] =
        {
#ifdef SYS_ADPT_FS_UBOOT_MTD_NAME
            {FS_MTD_PART_UBOOT,   SYS_ADPT_FS_UBOOT_MTD_NAME     },
#endif
#ifdef SYS_ADPT_FS_UBOOT_ENV_MTD_NAME
            {FS_MTD_PART_UB_ENV,  SYS_ADPT_FS_UBOOT_ENV_MTD_NAME },
#endif
#ifdef SYS_ADPT_FS_HWINFO_MTD_NAME
            {FS_MTD_PART_HW_INFO, SYS_ADPT_FS_HWINFO_MTD_NAME    },
#else
#error "SYS_ADPT_FS_HWINFO_MTD_NAME is not defined"
#endif
#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
            {FS_MTD_PART_DATA_STORAGE,   SYS_ADPT_FS_DATA_STORAGE_MTD_NAME     },
#endif
        };
    UI8_T i;

    /* init all entries as FS_TYPE_INVALID_MTD_ID
     */
    memset(fs_shmem_data_p->fs_mtd_type_to_mtd_id_ar, FS_TYPE_INVALID_MTD_ID,
        sizeof(UI8_T)*FS_MTD_PART_MAX_NUMBER);

    script_cmd_buf_p=malloc(FS_GET_MTD_ID_FROM_SCRIPT_CMD_BUF_SIZE);
    if (script_cmd_buf_p==NULL)
    {
        printf("%s(%d)Failed to malloc %d bytes.\r\n", __FUNCTION__, __LINE__,
            FS_GET_MTD_ID_FROM_SCRIPT_CMD_BUF_SIZE);
        return;
    }

    for (i=0; i<sizeof(mtd_type_n_name_ar)/sizeof(mtd_type_n_name_ar[0]); i++)
    {
        snprintf(script_cmd_buf_p, FS_GET_MTD_ID_FROM_SCRIPT_CMD_BUF_SIZE,
            "%s %s", FS_GET_MTD_ID_FROM_SCRIPT_FILENAME, mtd_type_n_name_ar[i].mtd_name_p);
        fp = popen(script_cmd_buf_p, "r");
        if (fp == NULL)
        {
            printf("%s(%d)Failed to run '%s'\r\n",
                __FUNCTION__, __LINE__, script_cmd_buf_p);
            continue;
        }
        else
        {
            if (fgets(script_output_buf, sizeof(script_output_buf), fp)==NULL)
            {
                printf("%s(%d):Failed to get mtd id from script %s\r\n",
                    __FUNCTION__, __LINE__, FS_GET_MTD_ID_FROM_SCRIPT_FILENAME);
            }
            else if (strncmp(script_output_buf, "Err", 3)==0)
            {
                printf("%s(%d):Script '%s' returns error('%s')\r\n",
                    __FUNCTION__, __LINE__, script_cmd_buf_p, script_output_buf);
            }
            else
            {
                mtd_type=mtd_type_n_name_ar[i].mtd_type;
                if(mtd_type>=FS_MTD_PART_MAX_NUMBER)
                {
                    printf("%s(%d)Error, invalid mtd_type(%lu)(mtd_name='%s')\r\n",
                        __FUNCTION__, __LINE__, mtd_type, mtd_type_n_name_ar[i].mtd_name_p);
                }
                else
                {
                    fs_shmem_data_p->fs_mtd_type_to_mtd_id_ar[mtd_type] = atoi(script_output_buf);
                }
            }
            pclose(fp);
        }

    }

    free(script_cmd_buf_p);
}
#endif /* end of #if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE) */

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
/* FUNCTION NAME: FS_OM_GetONIEInstallerDebugFlag
 *-----------------------------------------------------------------------------
 * PURPOSE: Get ONIE installer debug flag
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : ONIE installer debug flag
 *-----------------------------------------------------------------------------
 * NOTES: This function is used by FS backdoor to get the ONIE installer debug
 *        flag.
 */
BOOL_T FS_OM_GetONIEInstallerDebugFlag(void)
{
    return fs_shmem_data_p->onie_installer_debug_flag;
}

/* FUNCTION NAME: FS_OM_SetONIEInstallerDebugFlag
 *-----------------------------------------------------------------------------
 * PURPOSE: Set ONIE installer debug flag
 *-----------------------------------------------------------------------------
 * INPUT    : flag  -  the value of the ONIE installer debug flag to be set.
 *                     TRUE : turn the debug flag on.
 *                     FALSE: turn the debug flag off.
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is used by FS backdoor to set the ONIE installer debug
 *        flag.
 */
void FS_OM_SetONIEInstallerDebugFlag(BOOL_T flag)
{
    fs_shmem_data_p->onie_installer_debug_flag=flag;
}
#endif /* #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

/* FUNCTION NAME: FS_OM_GetDebugFlag
 *-----------------------------------------------------------------------------
 * PURPOSE: Get FS debug flag
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : FS debug flag
 *-----------------------------------------------------------------------------
 * NOTES: This function is used by FS backdoor to get the FS debug flag.
 */
BOOL_T FS_OM_GetDebugFlag(void)
{
    return fs_shmem_data_p->fs_debug_flag;
}

/* FUNCTION NAME: FS_OM_SetDebugFlag
 *-----------------------------------------------------------------------------
 * PURPOSE: Set FS debug flag
 *-----------------------------------------------------------------------------
 * INPUT    : flag  -  the value of the FS debug flag to be set.
 *                     TRUE : turn the debug flag on.
 *                     FALSE: turn the debug flag off.
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is used by FS backdoor to set the FS debug flag.
 */
void FS_OM_SetDebugFlag(BOOL_T flag)
{
    fs_shmem_data_p->fs_debug_flag=flag;
}

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
/* FUNCTION NAME: FS_OM_SetDataStorageMTDDevID
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the mtd device id for data storage service
 *-----------------------------------------------------------------------------
 * INPUT    : mtd_dev_id - mtd device id for data storage service
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
void FS_OM_SetDataStorageMTDDevID(UI8_T mtd_dev_id)
{
    fs_shmem_data_p->fs_data_storage_ctl.data_storage_mtd_dev_id=mtd_dev_id;
}

/* FUNCTION NAME: FS_OM_GetDataStorageMTDDevID
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the mtd device id for data storage service
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : mtd_dev_id mtd device id for data storage service
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
UI8_T FS_OM_GetDataStorageMTDDevID(void)
{
    return fs_shmem_data_p->fs_data_storage_ctl.data_storage_mtd_dev_id;
}

/* FUNCTION NAME: FS_OM_SetDataStorageMTDDevSize
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the size of the mtd device for data storage service
 *-----------------------------------------------------------------------------
 * INPUT    : mtd_dev_size - The size of the mtd device for data storage service
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
void FS_OM_SetDataStorageMTDDevSize(UI32_T mtd_dev_size)
{
    fs_shmem_data_p->fs_data_storage_ctl.mtd_dev_size=mtd_dev_size;
}

/* FUNCTION NAME: FS_OM_GetDataStorageMTDDevSize
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the size of the mtd device for data storage service
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : The size of the mtd device for data storage service in byte
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
UI32_T FS_OM_GetDataStorageMTDDevSize(void)
{
    return fs_shmem_data_p->fs_data_storage_ctl.mtd_dev_size;
}

/* FUNCTION NAME: FS_OM_SetDataStorageBlockSize
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the block size of the mtd device for data storage service
 *-----------------------------------------------------------------------------
 * INPUT    : block_size - block size of the mtd device for data storage service
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
void FS_OM_SetDataStorageBlockSize(UI32_T block_size)
{
    fs_shmem_data_p->fs_data_storage_ctl.block_size=block_size;
}

/* FUNCTION NAME: FS_OM_GetDataStorageBlockSize
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the block size of the mtd device for data storage service
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Block size of the mtd device for data storage service
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
UI32_T FS_OM_GetDataStorageBlockSize(void)
{
    return fs_shmem_data_p->fs_data_storage_ctl.block_size;
}

/* FUNCTION NAME: FS_OM_SetDataStorageInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the data storage info
 *-----------------------------------------------------------------------------
 * INPUT    : storage_id   - the enum value defined in FS_TYPE_DataStorageId_T
 *            storage_size - the real allocated storage size for the specified
 *                           storage id in byte
 *            storage_offset
 *                         - the offset to the starting of the mtd device for
 *                           data storage service
 * OUTPUT   : None
 * RETURN   : TRUE - Success, FALSE - Failed
 *-----------------------------------------------------------------------------
 * NOTES: 1. storage_size and storage_offset must be aligned to the block size.
 */
BOOL_T FS_OM_SetDataStorageInfo(FS_DataStorage_Info_T *data_storage_info_p)
{
    if (data_storage_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)data_storage_info_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    fs_shmem_data_p->fs_data_storage_ctl.info = *data_storage_info_p;

    return TRUE;
}

/* FUNCTION NAME: FS_OM_GetDataStorageSizeAndOffset
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the data storage size and the starting offset of the specified
 *          data storage id
 *-----------------------------------------------------------------------------
 * INPUT    : storage_id   - the enum value defined in FS_TYPE_DataStorageId_T
 * OUTPUT   : storage_size_p
 *                         - the real allocated storage size for the specified
 *                           storage id in byte
 *            storage_offset_p
 *                         - the offset to the starting of the mtd device for
 *                           data storage service
 * RETURN   : TRUE - Success, FALSE - Failed
 *-----------------------------------------------------------------------------
 * NOTES: None
 */
BOOL_T FS_OM_GetDataStorageSizeAndOffset(FS_TYPE_DataStorageId_T storage_id, UI32_T *storage_size_p, UI32_T *storage_offset_p)
{
    if (storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
        return FALSE;

    if (storage_size_p==NULL || storage_offset_p==NULL)
        return FALSE;

    *storage_size_p=fs_shmem_data_p->fs_data_storage_ctl.info.data_storage_table[storage_id].storage_size;
    *storage_offset_p=fs_shmem_data_p->fs_data_storage_ctl.info.data_storage_table[storage_id].abs_offset;

    return TRUE;
}

/* FUNCTION NAME: FS_OM_GetDataStorageRegionInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the data storage region info of the specified storage id
 *-----------------------------------------------------------------------------
 * INPUT    : storage_id   - the enum value defined in FS_TYPE_DataStorageId_T
 * OUTPUT   : region_size_p
 *                         - the size of one data region
 *            region_number_p
 *                         - total number of region of the specified data storage
 *                           id
 * RETURN   : TRUE - Success, FALSE - Failed
 *-----------------------------------------------------------------------------
 * NOTES: The meaning of region might be different among different service type.
 *        For example, one region is the area to keep the data of the single
 *        writing transaction for data service type FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE.
 */
BOOL_T FS_OM_GetDataStorageRegionInfo(FS_TYPE_DataStorageId_T storage_id, UI32_T *region_size_p, UI32_T *region_number_p)
{
    if (storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
        return FALSE;

    if (region_size_p==NULL)
        return FALSE;

    *region_size_p=fs_shmem_data_p->fs_data_storage_ctl.info.data_storage_table[storage_id].data_region_size;
    *region_number_p=fs_shmem_data_p->fs_data_storage_ctl.info.data_storage_table[storage_id].data_region_number;
    return TRUE;
}

/* FUNCTION NAME: FS_OM_GetDataStorageServiceType
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the data storage service type of the specified storage id
 *-----------------------------------------------------------------------------
 * INPUT    : storage_id   - the enum value defined in FS_TYPE_DataStorageId_T
 * OUTPUT   : service_type_p
 *                         - the service type of the spcified storage id
 * RETURN   : TRUE - Success, FALSE - Failed
 *-----------------------------------------------------------------------------
 * NOTES: The meaning of region might be different among different service type.
 *        For example, one region is the area to keep the data of the single
 *        writing transaction for data service type FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE.
 */
BOOL_T FS_OM_GetDataStorageServiceType(FS_TYPE_DataStorageId_T storage_id, FS_TYPE_DataStorageServiceType_T* service_type_p)
{
    if (storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
        return FALSE;

    if (service_type_p==NULL)
        return FALSE;

    *service_type_p=fs_shmem_data_p->fs_data_storage_ctl.info.data_storage_table[storage_id].data_service_type;
    return TRUE;
}

#endif /* end of #if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT==TRUE) */

