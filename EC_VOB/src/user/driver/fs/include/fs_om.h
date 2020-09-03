#ifndef _FS_OM_H
#define _FS_OM_H

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysrsc_mgr.h"
#include "sys_hwcfg.h"
#include "fs_type.h"

#ifdef FS_UTEST
#define FS_DEBUG
#endif

#ifdef FS_DEBUG
#define DEBUGSTR(...) \
{								\
    printf (__VA_ARGS__);		\
}
#else
#define DEBUGSTR(...) do {} while (0)
#endif

#ifdef FS_DEBUG
#define ERRORSTR(...) \
{								\
    printf (__VA_ARGS__);		\
}
#else
#define ERRORSTR(...) do {} while (0)
#endif


#define FS_FILE_COMMENT_LENGTH      32

/* for projects that do not have the corresponding SYS_ADPT_MAX_NUM_OF_FILE_XXX
 * definitions
 */
#ifndef SYS_ADPT_MAX_NUM_OF_FILE_CPEFIRMWARE
#define SYS_ADPT_MAX_NUM_OF_FILE_CPEFIRMWARE 0
#endif
#ifndef SYS_ADPT_MAX_NUM_OF_FILE_CPECONFIG
#define SYS_ADPT_MAX_NUM_OF_FILE_CPECONFIG 0
#endif
#ifndef SYS_ADPT_MAX_NUM_OF_FILE_FILEMAPPING
#define SYS_ADPT_MAX_NUM_OF_FILE_FILEMAPPING 1
#endif
#ifndef SYS_ADPT_MAX_NUM_OF_FILE_NOS_INSTALLER
#define SYS_ADPT_MAX_NUM_OF_FILE_NOS_INSTALLER 0
#endif

/* Note: Needs to update FS_MAX_FILE_NUMBER when adding new
 *       FS_MAX_NUM_OF_FILE_XXX definition.
 */
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
    FS_MAX_NUM_OF_FILE_CPEFIRMWARE      = SYS_ADPT_MAX_NUM_OF_FILE_CPEFIRMWARE,
    FS_MAX_NUM_OF_FILE_CPECONFIG        = SYS_ADPT_MAX_NUM_OF_FILE_CPECONFIG,
    FS_MAX_NUM_OF_FILE_FILEMAPPING      = SYS_ADPT_MAX_NUM_OF_FILE_FILEMAPPING,
    FS_MAX_NUM_OF_FILE_LICENSE          = SYS_ADPT_MAX_NUM_OF_FILE_LICENSE,
    FS_MAX_NUM_OF_FILE_NOS_INSTALLER    = SYS_ADPT_MAX_NUM_OF_FILE_NOS_INSTALLER,
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
                                    FS_MAX_NUM_OF_FILE_PUBLIC+ \
                                    FS_MAX_NUM_OF_FILE_CPEFIRMWARE+ \
                                    FS_MAX_NUM_OF_FILE_CPECONFIG+ \
                                    FS_MAX_NUM_OF_FILE_FILEMAPPING+ \
                                    FS_MAX_NUM_OF_FILE_LICENSE+ \
                                    FS_MAX_NUM_OF_FILE_NOS_INSTALLER)
                                    

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

/* FS non-volatile data storage service related definition -- START */
/* the entry for information of one data storage service
 */
typedef struct
{
    /* the starting absolute offset to the beginning of the data storage device
     * for a data storage service
     */
    UI32_T abs_offset;

    /* the total storage size for a data storage service
     * storage_size = data_region_size * data_region_number
     */
    UI32_T storage_size;

    /* the size for one data region for a data storage service
     * note that data_region_size must be aligned to the block size
     */
    UI32_T data_region_size;

    /* the number of the data region for a data storage service
     */
    UI32_T data_region_number;

    /* the type of service for a data storage service
     */
    FS_TYPE_DataStorageServiceType_T data_service_type;
} FS_DataStorage_InfoEntry_T;

typedef struct
{
    FS_DataStorage_InfoEntry_T data_storage_table[FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM];
} FS_DataStorage_Info_T;

/* The data in FS_TYPE_DataStorage_Header_T will be written to the beginning of
 * the FS_DATA_STORAGE_HEADER_BLOCK_ID of the mtd device for data storage
 */
typedef struct
{
    UI32_T magic; /* valid value of magic is FS_DATA_STORAGE_MAGIC */
    UI32_T version; /* version should be changed when the way to store data is changed */
    FS_DataStorage_Info_T info;
    UI32_T checksum;
} FS_DataStorage_Header_T;

typedef struct
{
    UI32_T mtd_dev_size; /* total size of the mtd device for data storage service in byte */
    UI32_T block_size;   /* size of one block in byte */
    FS_DataStorage_Info_T info;
    UI8_T  data_storage_mtd_dev_id; /* mtd device id for the data storage service */
} FS_DataStorage_Control_T;

/* FS non-volatile data storage service related definition -- END   */

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
BOOL_T FS_OM_GetMtdDevIdByMtdType(UI32_T mtd_type, UI32_T *mtd_dev_id_p);

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
BOOL_T FS_OM_GetONIEInstallerDebugFlag(void);

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
void FS_OM_SetONIEInstallerDebugFlag(BOOL_T flag);
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
BOOL_T FS_OM_GetDebugFlag(void);

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
void FS_OM_SetDebugFlag(BOOL_T flag);

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
void FS_OM_SetDataStorageMTDDevID(UI8_T mtd_dev_id);

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
UI8_T FS_OM_GetDataStorageMTDDevID(void);

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
void FS_OM_SetDataStorageMTDDevSize(UI32_T mtd_dev_size);

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
UI32_T FS_OM_GetDataStorageMTDDevSize(void);

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
void FS_OM_SetDataStorageBlockSize(UI32_T block_size);

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
UI32_T FS_OM_GetDataStorageBlockSize(void);

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
BOOL_T FS_OM_SetDataStorageInfo(FS_DataStorage_Info_T *data_storage_info_p);

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
BOOL_T FS_OM_GetDataStorageSizeAndOffset(FS_TYPE_DataStorageId_T storage_id, UI32_T *storage_size_p, UI32_T *storage_offset_p);

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
BOOL_T FS_OM_GetDataStorageRegionInfo(FS_TYPE_DataStorageId_T storage_id, UI32_T *region_size_p, UI32_T *region_number_p);

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
BOOL_T FS_OM_GetDataStorageServiceType(FS_TYPE_DataStorageId_T storage_id, FS_TYPE_DataStorageServiceType_T* service_type_p);

#endif // _FS_OM_H

