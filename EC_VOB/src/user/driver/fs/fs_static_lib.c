/*-------------------------------------------------------------------------
 * Module Name:fs_static_lib.c
 *-------------------------------------------------------------------------
 * Purpose:
 *   fs.c will be compiled and linked as a so(shared object) library.
 *   This file(fs_static_lib.c) will be compiled and put in the file
 *   libfs_static.a to be used to link statically. The reason to put
 *   some sensitive functions in this .c file is to make it hard for making
 *   function calls out of the malicious purpose(e.g. tamper with the data
 *   managed by the FS data storage)
 *
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    09/16/2015 - Charlie Chen, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2015
 *-------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_module.h"

#include "l_cvrt.h"
#include "l_pbmp.h"
#include "l_math.h"

#include "backdoor_mgr.h"
#include "fs_type.h"
#include "fs.h"
#include "fs_om.h"

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
#define FS_FLASH_SYS_FS_MTD_PREFIX "/sys/class/mtd/"
/* special signature value for validation of FS_DataStorage_Header_T
 */
#define FS_DATA_STORAGE_MAGIC 0x9e7260daUL

#ifndef SYS_ADPT_FS_DATA_STORAGE_VER
#define FS_DATA_STORAGE_VER 0 /* default data storage version is 0 */
#else
#define FS_DATA_STORAGE_VER SYS_ADPT_FS_DATA_STORAGE_VER
#endif

#ifndef SYS_ADPT_FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE
#define FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE 8
#else
#define FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE SYS_ADPT_FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE
#endif


#ifndef SYS_ADPT_FS_DATA_STORAGE_HEADER_BLOCK_ID
#define FS_DATA_STORAGE_HEADER_BLOCK_ID 1 /* 0 is the first block */
#else
#define FS_DATA_STORAGE_HEADER_BLOCK_ID SYS_ADPT_FS_DATA_STORAGE_HEADER_BLOCK_ID
#endif

/* The data for storage ids will be written from the beginning of
 * the DATA_STORAGE_DATA_START_BLOCK_ID block of the mtd device for data storage
 */
#define DATA_STORAGE_DATA_START_BLOCK_ID (FS_DATA_STORAGE_HEADER_BLOCK_ID+1)

/* FS_DATA_STORAGE_TOTAL_DATA_REGION_ALU
 *     Total number of data region to keep the data for ALU. One data region
 *     keeps the data for one writing transaction.
 */
#define FS_DATA_STORAGE_TOTAL_DATA_REGION_ALU 3
#define FS_DATA_STORAGE_INVALID_REGION_ID (UI32_T)(-1)
#define FS_FAIL_SAFE_DATA_MAX_SEQ_NUM (UI32_T)(-1)

/* The data that is written in fail-safe method will be prepended to the data
 * defined in FS_DataStorage_FailSafeData_Header_T.
 */
typedef struct
{
    UI32_T data_len; /* length of data behind the header block */
    UI32_T seq_num;  /* sequence number of the data region
                      * Larger value means newer version of data unless
                      * the value reaches 0xFFFFFFFF and wraps to 0.
                      */
    UI32_T checksum; /* checksum of the data behind the header block */
} FS_DataStorage_FailSafeData_Header_T;

typedef struct
{
    /* the oldest sequence number among all of the valid fail-safe data regions
     */
    UI32_T oldest_data_seq_num;

    /* the region id(starts from 0) of the oldest sequence number among all of
     * the valid fail-safe data regions
     */
    UI32_T oldest_data_region_id;

    /* the newest sequence number among all of the valid fail-safe data regions
     */
    UI32_T newest_data_seq_num;

    /* the region id(starts from 0) of the newest sequence number among all of
     * the valid fail-safe data regions
     */
    UI32_T newest_data_region_id;

    /* the sequence number for each of the fail-safe data region
     * existing_seq_num[0] puts the sequence number for data region 0 and so on.
     * Note that existing_seq_num[] is valid only when the correspinding bit
     * in existing_seq_num_valid_bmp[] is set.
     */
    UI32_T existing_seq_num[FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE];

    /* bitmap to indicate the data region that contains valid fail-safe data
     * Bit 0 of element of array index 0 represents the valid bit of region 0.
     */
    UI8_T  existing_seq_num_valid_bmp[L_ALIGN(FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE, 8)/8];
} FS_FailSafeDataInfo_T;

#endif /* end of #if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE) */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define DBGMSG_EX(...)      if(FS_OM_GetDebugFlag()==TRUE)                      \
                                BACKDOOR_MGR_Printf(__VA_ARGS__);

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
static BOOL_T FS_InitDataStorageInfo(void);
static BOOL_T FS_NonVolatileDataStorage_ReadFailSafeData(FS_TYPE_DataStorageId_T data_storage_id, UI32_T *read_data_size_p, UI8_T *read_data_p);
static BOOL_T FS_NonVolatileDataStorage_GetFailSafeDataInfo(FS_TYPE_DataStorageId_T data_storage_id, FS_FailSafeDataInfo_T* data_info_p);
static BOOL_T FS_NonVolatileDataStorage_BlockIdToOffset(FS_TYPE_DataStorageId_T data_storage_id, UI32_T block_id, UI32_T* offset_p);
static BOOL_T FS_NonVolatileDataStorage_WriteFailSafeData(FS_TYPE_DataStorageId_T data_storage_id, UI32_T write_data_size, UI8_T *write_data_p);
static BOOL_T FS_NonVolatileDataStorage_LowLevelErase_Internal(FS_TYPE_DataStorageId_T data_storage_id, UI32_T start_block_id, UI32_T total_erase_block_num, BOOL_T take_fs_lock);
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
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
BOOL_T FS_InitDataStorageControl(void)
{
    char *shell_cmd_buf_p=NULL;
    char output_buf[32];
    UI32_T  data_storage_mtd_dev_id;
    UI32_T  output_buf_size, val;
    BOOL_T  ret=TRUE;

    if (FS_OM_GetMtdDevIdByMtdType(FS_MTD_PART_DATA_STORAGE, &data_storage_mtd_dev_id)==FALSE)
    {
        DBGMSG_EX("%s(%d)Failed to get mtd dev id\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    FS_OM_SetDataStorageMTDDevID(data_storage_mtd_dev_id);

    shell_cmd_buf_p=malloc(FS_MAX_PATHNAME_BUF_SIZE);
    if (shell_cmd_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to malloc %d bytes.\r\n", __FUNCTION__, __LINE__,
            FS_MAX_PATHNAME_BUF_SIZE);
        return FALSE;
    }

    /* get the size of the mtd device for data storage
     */
    snprintf(shell_cmd_buf_p, FS_MAX_PATHNAME_BUF_SIZE+1, "cat %s/mtd%lu/size", FS_FLASH_SYS_FS_MTD_PREFIX, data_storage_mtd_dev_id);
    output_buf_size=sizeof(output_buf);
    if (SYSFUN_GetExecuteCmdOutput(shell_cmd_buf_p, &output_buf_size, output_buf)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)SYSFUN_GetExecuteCmdOutput error(cmd=%s).\r\n", __FUNCTION__, __LINE__,
            shell_cmd_buf_p);
        ret=FALSE;
        goto exit;
    }
    val=strtoul(output_buf, NULL, 10);
    FS_OM_SetDataStorageMTDDevSize(val);

    /* get the block size of the mtd device for data storage
     */
    snprintf(shell_cmd_buf_p, FS_MAX_PATHNAME_BUF_SIZE+1, "cat %s/mtd%lu/erasesize", FS_FLASH_SYS_FS_MTD_PREFIX, data_storage_mtd_dev_id);
    output_buf_size=sizeof(output_buf);
    if (SYSFUN_GetExecuteCmdOutput(shell_cmd_buf_p, &output_buf_size, output_buf)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)SYSFUN_GetExecuteCmdOutput error(cmd=%s).\r\n", __FUNCTION__, __LINE__,
            shell_cmd_buf_p);
        ret=FALSE;
        goto exit;
    }
    val=strtoul(output_buf, NULL, 10);
    FS_OM_SetDataStorageBlockSize(val);
    if (FS_InitDataStorageInfo()==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_InitDataStorageInfo error.\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto exit;
    }

exit:
    free(shell_cmd_buf_p);
    return ret;
}

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
UI32_T FS_NonVolatileDataStorage_GetSize(FS_TYPE_DataStorageId_T data_storage_id)
{
    UI32_T storage_size, storage_offset;
    UI32_T data_region_size, data_region_total_num;
    BOOL_T rc;

    if (data_storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return 0;
    }

    /* the storage size and offset will not be changed after initialization
     * no need to use lock/unlock
     */
    /* FS_SEMAPHORE_LOCK(); */
    rc=FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &storage_offset);
    /* FS_SEMAPHORE_UNLOCK(); */
    if (rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error for data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return 0;
    }
    rc=FS_OM_GetDataStorageRegionInfo(data_storage_id, &data_region_size, &data_region_total_num);
    if (rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error for data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return 0;
    }

    switch (data_storage_id)
    {
        case FS_TYPE_DATA_STORAGE_ID_ALU:
            return (data_region_size - sizeof(FS_DataStorage_FailSafeData_Header_T));
            break;
        case FS_TYPE_DATA_STORAGE_ID_SYSTIME:
            return storage_size;
            break;
        default:
                BACKDOOR_MGR_Printf("%s(%d)Unknown data storage id %lu.\r\n",
                    __FUNCTION__, __LINE__, data_storage_id);
            break;
    }

    return 0;
}

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
BOOL_T FS_NonVolatileDataStorage_Write(FS_TYPE_DataStorageId_T data_storage_id, UI32_T write_data_size, UI8_T *write_data_p)
{
    BOOL_T ret;

    if (data_storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    switch (data_storage_id)
    {
        case FS_TYPE_DATA_STORAGE_ID_ALU:
            FS_LOCK(FALSE);
            ret=FS_NonVolatileDataStorage_WriteFailSafeData(data_storage_id, write_data_size, write_data_p);
            FS_UNLOCK();
            return ret;
            break;

        default:
            BACKDOOR_MGR_Printf("%s(%d)Not support data storage id %lu.\r\n",
                __FUNCTION__, __LINE__, data_storage_id);
            break;
    }
    return FALSE;
}

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
BOOL_T FS_NonVolatileDataStorage_Read(FS_TYPE_DataStorageId_T data_storage_id, UI32_T* read_data_size_p, UI8_T *read_data_p)
{
    BOOL_T ret;

    if (data_storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    switch (data_storage_id)
    {
        case FS_TYPE_DATA_STORAGE_ID_ALU:
            FS_LOCK(FALSE);
            ret=FS_NonVolatileDataStorage_ReadFailSafeData(data_storage_id, read_data_size_p, read_data_p);
            FS_UNLOCK();
            return ret;
            break;

        default:
            BACKDOOR_MGR_Printf("%s(%d)Not support data storage id %lu.\r\n",
                __FUNCTION__, __LINE__, data_storage_id);
            break;
    }
    return FALSE;
}

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
BOOL_T FS_NonVolatileDataStorage_LowLevelWrite(FS_TYPE_DataStorageId_T data_storage_id, UI32_T block_id, UI32_T offset, UI32_T write_data_size, UI8_T *write_data_p)
{
    UI32_T storage_size,storage_offset,block_size;
    UI32_T total_block_num, abs_offset, real_write_data_size;
    FS_TYPE_DataStorageServiceType_T service_type;
    BOOL_T ret;

    /* Only service type FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE is allowed
     * to do low level operation
     */
    if (FS_OM_GetDataStorageServiceType(data_storage_id, &service_type)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageServiceType error for data_storage_id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (service_type!=FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE)
    {
        BACKDOOR_MGR_Printf("%s(%d)service type %d not support (data_storage_id=%d)\r\n",
            __FUNCTION__, __LINE__, service_type, data_storage_id);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &storage_offset)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error, storage id %lu.\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }
    block_size=FS_OM_GetDataStorageBlockSize();
    total_block_num = storage_size/block_size;

    if (block_id >= total_block_num)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid block_id %lu(max=%lu) storage id %lu.\r\n",
            __FUNCTION__, __LINE__,block_id, total_block_num-1, data_storage_id);
        return FALSE;
    }

    if (offset>=FS_OM_GetDataStorageBlockSize())
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid offset 0x%08lX, data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, offset, data_storage_id);
        return FALSE;
    }

    if (write_data_size > (FS_OM_GetDataStorageBlockSize()-offset))
    {
        BACKDOOR_MGR_Printf("%s(%d)The data can only be written to the specified block id %lu, data storage id %lu.\r\n",
            __FUNCTION__, __LINE__, block_id, data_storage_id);
        return FALSE;
    }

    if (write_data_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)write_data_p is NULL, data storage id %lu\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    abs_offset=storage_offset + (block_size*block_id) + offset;
    real_write_data_size=write_data_size;
    FS_LOCK(FALSE);
    ret=FS_WriteMtdDev(FS_OM_GetDataStorageMTDDevID(),
        abs_offset, write_data_p, &real_write_data_size);
    FS_UNLOCK();

    if ((ret==TRUE) && (write_data_size!=real_write_data_size))
        ret=FALSE;

    return ret;
}

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
BOOL_T FS_NonVolatileDataStorage_OffsetToBlockId(FS_TYPE_DataStorageId_T data_storage_id, UI32_T offset, UI32_T *block_id_p)
{
    UI32_T storage_size, abs_storage_offset_base;
    UI32_T block_size;

    if (block_id_p==NULL)
    {
        DBGMSG_EX("%s(%d)block_id_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_LowLevelGetBlockSize(data_storage_id, &block_size)==FALSE)
    {
        DBGMSG_EX("%s(%d)FS_NonVolatileDataStorage_LowLevelGetBlockSize error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &abs_storage_offset_base)==FALSE)
    {
        DBGMSG_EX("%s(%d)FS_OM_GetDataStorageSizeAndOffset error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (offset >= storage_size)
    {
        DBGMSG_EX("%s(%d)Illegal offset(0x%08lX) for storage id %d\r\n", __FUNCTION__, __LINE__,
            offset, data_storage_id);
        return FALSE;
    }

    *block_id_p = offset/block_size;
    return TRUE;
}

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
BOOL_T FS_NonVolatileDataStorage_LowLevelErase(FS_TYPE_DataStorageId_T data_storage_id, UI32_T start_block_id, UI32_T total_erase_block_num)
{
    return FS_NonVolatileDataStorage_LowLevelErase_Internal(data_storage_id, start_block_id, total_erase_block_num, TRUE);
}

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
BOOL_T FS_NonVolatileDataStorage_LowLevelRead(FS_TYPE_DataStorageId_T data_storage_id, UI32_T block_id, UI8_T *block_data_p)
{
    UI32_T abs_offset, rel_offset, storage_size, storage_abs_offset_base;
    UI32_T block_size, read_data_len;

    if (block_data_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)block_data_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_LowLevelGetBlockSize(data_storage_id, &block_size)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_NonVolatileDataStorage_LowLevelGetBlockSize error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_BlockIdToOffset(data_storage_id, block_id, &rel_offset)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_NonVolatileDataStorage_BlockIdToOffset error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &storage_abs_offset_base)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    abs_offset=storage_abs_offset_base+rel_offset;
    read_data_len=block_size;
    if (FS_ReadMtdDev(FS_OM_GetDataStorageMTDDevID(), abs_offset, &read_data_len, block_data_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_ReadMtdDev error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    return TRUE;
}

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
BOOL_T FS_NonVolatileDataStorage_LowLevelGetBlockSize(FS_TYPE_DataStorageId_T data_storage_id, UI32_T *block_size_p)
{
    if (block_size_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)block_size_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (data_storage_id>=FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid data storage id %lu\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }
    *block_size_p=FS_OM_GetDataStorageBlockSize();
    return TRUE;
}

#endif /* end of #if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE)
/* FUNCTION NAME: FS_InitDataStorageInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Initialize the information for each data storage id
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE - Success, FALSE - Error
 *-----------------------------------------------------------------------------
 * NOTES: 1. This function is called when SYS_CPNT_FS_SUPPORT_DATA_STORAGE is TRUE
 *        2. Header is located at the beginning of the block FS
 *           DATA_STORAGE_HEADER_BLOCK_ID of the mtd device for data storage.
 *           The segment for each data storage id is allocated from the DATA_STORAGE_DATA_START_BLOCK_ID
 *           block.
 */
static BOOL_T FS_InitDataStorageInfo(void)
{
    FS_DataStorage_Header_T *header_from_mtd_p;
    FS_TYPE_DataStorageId_T storage_id, prev_storage_id;
    UI32_T data_len, offset, block_size, prev_offset;
    UI32_T data_storage_mtd_size, data_storage_total_size, checksum_in_header;
    UI32_T eval_checksum;
    BOOL_T ret_val=TRUE, is_valid_header_on_flash=FALSE;
    UI8_T  data_storage_mtd_dev_id;

/* SYS_ADPT_FS_DATA_STORAGE_SIZE_ALU:
 *   The maximum storage size that can be written for data storage id
 *   FS_TYPE_DATA_STORAGE_ID_ALU
 */
/* The data to be written to the data storage media will prepend with a
 * header(FS_DataStorage_FailSafeData_Header_T), need to count the size
 * of that header in for the real occupied size for FS_TYPE_DATA_STORAGE_ID_ALU
 */
#ifndef SYS_ADPT_FS_DATA_STORAGE_SIZE_ALU
#define FS_DATA_STORAGE_USER_VISABLE_SIZE_ALU (1*SYS_TYPE_1K)
#else
#define FS_DATA_STORAGE_USER_VISABLE_SIZE_ALU SYS_ADPT_FS_DATA_STORAGE_SIZE_ALU
#endif

#define FS_DATA_STORAGE_SIZE_ALU ((FS_DATA_STORAGE_USER_VISABLE_SIZE_ALU)+sizeof(FS_DataStorage_FailSafeData_Header_T))

#ifndef SYS_ADPT_FS_DATA_STORAGE_TOTAL_BLOCK_NUM_SYSTIME
#define FS_DATA_STORAGE_TOTAL_BLOCK_NUM_SYSTIME 2
#else
#define FS_DATA_STORAGE_TOTAL_BLOCK_NUM_SYSTIME SYS_ADPT_FS_DATA_STORAGE_TOTAL_BLOCK_NUM_SYSTIME
#endif

    header_from_mtd_p=(FS_DataStorage_Header_T*)malloc(sizeof(FS_DataStorage_Header_T));
    if (header_from_mtd_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)malloc failed.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    data_storage_mtd_dev_id=FS_OM_GetDataStorageMTDDevID();
    /* read header from the the data-storage mtd device
     */
    data_len=sizeof(FS_DataStorage_Header_T);
    block_size=FS_OM_GetDataStorageBlockSize();
    offset=block_size*FS_DATA_STORAGE_HEADER_BLOCK_ID;
    if (FS_ReadMtdDev(data_storage_mtd_dev_id, offset, &data_len, header_from_mtd_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_ReadMtdDev failed.\r\n", __FUNCTION__, __LINE__);
        ret_val=FALSE;
        goto exit;
    }
    if (data_len!=sizeof(FS_DataStorage_Header_T))
    {
        BACKDOOR_MGR_Printf("%s(%d)Incorrect data len(len=%lu, expect len=%lu)\r\n",
            __FUNCTION__, __LINE__, data_len, sizeof(FS_DataStorage_Header_T));
        ret_val=FALSE;
        goto exit;
    }

    /* validate header through magic byte and checksum
     */
    if (header_from_mtd_p->magic == FS_DATA_STORAGE_MAGIC)
    {
        checksum_in_header=header_from_mtd_p->checksum;
        header_from_mtd_p->checksum=0;
        eval_checksum=L_MATH_CheckSum(header_from_mtd_p, data_len);
        if (eval_checksum==checksum_in_header)
        {
            is_valid_header_on_flash=TRUE;
        }
        header_from_mtd_p->checksum=checksum_in_header;
    }

    if (is_valid_header_on_flash == FALSE)
    {
        FS_DataStorage_Header_T default_header;

        /* init default_header
         */
        memset(&default_header, 0, sizeof(default_header));
        default_header.magic=FS_DATA_STORAGE_MAGIC;
        default_header.version=FS_DATA_STORAGE_VER;

        /* init default setting for data storage service id FS_TYPE_DATA_STORAGE_ID_ALU
         */
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_ALU].data_region_size =
            L_ALIGN(FS_DATA_STORAGE_SIZE_ALU, block_size);
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_ALU].data_region_number =
            FS_DATA_STORAGE_TOTAL_DATA_REGION_ALU;
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_ALU].storage_size =
            default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_ALU].data_region_size *
            default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_ALU].data_region_number;
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_ALU].data_service_type =
            FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE;

        /* init default setting for data storage service id FS_TYPE_DATA_STORAGE_ID_SYSTIME
         */
        /* storage size for FS_TYPE_DATA_STORAGE_ID_SYSTIME is evaluated by
         * multiplying the block size by FS_DATA_STORAGE_TOTAL_BLOCK_NUM_SYSTIME
         */
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_SYSTIME].data_region_size = block_size;
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_SYSTIME].data_region_number = FS_DATA_STORAGE_TOTAL_BLOCK_NUM_SYSTIME;
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_SYSTIME].storage_size =
            default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_SYSTIME].data_region_size *
            default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_SYSTIME].data_region_number;
        default_header.info.data_storage_table[FS_TYPE_DATA_STORAGE_ID_SYSTIME].data_service_type =
            FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE;

        /* evaluate the offset for each data storage id
         */
        prev_offset = default_header.info.data_storage_table[0].abs_offset
            = block_size*DATA_STORAGE_DATA_START_BLOCK_ID; /* 0 is FS_TYPE_DATA_STORAGE_ID_ALU */
        prev_storage_id=(FS_TYPE_DataStorageId_T)0;
        for (storage_id=(FS_TYPE_DataStorageId_T)1; storage_id<FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM; storage_id++)
        {
            if (default_header.info.data_storage_table[storage_id].storage_size==0)
            {
                /* skip reserved data storage id
                 */
                break;
            }

            default_header.info.data_storage_table[storage_id].abs_offset=prev_offset+default_header.info.data_storage_table[prev_storage_id].storage_size;

            prev_offset=default_header.info.data_storage_table[storage_id].abs_offset;
            prev_storage_id=storage_id;
        }

        default_header.checksum=0;
        eval_checksum=L_MATH_CheckSum(&default_header, sizeof(FS_DataStorage_Header_T));
        default_header.checksum=eval_checksum;

        /* Write the default header if the current header on flash is invalid
         */
        offset=block_size*FS_DATA_STORAGE_HEADER_BLOCK_ID;
        if (FS_EraseMtdDev(data_storage_mtd_dev_id, offset, L_ALIGN(sizeof(FS_DataStorage_Header_T),block_size))==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)FS_EraseMtdDev failed.\r\n", __FUNCTION__, __LINE__);
            ret_val=FALSE;
            goto exit;
        }

        data_len=sizeof(FS_DataStorage_Header_T);
        offset=block_size*FS_DATA_STORAGE_HEADER_BLOCK_ID;
        if (FS_WriteMtdDev(data_storage_mtd_dev_id, offset, &default_header, &data_len)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)FS_WriteMtdDev failed.\r\n", __FUNCTION__, __LINE__);
            ret_val=FALSE;
            goto exit;
        }
        if (data_len!=sizeof(FS_DataStorage_Header_T))
        {
            BACKDOOR_MGR_Printf("%s(%d)Incorrect write data len(len=%lu, expected len=%lu).\r\n",
                __FUNCTION__, __LINE__, data_len, sizeof(FS_DataStorage_Header_T));
            ret_val=FALSE;
            goto exit;
        }

        *header_from_mtd_p=default_header;
    }
    else if (header_from_mtd_p->version != FS_DATA_STORAGE_VER)
    {
        /* add code to handle the change of the version if required
         */
        BACKDOOR_MGR_Printf("%s(%d)Do not know how to handle version 0x%08lX, reset data.\r\n",
            __FUNCTION__, __LINE__, header_from_mtd_p->version);
        ret_val=FALSE;
        goto exit;
    }

    /* validate the offset settings in data storage info
     */
    for (prev_offset=0, storage_id=0, data_storage_total_size=0;
        storage_id<FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM; storage_id++)
    {
        if (header_from_mtd_p->info.data_storage_table[storage_id].storage_size==0)
        {
            /* skip reserved data storage id
             */
            break;
        }

        data_storage_total_size+=header_from_mtd_p->info.data_storage_table[storage_id].storage_size;

        if (L_ALIGN(header_from_mtd_p->info.data_storage_table[storage_id].storage_size, block_size)!=header_from_mtd_p->info.data_storage_table[storage_id].storage_size)
        {
            BACKDOOR_MGR_Printf("%s(%d)Invalid storage_size 0x%08lX(block_size=0x%08lX) for storage id %lu\r\n",
                __FUNCTION__, __LINE__, header_from_mtd_p->info.data_storage_table[storage_id].storage_size,
                block_size, storage_id);
            ret_val=FALSE;
            goto exit;
        }

        if (L_ALIGN(header_from_mtd_p->info.data_storage_table[storage_id].abs_offset, block_size)!=header_from_mtd_p->info.data_storage_table[storage_id].abs_offset)
        {
            BACKDOOR_MGR_Printf("%s(%d)Invalid offset 0x%08lX(block_size=0x%08lX) for storage id %lu\r\n",
                __FUNCTION__, __LINE__, header_from_mtd_p->info.data_storage_table[storage_id].abs_offset,
                block_size, storage_id);
            ret_val=FALSE;
            goto exit;
        }

        if (header_from_mtd_p->info.data_storage_table[storage_id].abs_offset<prev_offset)
        {
            BACKDOOR_MGR_Printf("%s(%d)Invalid offset for storage id %lu(current offset=0x%08lX, prev offset=0x%08lX).\r\n",
                __FUNCTION__, __LINE__, storage_id, header_from_mtd_p->info.data_storage_table[storage_id].abs_offset, prev_offset);
            ret_val=FALSE;
            goto exit;
        }
        else
        {
            prev_offset=header_from_mtd_p->info.data_storage_table[storage_id].abs_offset;
        }
    }
    data_storage_mtd_size=FS_OM_GetDataStorageMTDDevSize();
    if (data_storage_total_size>data_storage_mtd_size)
    {
        BACKDOOR_MGR_Printf("%s(%d)Illegal data storage size settings(total data storage size=0x%08lX, mtd size=0x%08lX.\r\n",
            __FUNCTION__, __LINE__, data_storage_total_size, data_storage_mtd_size);
        ret_val=FALSE;
        goto exit;
    }


    /* update data storage info to FS_OM
     */
    if (FS_OM_SetDataStorageInfo(&(header_from_mtd_p->info))==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_SetDataStorageInfo error\r\n",
            __FUNCTION__, __LINE__);
        ret_val=FALSE;
    }

exit:
    free(header_from_mtd_p);
    return ret_val;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_ReadFailSafeData
 * ------------------------------------------------------------------------
 * FUNCTION : This function read the newest version of the checksum-verified
 *            data for the specified data storage id.
 * INPUT    : read_data_size_p - The size of the data to be read
 * OUTPUT   : read_data_size_p - The actual size of the read data
 *            read_data_p      - The read data will be put to this buffer.
 *                               The size of the buffer must be larger than
 *                               input value of *read_data_size_p.
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 1. Only FS_TYPE_DATA_STORAGE_ID_ALU can call this function
 *               currently.
 *            2. If *read_data_size_p is less than the actual data size,
 *               only the leading *read_data_size_p bytes will be output.
 *            3. The caller of this function shall hold fs lock.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_NonVolatileDataStorage_ReadFailSafeData(FS_TYPE_DataStorageId_T data_storage_id, UI32_T *read_data_size_p, UI8_T *read_data_p)
{
    FS_FailSafeDataInfo_T data_info;
    FS_DataStorage_FailSafeData_Header_T header;
    UI32_T storage_size, storage_abs_offset_base;
    UI32_T data_region_size, data_region_total_num, read_len;

    if (data_storage_id!=FS_TYPE_DATA_STORAGE_ID_ALU)
    {
        BACKDOOR_MGR_Printf("%s(%d)data_storage_id %d not support\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if ((read_data_size_p==NULL) || (read_data_p==NULL))
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid input argument\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_GetFailSafeDataInfo(data_storage_id, &data_info)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_NonVolatileDataStorage_GetFailSafeDataInfo error. data_storage_id=%d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &storage_abs_offset_base)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error. data_storage_id=%d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (storage_size==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid storage size for data_storage_id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (L_PBMP_IS_ALL_ZEROS_PBMP_ARRAY(data_info.existing_seq_num_valid_bmp, FS_DATA_STORAGE_MAX_NUMBER_OF_DATA_REGION_FOR_ONE_STORAGE_SERVICE))
    {
        DBGMSG_EX("%s(%d)No valid data region is found for data_storage_id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (FS_OM_GetDataStorageRegionInfo(data_storage_id, &data_region_size, &data_region_total_num)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageRegionInfo error for data_storage_id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    read_len = sizeof(FS_DataStorage_FailSafeData_Header_T);
    if (FS_ReadMtdDev(FS_OM_GetDataStorageMTDDevID(),
        storage_abs_offset_base + (data_region_size*data_info.newest_data_region_id),
        &read_len, &header)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_ReadMtdDev for header data error(data_storage_id=%d)\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (read_len != sizeof(FS_DataStorage_FailSafeData_Header_T))
    {
        BACKDOOR_MGR_Printf("%s(%d)read len err(actual read len=%lu, real read len=%lu)\r\n",
            __FUNCTION__, __LINE__, read_len, sizeof(FS_DataStorage_FailSafeData_Header_T));
        return FALSE;
    }

    if (*read_data_size_p>header.data_len)
    {
        DBGMSG_EX("%s(%d)The request data length is larger than the actual data length(request=%lu,actual=%lu)\r\n",
            __FUNCTION__, __LINE__, *read_data_size_p, header.data_len);
        read_len = header.data_len;
    }
    else
    {
        read_len = *read_data_size_p;
    }

    if (FS_ReadMtdDev(FS_OM_GetDataStorageMTDDevID(),
        storage_abs_offset_base + (data_region_size*data_info.newest_data_region_id) +
        sizeof(FS_DataStorage_FailSafeData_Header_T),
        &read_len, read_data_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_ReadMtdDev for data error(data_storage_id=%d)\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }
    *read_data_size_p=read_len;

    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_GetFailSafeDataInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function scans all of the regions of the fail-safe data
 *            of the specified data_storage_id and output the information
 *            that is required for doing read/write operations.
 * INPUT    : data_storage_id  - The id of the data storage
 * OUTPUT   : data_info_p      - The info of the fail-safe data of the specified
 *                               data storage id
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_NonVolatileDataStorage_GetFailSafeDataInfo(FS_TYPE_DataStorageId_T data_storage_id, FS_FailSafeDataInfo_T* data_info_p)
{
    FS_DataStorage_FailSafeData_Header_T *header_p;
    UI32_T storage_size, abs_storage_offset_base, storage_offset, abs_storage_offset;
    UI32_T region, region_size, region_total_num;
    UI32_T len;
    UI32_T checksum, checksum_in_header;
    UI8_T  *region_data_p=NULL;
    BOOL_T rc, ret=TRUE, is_max_seq_num_exist=FALSE;
    BOOL_T first_valid_region_is_checked;

    if (data_info_p==NULL)
    {
        DBGMSG_EX("%s(%d)data_info_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &abs_storage_offset_base)==FALSE)
    {
        DBGMSG_EX("%s(%d)Error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (storage_size==0)
    {
        DBGMSG_EX("%s(%d)Error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* init data_info_p
     */
    memset(data_info_p, 0, sizeof(FS_FailSafeDataInfo_T));

    if (FS_OM_GetDataStorageRegionInfo(data_storage_id, &region_size, &region_total_num)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageRegionInfo error for data_storage_id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    region_data_p = (UI8_T*)L_MM_Malloc(region_size, L_MM_USER_ID2(SYS_MODULE_FS,FS_TYPE_TRACE_ID_FS_GETFAILSAFEDATANEXTWRITEINFO));
    header_p=(FS_DataStorage_FailSafeData_Header_T*)region_data_p;

    if (region_data_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)malloc %lu bytes failed.\r\n",
            __FUNCTION__, __LINE__, region_size);
        return FALSE;
    }

    for (region=0; region<region_total_num; region++)
    {
        abs_storage_offset=abs_storage_offset_base+region_size*region;
        len=sizeof(FS_DataStorage_FailSafeData_Header_T);
        if (FS_ReadMtdDev(FS_OM_GetDataStorageMTDDevID(), abs_storage_offset,
            &len, header_p) == FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)FS_ReadMtdDev error\r\n",
                __FUNCTION__, __LINE__);
            ret=FALSE;
            goto exit;
        }

        if (len!=sizeof(FS_DataStorage_FailSafeData_Header_T))
        {
            BACKDOOR_MGR_Printf("%s(%d)Read len incorrect(len=%lu, expected len=%lu)\r\n",
                __FUNCTION__, __LINE__, len, sizeof(FS_DataStorage_FailSafeData_Header_T));
            ret=FALSE;
            goto exit;
        }

        /* the reasonable header_p->data_len should not be larger than 0x7FFFFFFF
         * so convert it as I32_T and check whether is it positive value
         */
        if (((I32_T)header_p->data_len<0) || (header_p->data_len+sizeof(FS_DataStorage_FailSafeData_Header_T))>region_size)
        {
            /* Illegal header_p->data_len, an empty region or data corrupted
             * region which can be used for the next write transaction
             */
            if (header_p->data_len!=0xFFFFFFFFUL)
            {
                DBGMSG_EX("%s(%d)Illegal data_len in header.(val=0x%08lX, max=0x%08lX)\r\n",
                    __FUNCTION__, __LINE__,(header_p->data_len+sizeof(FS_DataStorage_FailSafeData_Header_T)),
                    region_size);
            }
            continue;
        }

        len=header_p->data_len;
        rc=FS_ReadMtdDev(FS_OM_GetDataStorageMTDDevID(), abs_storage_offset+sizeof(FS_DataStorage_FailSafeData_Header_T),
            &len, (header_p+1));
        if (rc == FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)FS_ReadMtdDev error\r\n",
                __FUNCTION__, __LINE__);
            ret=FALSE;
            goto exit;
        }

        if (len!=header_p->data_len)
        {
            BACKDOOR_MGR_Printf("%s(%d)Read len incorrect(len=%lu, expected len=%lu)\r\n",
                __FUNCTION__, __LINE__, len, header_p->data_len);
            ret=FALSE;
            goto exit;
        }

        checksum_in_header=header_p->checksum;
        header_p->checksum=0; /* set header_p->checksum as 0 to evalute the checksum of the data in header */
        checksum=L_MATH_CheckSum((void*)(header_p), sizeof(FS_DataStorage_FailSafeData_Header_T));
        checksum^=L_MATH_CheckSum((void*)(header_p+1), header_p->data_len);
        if (checksum!=checksum_in_header)
        {
            /* Illegal checksum
             */
            DBGMSG_EX("%s(%d)Illegal checksum in header.(checksum=0x%08lX, expected checksum=0x%08lX\r\n",
                __FUNCTION__, __LINE__, checksum_in_header, checksum);
            continue;
        }
        L_PBMP_SET_PORT_IN_PBMP_ARRAY(data_info_p->existing_seq_num_valid_bmp, region+1);
        data_info_p->existing_seq_num[region]=header_p->seq_num;

        if (header_p->seq_num==FS_FAIL_SAFE_DATA_MAX_SEQ_NUM)
        {
            is_max_seq_num_exist=TRUE;
        }
    }

    /* search for the region id and sequence number of the oldest and the
     * newest data
     */
    first_valid_region_is_checked=FALSE;
    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(data_info_p->existing_seq_num_valid_bmp, region_total_num, region)
    {
        /* region used in L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY is started from 1
         * need to decrease by one to get 0-based index
         */
        if (first_valid_region_is_checked==FALSE)
        {
            data_info_p->newest_data_seq_num=data_info_p->oldest_data_seq_num=data_info_p->existing_seq_num[region-1];
            data_info_p->oldest_data_region_id=region-1;
            first_valid_region_is_checked=TRUE;
            continue;
        }

        /* convert to I32_T in order to find the oldest sequence number
         * and the newest sequence number correctly when the sequence numbers
         * fall around the FS_FAIL_SAFE_DATA_MAX_SEQ_NUM(i.e. 0xFFFFFFFF)
         */
        if (is_max_seq_num_exist==TRUE)
        {
            if ((I32_T)data_info_p->existing_seq_num[region-1] < (I32_T)data_info_p->oldest_data_seq_num)
            {
                data_info_p->oldest_data_seq_num=data_info_p->existing_seq_num[region-1];
                data_info_p->oldest_data_region_id=region-1;
            }

            if ((I32_T)data_info_p->existing_seq_num[region-1] > (I32_T)data_info_p->newest_data_seq_num)
            {
                data_info_p->newest_data_seq_num=data_info_p->existing_seq_num[region-1];
                data_info_p->newest_data_region_id=region-1;
            }
        }
        else
        {
            if (data_info_p->existing_seq_num[region-1] < data_info_p->oldest_data_seq_num)
            {
                data_info_p->oldest_data_seq_num=data_info_p->existing_seq_num[region-1];
                data_info_p->oldest_data_region_id=region-1;
            }

            if (data_info_p->existing_seq_num[region-1] > data_info_p->newest_data_seq_num)
            {
                data_info_p->newest_data_seq_num=data_info_p->existing_seq_num[region-1];
                data_info_p->newest_data_region_id=region-1;
            }

        }
    }

exit:
    if (region_data_p)
        L_MM_Free(region_data_p);

    return ret;

}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_BlockIdToOffset
 * ------------------------------------------------------------------------
 * FUNCTION : This function converts the given storage id and block id into
 *            offset.
 * INPUT    : data_storage_id - The id of the data storage
 *            block_id        - The block id of the specified offset and data
 *                              storage id. 0 means the first block.
 * OUTPUT   : offset_p        - Offset to the beginning of the specified data
 *                              storage id
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_NonVolatileDataStorage_BlockIdToOffset(FS_TYPE_DataStorageId_T data_storage_id, UI32_T block_id, UI32_T* offset_p)
{
    UI32_T storage_size, abs_storage_offset_base, max_block_id;
    UI32_T block_size;

    if (offset_p==NULL)
    {
        DBGMSG_EX("%s(%d)block_id_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_LowLevelGetBlockSize(data_storage_id, &block_size)==FALSE)
    {
        DBGMSG_EX("%s(%d)FS_NonVolatileDataStorage_LowLevelGetBlockSize error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &abs_storage_offset_base)==FALSE)
    {
        DBGMSG_EX("%s(%d)FS_OM_GetDataStorageSizeAndOffset error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (storage_size==0)
    {
        DBGMSG_EX("%s(%d)Illegal storage size for data storage id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }
    max_block_id = (storage_size/block_size)-1;

    if (block_id > max_block_id)
    {
        DBGMSG_EX("%s(%d)block id %lu for storage id %d\r\n", __FUNCTION__, __LINE__,
            block_id, data_storage_id);
        return FALSE;
    }

    *offset_p = block_id * FS_OM_GetDataStorageBlockSize();
    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_WriteFailSafeData
 * ------------------------------------------------------------------------
 * FUNCTION : This function writes the given data to the data storage media
 *            with a fail safe mechanism. If the write operation is interrupted
 *            unexpectedly, the data written by the success write operation
 *            would be used when calling the function
 *            FS_NonVolatileDataStorage_ReadDataFailSafe() to read the data.
 * INPUT    : data_storage_id - The id of the data storage
 *            write_data_size - The size of data to be written
 *            write_data_p    - The data to be written
 * OUTPUT   : None
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 1. Only service type FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE
 *               can call this function currently.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_NonVolatileDataStorage_WriteFailSafeData(FS_TYPE_DataStorageId_T data_storage_id, UI32_T write_data_size, UI8_T *write_data_p)
{
    UI32_T data_region_id, write_data_region_id=FS_DATA_STORAGE_INVALID_REGION_ID;
    UI32_T checksum, data_region_size, data_region_total_num;
    UI32_T storage_size, storage_offset, block_size;
    FS_FailSafeDataInfo_T data_info;
    UI32_T write_offset, real_write_len;
    UI32_T new_data_seq_num;
    FS_DataStorage_FailSafeData_Header_T header;
    FS_TYPE_DataStorageServiceType_T service_type;

    if (FS_OM_GetDataStorageServiceType(data_storage_id, &service_type)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageServiceType error for data_storage_id %d\r\n",
            __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }
    /* only FS_TYPE_DATA_STORAGE_ID_ALU uses fail-safe writing operation
     */
    if (service_type!=FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Not support service type %d (data_storage_id=%d)\r\n",
            __FUNCTION__, __LINE__, service_type, data_storage_id);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_LowLevelGetBlockSize(data_storage_id, &block_size)==FALSE)
    {
        DBGMSG_EX("%s(%d)FS_NonVolatileDataStorage_LowLevelGetBlockSize error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* get the info for this writing transaction
     */
    if (FS_NonVolatileDataStorage_GetFailSafeDataInfo(data_storage_id, &data_info) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_NonVolatileDataStorage_GetFailSafeDataInfo error\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_OM_GetDataStorageRegionInfo(data_storage_id, &data_region_size, &data_region_total_num) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageRegionInfo error\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* evaluate the data region id for the new data
     */
    for (data_region_id=0; data_region_id<data_region_total_num; data_region_id++)
    {
        /* search for unused data region
         */
        if (!L_PBMP_GET_PORT_IN_PBMP_ARRAY(data_info.existing_seq_num_valid_bmp, data_region_id+1))
        {
            write_data_region_id=data_region_id;
            break;
        }
    }

    if (write_data_region_id==FS_DATA_STORAGE_INVALID_REGION_ID)
    {
        /* all data regions are occupied
         * overwrite the data region with the oldest version of data
         */
        write_data_region_id=data_info.oldest_data_region_id;
    }

    /* evaluate the sequence number for the new data
     */
    new_data_seq_num=data_info.newest_data_seq_num+1;
    memset(&header, 0, sizeof(header));


    header.data_len=write_data_size;
    header.seq_num=new_data_seq_num;
    header.checksum=0;

    /* evaluate the checksum of the header first
     */
    checksum=L_MATH_CheckSum((void *)&header, sizeof(header));

    /* evaluate the checksum of the data part
     */
    checksum^=L_MATH_CheckSum((void*)write_data_p, write_data_size);

    /* fill the evaluated checksum to the header
     */
    header.checksum=checksum;

    /* evaluate the start writing absolute offset
     */
    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &storage_offset)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    write_offset=data_region_size*write_data_region_id;

    /* perform erase for the blocks to be written
     */
    if (FS_EraseMtdDev(FS_OM_GetDataStorageMTDDevID(), storage_offset + write_offset, data_region_size)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_EraseMtdDev error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* write data to the mtd device
     */
    real_write_len=sizeof(header);
    if (FS_WriteMtdDev(FS_OM_GetDataStorageMTDDevID(), storage_offset + write_offset,
        &header, &real_write_len) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_WriteMtdDev for header error\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (real_write_len!=sizeof(header))
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_WriteMtdDev for header error(expected_write_len=%lu,real_write_len=%lu)\r\n",
            __FUNCTION__, __LINE__, sizeof(header), real_write_len);
        return FALSE;
    }

    real_write_len=write_data_size;
    if (FS_WriteMtdDev(FS_OM_GetDataStorageMTDDevID(), storage_offset + write_offset + sizeof(header),
        write_data_p, &write_data_size) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_WriteMtdDev for data error\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (real_write_len!=write_data_size)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_WriteMtdDev for header error(expected_write_len=%lu,real_write_len=%lu)\r\n",
            __FUNCTION__, __LINE__, write_data_size, real_write_len);
        return FALSE;
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NonVolatileDataStorage_LowLevelErase_Internal
 * ------------------------------------------------------------------------
 * FUNCTION : Perform low level erase operation according to the given
 *            data storage id, start block id and total number of erase block.
 * INPUT    : data_storage_id - The id of the data storage
 *            start_block_id  - The starting block id to perform erase operation
 *            total_erase_block_num
 *                            - Total number of block to perform erase operation
 *            take_fs_lock    - TRUE: take fs lock when doing erase operation
 *                              FALSE: do not take fs lock when doing erase operation
 * OUTPUT   : None
 * RETURN   : TRUE  -  Successfully
 *            FALSE -  Error
 * NOTE     : 1. Only service type FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE
 *               can call this function currently.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_NonVolatileDataStorage_LowLevelErase_Internal(FS_TYPE_DataStorageId_T data_storage_id, UI32_T start_block_id, UI32_T total_erase_block_num, BOOL_T take_fs_lock)
{
    FS_TYPE_DataStorageServiceType_T service_type;
    UI32_T storage_size, abs_storage_offset_base, rel_offset, total_block_num, block_size;
    BOOL_T rc;

    if (FS_OM_GetDataStorageServiceType(data_storage_id, &service_type)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageServiceType error for data_storage_id %d\r\n", __FUNCTION__, __LINE__, data_storage_id);
        return FALSE;
    }

    if (service_type!=FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE)
    {
        BACKDOOR_MGR_Printf("%s(%d)service type %d not support (data_storage_id=%d)\r\n",
            __FUNCTION__, __LINE__, service_type, data_storage_id);
        return FALSE;
    }

    if (FS_OM_GetDataStorageSizeAndOffset(data_storage_id, &storage_size, &abs_storage_offset_base)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_OM_GetDataStorageSizeAndOffset error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_LowLevelGetBlockSize(data_storage_id, &block_size)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_NonVolatileDataStorage_LowLevelGetBlockSize error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (total_erase_block_num==0)
    {
        return TRUE;
    }

    total_block_num=storage_size/block_size;
    if (total_erase_block_num>total_block_num)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid total erase block number %lu for data storage id %lu\r\n",
            __FUNCTION__, __LINE__, total_erase_block_num, data_storage_id);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_BlockIdToOffset(data_storage_id, start_block_id, &rel_offset)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_NonVolatileDataStorage_BlockIdToOffset error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (take_fs_lock==TRUE)
    {
        FS_LOCK(FALSE);
    }

    rc=FS_EraseMtdDev(FS_OM_GetDataStorageMTDDevID(), abs_storage_offset_base+rel_offset, total_erase_block_num*FS_OM_GetDataStorageBlockSize());
    if (take_fs_lock==TRUE)
    {
        FS_UNLOCK();
    }

    if (rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_EraseMtdDev error\r\n", __FUNCTION__, __LINE__);
    }

    return rc;
}

#endif /* end of #if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE==TRUE) */

