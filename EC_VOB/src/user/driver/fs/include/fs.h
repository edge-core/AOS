/*-------------------------------------------------------------------------
 * Module Name:fs.h
 *-------------------------------------------------------------------------
 * Purpose    : A header file for the API definition of File System
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    09/01/2001 - Jimmy Lin, Created
 *    11/23/2001 - Allen Cheng, Revised
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-------------------------------------------------------------------------
 */


#ifndef _FS_H_
#define _FS_H_

#ifdef INCLUDE_DIAG
#undef SYS_CPNT_STACKING
#endif

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif
#include "fs_om.h"
#include "fs_type.h"
#include "sys_imghdr.h"

/************************************
 *** NAMING CONSTANT DECLARATIONS ***
 ************************************/
/* FS_PATHNAME_MAX_BUF_SIZE
 *     This constant defines the size of buffer to be allocated for storing
 *     the path name string.(the size includes the null-terminated char)
 */
#define FS_MAX_PATHNAME_BUF_SIZE (SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1)

/************************************
 ***      MACRO DEFINITIONS       ***
 ************************************/
#define FS_SEMAPHORE_LOCK()     SYSFUN_TakeSem(FS_OM_GetSemaphore(), SYSFUN_TIMEOUT_WAIT_FOREVER);
#define FS_SEMAPHORE_UNLOCK()   SYSFUN_GiveSem(FS_OM_GetSemaphore());
#define FS_LOCK(_ret_)      {   FS_SEMAPHORE_LOCK();                \
                                if (FS_OM_GetShutdownFlag())        \
                                {                                   \
                                    FS_SEMAPHORE_UNLOCK();          \
                                    return _ret_;                   \
                                }                                   \
                            }

#define FS_UNLOCK()         {   FS_SEMAPHORE_UNLOCK();              \
                            }

/* use of mode_bitmapn on diag
 * mode_bitmap1   --   current count of execution of burn-in mode post
 * mode_bitmap2   --   the target execution count of burn-in mode post
 *
 * use of mode_bitmapn specific on ASF4612MMS-FLF-08
 * mode_bitmap3   --   the lower limit of temperature for burn-in test in chamber
 * mode_bitmap4   --   the upper limit of temperature for burn-in test in chamber
 *
 * WARNING!!! Please update the macro to check size of FS_HW_Info_T when the
 * fields in FS_HW_Info_T is changed!!!
 *
 * WARNING!!! There is a duplicated definition of FS_HW_Info_T in ams_common.h
 * which is used by uboot. Please ensure that the change of FS_HW_Info_T must
 * be synced to ams_common.h
 */
typedef struct FS_HW_Info_S
{
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN];                    /* MAC address */
    UI8_T   serial_no[SYS_ADPT_SERIAL_NO_STR_LEN + 1];          /* serial number */
    UI8_T   agent_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];          /* agent board hardware version (len 5+1) */
    UI8_T   manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN];    /* Product date, option to key in. NOTE!!! the string in this field is not terminated with '\0'! */
    UI32_T  model_num;                                          /* Model number */
    UI32_T  mode_bitmap1;                                       /* Mode bitmap */
    UI32_T  mode_bitmap2;                                       /* Mode bitmap */
    UI32_T  mode_bitmap3;                                       /* Mode bitmap */
    UI32_T  mode_bitmap4;                                       /* Mode bitmap */
    UI8_T   service_tag[SYS_ADPT_SERIAL_NO_STR_LEN +1];         /* Service tag */
    UI32_T  project_id;                                         /* Project ID  */
    UI32_T  board_id;                                           /* Board ID    */
    UI8_T   password[SYS_ADPT_MAX_PASSWORD_LEN + 1];            /* OEM password */
    UI32_T  baudrate;                                           /* Filled by loader information */
    UI8_T   model_num_by_string[SYS_ADPT_MODEL_NUMBER_LEN + 1]; /* For model number in character string. */
    UI8_T   post_mode;                                          /* POST_ITEM_INDEX_T defined in post_list.h */
    UI32_T  special_boot_op;                                    /* special boot option, for nand flash MFG */
    UI8_T   burn_in_post_ever_failed;                           /* burn in post ever failed, allowed value should be type of POST_BURN_IN_POST_EVER_FAILED_T which is defined in post.h */
    UI8_T   capability;                                         /* bitmap for capability of the device(e.g. capability to change loader password) */
    UI8_T   reserved2[SYS_ADPT_RESERVED_INFO_LEN + (SYS_ADPT_RESERVED_INFO_LEN - 21 - SYS_ADPT_MODEL_NUMBER_LEN - 1 - 1/*post_mode*/ - 4 /*special_boot_op*/ - 1/*burn_in_post_ever_failed*/-1/*capability*/)]; /* reserved for more board info */
    UI8_T   reserved3[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved4[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved5[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved6[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved7[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved8[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI16_T  check_sum;                                          /* check sum */
}__attribute__((packed, aligned(1))) FS_HW_Info_T;

/* The size of FS_HW_Info_T should always be the same in a project.
 * Evaluate the size of FS_HW_Info_T and issue an error if the size is changed.
 * To allow the variation of the defined value among different projects,
 * the project may define the value of SYS_ADPT_SIZEOF_FS_HW_INFO_T in its sys_adpt.h
 * if SYS_ADPT_SIZEOF_FS_HW_INFO_T is not defined, the default value of 
 * SYS_ADPT_SIZEOF_FS_HW_INFO_T is 368
 */
#ifndef SYS_ADPT_SIZEOF_FS_HW_INFO_T
#define SYS_ADPT_SIZEOF_FS_HW_INFO_T 368
#endif

#if (((SYS_ADPT_MAC_ADDR_LEN) /* mac_addr */ + \
      (SYS_ADPT_SERIAL_NO_STR_LEN+1) /* serial_no */ + \
      (SYS_ADPT_HW_VER_STR_LEN+1) /* agent_hw_ver */ + \
      (SYS_ADPT_MANUFACTURE_DATE_LEN) /* manufacture_date */ + \
      4 /* model_num */ + 4 /* mode_bitmap1 */ + 4 /* mode_bitmap2 */ + \
      4 /* mode_bitmap3 */ + 4 /* mode_bitmap4 */ + \
      (SYS_ADPT_SERIAL_NO_STR_LEN+1) /* service_tag */ + \
      4 /* project_id */ + 4 /* board_id */ + \
      (SYS_ADPT_MAX_PASSWORD_LEN+1) /* password */ + \
      4 /* baudrate */ + (SYS_ADPT_MODEL_NUMBER_LEN+1) /* model_num_by_string */ + \
      1 /* post_mode */ + 4 /* special_boot_op */ + \
      1 /* burn_in_post_ever_failed */ + 1 /* capability */ + \
      SYS_ADPT_RESERVED_INFO_LEN + (SYS_ADPT_RESERVED_INFO_LEN - 21 - SYS_ADPT_MODEL_NUMBER_LEN - 1 - 1 - 4 - 1 - 1 ) /* reserved2 */ + \
      (SYS_ADPT_RESERVED_INFO_LEN*6) /* reserved3-reserved8 */ + \
      2 /* check_sum */) != SYS_ADPT_SIZEOF_FS_HW_INFO_T)
#error "sizeof(FS_HW_Info_T) is changed, please check!"
#endif
    

#define FS_FILE_TYPE_MASK(file_type)    (0x00000001 << (file_type))
#define FS_FILE_TYPE_MASK_ALL           0xFFFFFFFF
#define FS_FACTORY_DEFAULT_CONFIG       SYS_DFLT_restartConfigFile

#define DUMMY_DRIVE                     (0xFFFFFFFF) /* unit number to represent local operation in transition mode */

BOOL_T  FS_Debug(UI32_T flag);
void    FS_SetDebugFlag(UI32_T flag);
void    FS_GetDebugFlag(UI32_T *flag);

/* FUNCTION NAME: FLASHDRV_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for FLASHDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void FS_InitiateSystemResources(void);

void FS_AttachSystemResources(void);

void FS_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);



/* ------------------------------------------------------------------------
 * FUNCTION NAME - FS_TASK_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create the fs task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_TASK_CreateTask(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - FS_TASK_Init
 *-------------------------------------------------------------------------
 * PURPOSE  : Init the FS task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void FS_TASK_Init(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function will initialize the file system
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_Init(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Create_InterCSC_Relation
 * ------------------------------------------------------------------------
 * FUNCTION : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    FS_Create_InterCSC_Relation(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_FilenameCheck
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the file name to be composed with the valid
 *            characters or not.
 * INPUT    : filename      -- a string specified by users
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E; FS_RETURN_OK returned if valid otherwise
 *            FS_RETURN_ERROR if filename contains the invalid character.
 * NOTE     : FS_RETURN_ERROR returned if filename is a NULL string
 * ------------------------------------------------------------------------
 */
UI32_T  FS_FilenameCheck(UI8_T *filename);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GenericFilenameCheck
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the file name to be composed with the valid
 *            characters or not. As for an invisible file name it should be
 *            composed with the prefixed character.
 * INPUT    : filename      -- a string specified by users
 *            file_type     -- file type
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E;
 *            FS_RETURN_OK      -- the input file name is valid
 *            FS_RETURN_ERROR   -- the input file name is invalid
 * NOTE     : FS_RETURN_ERROR returned if filename is a NULL string
 * ------------------------------------------------------------------------
 */
UI32_T  FS_GenericFilenameCheck(UI8_T *filename, UI32_T file_type);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetImageType
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image type of the input data
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : the image type of the data, only two kind of image can
 *            be validated now, kernel and rootfs
 *            FS_FILE_TYPE_KERNEL   --- kernel image
 *            FS_FILE_TYPE_RUNTIME  --- root file system
 *            FS_FILE_TYPE_TOTAL    --- cant't validate
 * NOTE     : return FS_FILE_TYPE_TOTAL, just mean that not kernel or rootfs
 *            it may be config file or other valid files
 * ------------------------------------------------------------------------
 */
//FS_File_Type_T  FS_GetImageType(UI8_T *data_p, UI32_T length);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckImageHeaderProductId
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image header product id
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : TRUE       -- product id check pass
 *            FALSE      -- product id check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckImageHeaderProductId(UI8_T *data_p, UI32_T length);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckLoaderImageHeaderProductId
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the loader image header product id
 * INPUT    : data_p     -- pointer to the loader image header
 * OUTPUT   : None
 * RETURN   : TRUE       -- product id check pass
 *            FALSE      -- product id check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckLoaderImageHeaderProductId(LDR_INFO_BLOCK_T *ldr_hdr_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckImageHeaderCrc
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image header crc
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : TRUE       -- CRC check pass or no need to check (cramfs)
 *            FALSE      -- CRC check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckImageHeaderCrc(UI8_T *data_p, UI32_T length);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckImageDataCrc
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image data crc
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : TRUE       -- CRC check pass
 *            FALSE      -- CRC check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckImageDataCrc(UI8_T *data_p, UI32_T length);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetStorageFreeSpace
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the available free space of the file system.
 * INPUT    : drive                     -- unit id or blade number
 * OUTPUT   : total_size_of_free_space  -- total size of the free space
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_GetStorageFreeSpace(UI32_T drive, UI32_T *total_size_of_free_space);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetFileStorageUsageSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the usage size of the specified file.
 * INPUT    : drive                     -- unit id or blade number
 *            file_name                 -- specified file name
 * OUTPUT   : usage_size                -- usage size of the file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_GetFileStorageUsageSize(UI32_T drive, UI8_T *file_name, UI32_T *usage_size);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the information of the file
 * INPUT    : drive                     -- unit id or blade number
 *            file_attr->file_name      -- the name of the file
 *            file_attr->file_type_mask -- the type mask of the file
 * OUTPUT   : file_attr                 -- the attribute of the file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetFileInfo(UI32_T drive, FS_File_Attr_T *file_attr);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetWebFileSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get size of the file
 * INPUT    : file_path     -- the path (if exist) and name of file to read; the
 *                             path is relative path to /usr/webroot/
 * OUTPUT   : file_size_p   -- the size of the file to read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1.the directory of web files: /usr/webroot/
 * ------------------------------------------------------------------------
 */
FS_RETURN_CODE_T FS_GetWebFileSize(const char *file_path, UI32_T *file_size_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetNextFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the information of the next file
 * INPUT    : drive                     -- unit id or blade number
 *            file_attr->file_name      -- the key to get
 *            file_attr->file_type_mask -- the type mask of the file
 * OUTPUT   : file_attr->file_name      -- the name of the next file
 *            file_attr                 -- the attribute of the file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The first file in the file list will be returned if the input
 *            file_name is a NULL pointer (file_name[0] == 0).
 *            2. drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetNextFileInfo(UI32_T *drive, FS_File_Attr_T *file_attr);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_DeleteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will delete a file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the file to delete
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_DeleteFile(UI32_T drive, UI8_T *file_name);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_DeleteProtectedFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will delete a protected file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the file to delete
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_DeleteProtectedFile(UI32_T drive, UI8_T *file_name);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read data from the specified file to the buffer
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of file to read
 *            buf           -- the destination buffer
 *            buf_size      -- the length of the buffer
 * OUTPUT   : read_count    -- the count of bytes read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. If the buf_size is less than the file size, the file will be
 *            truncated and FS_RETURN_FILE_TRUNCATED is returned.
 *            2. drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_ReadFile(UI32_T drive, UI8_T  *file_name, UI8_T  *buf, UI32_T buf_size, UI32_T *read_count);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadWebFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read data from the specified file to the buffer
 * INPUT    : file_path     -- the path (if exist) and name of file to read; the
 *                             path is relative path to /usr/webroot/
 *            buf           -- the destination buffer
 *            buf_size      -- the length of the buffer
 * OUTPUT   : read_count_p  -- the count of bytes read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1.the directory of web files: /usr/webroot/
 *            2.This API will not check the total length of the web file to be read. The maximum
 *              length that will be read is buf_size
 * ------------------------------------------------------------------------
 */
FS_RETURN_CODE_T FS_ReadWebFile(const char *file_path, UI8_T *buf, UI32_T buf_size, UI32_T *read_count_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CopyFileContent
 * ------------------------------------------------------------------------
 * FUNCTION : This function will copy data from the specified offset of the
 *            file to the buffer
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of file to be copied
 *            offset        -- the offset of the file to be copied
 *            buf           -- the destination buffer
 *            byte_num      -- the length of data to be copied
 * OUTPUT   : read_count    -- the count of bytes read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_CopyFileContent(UI32_T drive, UI8_T *file_name, UI32_T offset, UI8_T *buf, UI32_T byte_num);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data from buffer to a file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum size of
 *                             a file. resv_len < length will not reserve extra
 *                             space for the file.
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteFile(UI32_T drive, UI8_T  *file_name, UI8_T  *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteProtectedFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data from buffer to a protected file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum size of
 *                             a file. resv_len < length will not reserve extra
 *                             space for the file.
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteProtectedFile(UI32_T drive, UI8_T  *file_name, UI8_T  *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NumOfFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the number of the file of a given type
 * INPUT    : file_type     -- the given file type
 * OUTPUT   : None
 * RETURN   : Number of the file
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T  FS_NumOfFile(UI32_T file_type);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetNumOfFileByUnitID
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the number of the file of a given type by unit id
 * INPUT    :unit         --- unit id 
                  file_type     -- the given file type
 * OUTPUT   : None
 * RETURN   : Number of the file
 * NOTE     : None
 * ------------------------------------------------------------------------
 */

UI32_T FS_GetNumOfFileByUnitID(UI32_T unit, UI32_T file_type);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the default file of a given type
 * INPUT    : drive         -- unit id or blade number
 *            file_type     -- the type of file
 *            file_name     -- the name of file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_SetStartupFilename(UI32_T drive, UI32_T file_type, UI8_T  *file_name);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ResetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the default file of a given type
 * INPUT    : drive         -- unit id or blade number
 *            file_type     -- the type of file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_ResetStartupFilename(UI32_T drive, UI32_T file_type);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the default file of a given type
 * INPUT    : drive         -- unit id or blade number
 *            file_type     -- the type of file
 * OUTPUT   : file_name     -- the name of file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetStartupFilename(UI32_T drive, UI32_T file_type, UI8_T  *file_name);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteHardwareInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write hardware information to flash
 * INPUT    : hwinfo        -- hardware information to write
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T  FS_WriteHardwareInfo(FS_HW_Info_T *hwinfo);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteLoader
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write loader to flash
 * INPUT    : drive         -- unit id or blade number
 *            loader        -- loader to write
 *            size          -- size of the loader
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteLoader(UI32_T drive, UI8_T *loader, UI32_T size);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteKernel
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write kernel image to flash
 * INPUT    : drive         -- unit id or blade number
 *            kernel        -- loader to write
 *            size          -- size of the loader
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteKernel(UI32_T drive, UI8_T *kernel, UI32_T size);


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadHardwareInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read hardware information from flash
 * INPUT    : drive         -- unit number or blade number
 *            hwinfo        -- hardware information read from the file system
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_ReadHardwareInfo(UI32_T drive, FS_HW_Info_T *hwinfo);

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetOpCodeDirectory
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the directory where the op code files
 *            are put in.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
const char* FS_GetOpCodeDirectory(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRequiredTmpFilenameBufSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function calcuate the required buffer size when
 *            calling the function FS_CreateTmpFile().
 * INPUT    : prefix_tmp_filename_p --
 *                The name to be prefixed to the temporarily file name. Suggests
 *                to use CSC name or "CSC name"+"operation name".
 * OUTPUT   : required_buf_sz_p --
 *                The required buffer size when calling the function
 *                FS_CreateTmpFile().
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : The output value of required_buf_sz_p incudes the terminating
 *            null char.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetRequiredTmpFilenameBufSize(const char* prefix_tmp_filename_p, UI32_T *required_buf_sz_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CreateTmpFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function creates a temporarily file and write the given
 *            data to the file. The full path to the temporarily file will be
 *            output.
 * INPUT    : prefix_tmp_filename_p --
 *                The name to be prefixed to the temporarily file name. Suggests
 *                to use CSC name or "CSC name"+"operation name".
 *            file_data_p --
 *                The pointer to the data to be written to the new created
 *                temporarily file.
 *            file_data_size --
 *                The size of the data to be written to the new created
 *                temporarily file.
 *            tmpfilename_size --
 *                The size of the output buffer for full path to temporarily
 *                file. The size includes the terminating null char.
 * OUTPUT   : tmpfilename_p    --
 *                The full path to the new created temporarily file.
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The permission of the new created temporarily file is set
 *               as "rwx------".
 *            2. The required buffer size of tmpfilename_p can be calcuated
 *               by FS_GetRequiredTmpFilenameBufSize().
 *            3. The temporarily file is only created when return value is
 *               FS_RETURN_OK
 * ------------------------------------------------------------------------
 */
UI32_T FS_CreateTmpFile(const char* prefix_tmp_filename_p, const UI8_T *file_data_p,
    UI32_T file_data_size, UI32_T tmpfilename_size, char* tmpfilename_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoveTmpFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function removes the temporarily file created by the function
 *            FS_CreateTmpFile().
 * INPUT    : tmpfilename_p    --
 *                The full path to the temporarily file.
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : This function can only used to remove the file created by
 *            the function FS_CreateTmpFile
 * ------------------------------------------------------------------------
 */
UI32_T FS_RemoveTmpFile(char* tmpfilename_p);
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_BlockByteSum
 * ------------------------------------------------------------------------
 * FUNCTION : This function will calculate the sum of the specified block
 * INPUT    : block_id      -- block id
 * OUTPUT   : sum           -- sum of the specified block
 * RETURN   : FS_RETURN_OK, or others if any error occurs
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T  FS_BlockByteSum(UI32_T unit, UI32_T block_id, UI8_T *sum);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : This function signals FS to prepare to enter transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void  FS_SetTransitionMode(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : This put FS into transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void  FS_EnterTransitionMode(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : This put FS into master mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void  FS_EnterMasterMode(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : This put FS into slave mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void  FS_EnterSlaveMode(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : This function will perform the operation when the provision
 *            is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_ProvisionComplete(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Busy
 * ------------------------------------------------------------------------
 * FUNCTION : This function will return the availability of the FS module
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE: FS is busy
 *            FALSE: FS is not busy
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_Busy(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Abort
 * ------------------------------------------------------------------------
 * FUNCTION : Abort the current operation and return to idle
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void  FS_Abort(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Shutdown
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the shutdown flag to terminate all the
 *            access to the FS. All the FS operations will be rejected
 *            after this function is invoked and this function will not
 *            return until no operation is performing (or trying to perform)
 *            accessing the FS.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : -- Rapid shutdown procedure:
 *               1. set the flag;
 *               2. lock;
 *               3. unlock;
 *            -- Perfect shutdown procedure:
 *               1. lock;
 *               2. set the flag;
 *               3. unlock;
 * ------------------------------------------------------------------------
 */
void FS_Shutdown(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Restart
 * ------------------------------------------------------------------------
 * FUNCTION : This function will clear the shutdown flag to allow the
 *            access to the FS.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_Restart(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteFlashTaskMain
 * ------------------------------------------------------------------------
 * FUNCTION : The main function of the flash programming task. It sets the
 *            FS back to idle state after flash programming is complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function actually performs three actions.
 *          1. Write file
 *          2. Write loader : Filename[0] = 0;
 *          3. Erase file: file size = 0;
 * ------------------------------------------------------------------------
 */
void FS_RemoteFlashTaskMain(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SetTaskId
 * ------------------------------------------------------------------------
 * FUNCTION : This function will record the FS task ID.
 * INPUT    : task_id   -- the FS task ID
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    FS_SetTaskId(UI32_T task_id);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_LocalCheckFileSystem
 * ------------------------------------------------------------------------
 * FUNCTION : This function will check the local file system to remove the
 *            destroyed files.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_LocalCheckFileSystem(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteFileToMultipleUnit
 * ------------------------------------------------------------------------
 * FUNCTION : This function will concurrently write a file from buffer to
 *            the multiple unit.
 * INPUT    : dst_drive_bmp -- bit map of destination unit id
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum
 *                             size of a file. resv_len < length will not
 *                             reserve extra space for the file.
 * OUTPUT   : None
 * RETURN   : TRUE  -- write file to all unit successfully
 *            FALSE -- fail in writing file to any unit
 * NOTE     : In ACP_V3, this function uses ISC reliable multi-cast to
 *            implement concurrent communication with each unit.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_WriteFileToMultipleUnit (UI16_T  *dst_drive_bmp,  UI8_T   *file_name,
                                   UI8_T   *file_comment,   UI32_T  file_type,
                                   UI8_T   *buf,            UI32_T  length,
                                   UI32_T  resv_len);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteReservedBlock
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data to a reserved block of FLASH.
 * INPUT    : writing_addr -- specified FLASH address for writing data
 *            buf          -- the data source buffer
 *            size         -- data length to be written (data size)
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteReservedBlock(UI32_T writing_addr, UI8_T *buf, UI32_T size);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ISC_Handler
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the serive demultiplexer
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference for received packet
 * OUTPUT   : None
 * RETURN   : TRUE: function completed successfully, FALSE: function failed
 * NOTE     : 1. This function will invoke functions corresponding to the opcode
 *               of the received packet.
 *            2. This function will check the sequence_number of the received
 *               packet.
 * ------------------------------------------------------------------------
 */
#if (SYS_CPNT_STACKING == TRUE)
BOOL_T FS_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
#endif
/*fs partition table init*/
void FS_PartitionTableInit();

UI32_T  FS_GetStorageFreeSpaceForUpdateRuntime(UI32_T drive, UI32_T *free_space_p);

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
UI32_T FS_GetNumOfPartitionByTypeFromPartitionTable(UI32_T partition_type);
#endif

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadMtdDev
 * ------------------------------------------------------------------------
 * FUNCTION : Read data from the specified mtd device id from the given
 *            offset.
 * INPUT    : mtd_dev_id  - The mtd device id
 *            offset      - The offset to the beginning of the given mtd device
 *                          to read the data
 *            read_data_len_p
 *                        - The length of the data to be read
 * OUTPUT   : read_data_len_p
 *                        - The actual length of data read from the mtd device
 *            read_data_buf_p
 *                        - The data read from the mtd device
 * RETURN   : TRUE - Success, FALSE - Error
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T FS_ReadMtdDev(UI32_T mtd_dev_id, UI32_T offset, UI32_T* read_data_len_p, void *read_data_buf_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EraseMtdDev
 * ------------------------------------------------------------------------
 * FUNCTION : Erase the data at the specified offset with the given length.
 * INPUT    : mtd_dev_id  - The mtd device id
 *            offset      - The offset to the beginning of the given mtd device
 *                          to erase the data
 *            erase_len   - The length of the data to be erased
 * OUTPUT   : None
 * RETURN   : TRUE - Success, FALSE - Error
 * NOTE     : 1. This function provides the functionality to perform low-level
 *               erase operation to a flash through the mtd device.
 *            2. offset and erase_len must be aligned to the block size.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_EraseMtdDev(UI32_T mtd_dev_id, UI32_T offset, UI32_T erase_len);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteMtdDev
 * ------------------------------------------------------------------------
 * FUNCTION : Write the data at the specified offset with the given length.
 * INPUT    : mtd_dev_id  - The mtd device id
 *            offset      - The offset to the beginning of the given mtd device
 *                          to write the data
 *            write_data_buf_p
 *                        - The data to be written to the mtd device
 *            write_data_len_p
 *                        - The length of the data to be written
 * OUTPUT   : write_data_len_p
 *                        - The actual length of the data written to the mtd device
 * RETURN   : TRUE - Success, FALSE - Error
 * NOTE     : 1. This function provides the functionality to perform low-level
 *               write operation to the specified location of a flash through
 *               the mtd device.
 *            2. The data to be written to must have been erased.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_WriteMtdDev(UI32_T mtd_dev_id, UI32_T offset, void *write_data_buf_p, UI32_T* write_data_len_p);

/* FUNCTION NAME: FS_InitDataStorageControl
 *-----------------------------------------------------------------------------
 * PURPOSE: Initialize the data for data storage control
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - Success, FALSE - Error
 *-----------------------------------------------------------------------------
 * NOTES: This function is called when SYS_CPNT_FS_SUPPORT_DATA_STORAGE is TRUE
 */
BOOL_T FS_InitDataStorageControl(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_GetSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the maximum size of the non-volatile data
 *            storage for the specified data storage id.
 * INPUT    : data_storage_id  -  The id of the data storage.
 * OUTPUT   : None.
 * RETURN   : The maximum size of the data storage for the specified data storage id.
 *            The return value is 0 when error.
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
UI32_T FS_NonVolatileDataStorage_GetSize(FS_TYPE_DataStorageId_T data_storage_id);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_Write
 * ------------------------------------------------------------------------
 * FUNCTION : This function writes the specified data to the non-volatile data
 *            storage for the specified data storage id.
 * INPUT    : data_storage_id - The id of the data storage.
 *            write_data_size - The size of the data pointed by write_data_p.
 *            write_data_p    - The pointer to the data to be written.
 * OUTPUT   : None.
 * RETURN   : TRUE  -  Write data ok.
 *            FALSE -  Write data error.
 * NOTE     : 1. When the data to be written is aborted due to unexpected
 *               events(e.g. power failure), the read data at next time would
 *               be the latest data that was written without error.
 *            2. The data size cannot be larger than the value returned by
 *               FS_NonVolatileDataStorage_GetSize().
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_Write(FS_TYPE_DataStorageId_T data_storage_id, UI32_T write_data_size, UI8_T *write_data_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_Read
 * ------------------------------------------------------------------------
 * FUNCTION : This function reads the specified data from the non-volatile data
 *            storage for the specified data storage id..
 * INPUT    : data_storage_id - The id of the data storage.
 *            read_data_size_p- The size of the buffer pointed by read_data_p.
 * OUTPUT   : read_data_size_p- The size of the data read from the data storage.
 *            read_data_p     - The pointer to the read data
 * RETURN   : TRUE  -  Read data ok.
 *            FALSE -  Read data error.
 * NOTE     : 1. If there is no valid data in the non-volatile
 *               data storage of the specified data sorage id, the function
 *               returns FALSE.
 *            2. This function will return the latest data that is written
 *               without error. The corrupted written data will be dropped.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_Read(FS_TYPE_DataStorageId_T data_storage_id, UI32_T* read_data_size_p, UI8_T *read_data_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_LowLevelWrite
 * ------------------------------------------------------------------------
 * FUNCTION : This function performs low-level write operation, which will
 *            write the specified data to the specified block id at the
 *            specified offset to the beginning of the block with the specified
 *            length.
 * INPUT    : data_storage_id - The id of the data storage
 *            block_id        - Block id, starts from 0
 *            offset          - The specified data will be written to the offset
 *                              to the beginning of the block
 *            write_data_size - The size of the data to be written
 *            write_data_p    - The data to be written
 * OUTPUT   : None
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 1. This function can only write the specified data within the
 *               specified block. If the sum of the offset and write_data_size
 *               is larger than the block size, this function returns FALSE.
 *            2. The caller of this function is responsible to call
 *               FS_NonVolatileDataStorage_LowLevelErase to erase the block
 *               before writing when necessary.
 *            3. Caller should not hold FS lock when calling this function
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_LowLevelWrite(FS_TYPE_DataStorageId_T data_storage_id, UI32_T block_id, UI32_T offset, UI32_T write_data_size, UI8_T *write_data_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_OffsetToBlockId
 * ------------------------------------------------------------------------
 * FUNCTION : This function converts the given storage id and offset into
 *            block id.
 * INPUT    : data_storage_id - The id of the data storage
 *            offset          - Offset to the beginning of the specified data
 *                              storage id
 * OUTPUT   : block_id_p      - The block id of the specified offset and data
 *                              storage id. 0 means the first block.
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_OffsetToBlockId(FS_TYPE_DataStorageId_T data_storage_id, UI32_T offset, UI32_T *block_id_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_LowLevelErase
 * ------------------------------------------------------------------------
 * FUNCTION : Perform low level erase operation according to the given
 *            data storage id, start block id and total number of erase block.
 * INPUT    : data_storage_id - The id of the data storage
 *            start_block_id  - The starting block id to perform erase operation
 *            total_erase_block_num
 *                            - Total number of block to perform erase operation
 * OUTPUT   : None
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 1. Only service type FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE
 *               can call this function currently.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_LowLevelErase(FS_TYPE_DataStorageId_T data_storage_id, UI32_T start_block_id, UI32_T total_erase_block_num);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_LowLevelRead
 * ------------------------------------------------------------------------
 * FUNCTION : Perform low level read operation according to the given
 *            data storage id and block id
 * INPUT    : data_storage_id - The id of the data storage
 *            block_id        - The block id to perform read operation
 * OUTPUT   : block_data_p    - The read data will be put to this buffer. The
 *                              buffer size must be larger than the block size
 *                              got by FS_NonVolatileDataStorage_LowLevelGetBlockSize().
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_LowLevelRead(FS_TYPE_DataStorageId_T data_storage_id, UI32_T block_id, UI8_T *block_data_p);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_LowLevelGetBlockSize
 * ------------------------------------------------------------------------
 * FUNCTION : Get the block size of the specified data storage id
 *            data storage id and block id
 * INPUT    : data_storage_id - The id of the data storage
 * OUTPUT   : block_size_p    - The block size of the given data storage id
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 
 * ------------------------------------------------------------------------
 */
BOOL_T FS_NonVolatileDataStorage_LowLevelGetBlockSize(FS_TYPE_DataStorageId_T data_storage_id, UI32_T *block_size_p);

#endif /* _FS_H_ */
