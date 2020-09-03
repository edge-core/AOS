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

typedef struct FS_HW_Info_S
{
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN];                    /* MAC address */
    UI8_T   serial_no[SYS_ADPT_SERIAL_NO_STR_LEN + 1];          /* serial number */
    UI8_T   agent_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];          /* agent board hardware version (len 5+1) */
    UI8_T   manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN];    /* Product date, option to key in */
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
    UI8_T   reserved2[SYS_ADPT_RESERVED_INFO_LEN + (SYS_ADPT_RESERVED_INFO_LEN - 21 - SYS_ADPT_MODEL_NUMBER_LEN - 1 )];              /* reserved for more board info */
    UI8_T   reserved3[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved4[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved5[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved6[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved7[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI8_T   reserved8[SYS_ADPT_RESERVED_INFO_LEN];              /* reserved for more board info */
    UI16_T  check_sum;                                          /* check sum */
}__attribute__((packed, aligned(1))) FS_HW_Info_T;



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
FS_File_Type_T  FS_GetImageType(UI8_T *data_p, UI32_T length);

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

#endif /* _FS_H_ */
