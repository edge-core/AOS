#ifndef _FS_OM_H
#define _FS_OM_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sysrsc_mgr.h"
#include "sys_hwcfg.h"

#define ERRORSTR(...) do {} while (0)
#define DEBUGSTR(...) do {} while (0)

#define FS_FILE_COMMENT_LENGTH      32

enum
{
    FS_MAX_NUM_OF_FILE_SUBFILE          = SYS_ADPT_MAX_NUM_OF_FILE_SUBFILE,
    FS_MAX_NUM_OF_FILE_KERNEL           = SYS_ADPT_MAX_NUM_OF_FILE_KERNEL,
    FS_MAX_NUM_OF_FILE_DIAG             = SYS_ADPT_MAX_NUM_OF_FILE_DIAG,
    FS_MAX_NUM_OF_FILE_RUNTIME          = SYS_ADPT_MAX_NUM_OF_FILE_RUNTIME,
    FS_MAX_NUM_OF_FILE_SYSLOG           = SYS_ADPT_MAX_NUM_OF_FILE_SYSLOG,
    FS_MAX_NUM_OF_FILE_CMDLOG           = SYS_ADPT_MAX_NUM_OF_FILE_CMDLOG,
    FS_MAX_NUM_OF_FILE_CONFIG           = SYS_ADPT_MAX_NUM_OF_FILE_CONFIG,
    FS_MAX_NUM_OF_FILE_POSTLOG          = SYS_ADPT_MAX_NUM_OF_FILE_POSTLOG,
    FS_MAX_NUM_OF_FILE_PRIVATE          = SYS_ADPT_MAX_NUM_OF_FILE_PRIVATE,
    FS_MAX_NUM_OF_FILE_CERTIFICATE      = SYS_ADPT_MAX_NUM_OF_FILE_CERTIFICATE,
    FS_MAX_NUM_OF_FILE_ARCHIVE          = SYS_ADPT_MAX_NUM_OF_FILE_ARCHIVE,
    FS_MAX_NUM_OF_FILE_BINARY_CONFIG    = SYS_ADPT_MAX_NUM_OF_FILE_BINARY_CONFIG,
    FS_MAX_NUM_OF_FILE_PUBLIC           = SYS_ADPT_MAX_NUM_OF_FILE_PUBLIC,
    FS_MAX_NUM_OF_FILE_TOTAL            = SYS_ADPT_MAX_NUM_OF_FILE_TOTAL
};

#define FS_MAX_FILE_NUMBER         (FS_MAX_NUM_OF_FILE_SUBFILE+ \
                                    FS_MAX_NUM_OF_FILE_KERNEL+ \
                                    FS_MAX_NUM_OF_FILE_DIAG+ \
                                    FS_MAX_NUM_OF_FILE_RUNTIME+ \
                                    FS_MAX_NUM_OF_FILE_SYSLOG+ \
                                    FS_MAX_NUM_OF_FILE_CMDLOG+ \
                                    FS_MAX_NUM_OF_FILE_CONFIG+ \
                                    FS_MAX_NUM_OF_FILE_POSTLOG+ \
                                    FS_MAX_NUM_OF_FILE_PRIVATE+ \
                                    FS_MAX_NUM_OF_FILE_CERTIFICATE+ \
                                    FS_MAX_NUM_OF_FILE_ARCHIVE+ \
                                    FS_MAX_NUM_OF_FILE_BINARY_CONFIG+ \
                                    FS_MAX_NUM_OF_FILE_PUBLIC)
                                    

typedef struct FS_FileHeader_S
{
    UI8_T   file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]; /* the name of the file */
    UI8_T   file_comment[FS_FILE_COMMENT_LENGTH+1];     /* the comment of the file */
    UI8_T   file_type;                                  /* the type of the file */
    BOOL_T  startup;                                    /* if it a startup file? */
    UI32_T  create_time;                                /* the time when the file is created */
    UI32_T  file_size;                                  /* the size of the file */
    UI32_T  mtd_num;                                    /* mtd number if store in flash */
    UI32_T  magic_number;
// Wally    FS_FileBlockInfo_T  *file_block_list;               /* the file block list containing the blocks used by this file */
    I32_T   next_offset;                      /* pointer to the next file header */
} FS_FileHeader_T;

typedef struct
{
    UI32_T  file_type_mask;                             /* the type mask of the file */
    UI32_T  file_type;                                  /* the type of the file */
    UI32_T  create_time;                                /* the time when the file is created */
    UI32_T  file_size;                                  /* the size of the file */
    UI32_T  reserv_size;
    UI32_T  storage_size;
    BOOL_T  privilage;
    UI8_T   check_sum;
    UI8_T   file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]; /* the name of the file */
    UI8_T   file_comment[FS_FILE_COMMENT_LENGTH+1];     /* the comment of the file */
    BOOL_T  startup_file;                               /* startup file or not; TRUE if yes, or FALSE */
}__attribute__((packed, aligned(1))) FS_File_Attr_T;


#if (SYS_CPNT_STACKING == TRUE)
/********************
 * FS control block *
 ********************/
typedef struct FS_Control_S
{
    UI32_T          flash_operation;
    UI8_T           stacking_mode;      /* Current stacking mode */
    UI8_T           state;              /* Current FS state */
    UI16_T          seq_no;             /* Sequence number of sent/received packet */
    UI32_T          my_drive_id;        /* ID for myself */
    UI32_T          flash_task_id;      /* Task id for the flash programming task */
    UI32_T          flash_result;
    FS_File_Attr_T  file_attr;          /* File attribute of the file being processed */
    UI8_T           *file_buffer;       /* File buffer to hold data before write to flash */
    UI8_T           *file_ptr;          /* points to the position in file that is ready to append data */
}FS_Control_T;
#endif


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
void FS_OM_InitiateSystemResources(void);

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
void FS_OM_AttachSystemResources(void);


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

void FS_OM_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

#endif

BOOL_T FS_OM_InitateVariables(void);

FS_FileHeader_T *FS_OM_AllocateFileHeaderBuffer(void);

BOOL_T FS_OM_FreeFileHeaderBuffer(FS_FileHeader_T *buf_p);

void FS_OM_SetNextFileBuffer(FS_FileHeader_T *header, FS_FileHeader_T *next_ptr);

FS_FileHeader_T *FS_OM_GetNextFileBuffer(FS_FileHeader_T *header);

UI32_T FS_OM_GetTaskId(void);


void FS_OM_SetTaskId(UI32_T taskId);


UI32_T FS_OM_GetInitStatus(void);


void FS_OM_SetInitStatus(UI32_T istatus);

void FS_OM_AddFileHeader(FS_FileHeader_T *file_header_p);

BOOL_T FS_OM_RemoveFileHeaderByName(UI8_T file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]);

BOOL_T FS_OM_GetFileHeaderByName(FS_FileHeader_T *file_header_p, UI32_T type_mask);

BOOL_T FS_OM_GetNextFileHeaderByName(FS_FileHeader_T *file_header_p, UI32_T type_mask);

BOOL_T FS_OM_GetFirstFileHeaderByType(FS_FileHeader_T *file_header_p);

BOOL_T FS_OM_GetFirstFileHeaderByMtdNum(FS_FileHeader_T *file_header_p);

UI32_T FS_OM_GetSemaphore(void);

UI32_T FS_OM_GetCommSemaphore(void);



BOOL_T FS_OM_GetShutdownFlag(void);


void FS_OM_SetShutdownFlag(BOOL_T sflag);



BOOL_T FS_OM_GetFileChecksum(void);


void FS_OM_SetFileChecksum(BOOL_T sflag);


BOOL_T FS_OM_GetStartupName(UI32_T file_type, UI8_T name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]);


BOOL_T FS_OM_SetStartupName(UI32_T file_type,UI8_T name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]);

void FS_OM_SetControlDriverId(UI32_T driverid);

UI32_T FS_OM_GetControlDriverId(void);

UI16_T FS_OM_GetControlSeqNo(void);

void FS_OM_AddControlSeqNo(void);


void FS_OM_SetControlStackingMode(UI8_T mode);

UI8_T FS_OM_GetControlStackingMode(void);


void FS_OM_SetControlState(UI8_T mode);

UI8_T FS_OM_GetControlState(void);


void FS_OM_SetControlFileBuffer(UI8_T *buf);

UI8_T* FS_OM_GetControlFileBuffer(void);


void FS_OM_SetControlFilePtr(UI8_T *buf);

UI8_T* FS_OM_GetControlFilePtr(void);

void FS_OM_AddControlFilePtr(UI32_T size);


void FS_OM_SetControlFlashOperation(UI32_T op);

UI32_T FS_OM_GetControlFlashOperation(void);


void FS_OM_SetControlFlashResult(UI32_T op);

UI32_T FS_OM_GetControlFlashResult(void);


void FS_OM_ClearControlAttribution(void);

void FS_OM_GetControlAttribution(FS_File_Attr_T *attr);

void FS_OM_SetControlAttribution(FS_File_Attr_T *attr);


UI32_T FS_OM_GetControlAttrFileSize(void);

void FS_OM_MinusControlAttrFileSize(UI32_T size);

UI8_T FS_OM_GetControlAttrCheckSum(void);


UI32_T FS_OM_GetControlAttrFileType(void);

void FS_OM_GetControlAttrFileName(UI8_T name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]);

BOOL_T FS_OM_GetControlAttrPrivilage(void);

#endif // _FS_OM_H


