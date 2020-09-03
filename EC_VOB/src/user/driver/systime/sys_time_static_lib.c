/* MODULE NAME:  sys_time_static_lib.c
 * PURPOSE:
 *   sys_time.c will be compiled and linked as a so(shared object) library.
 *   This file(sys_time_static_lib.c) will be compiled and put in the file
 *   lib_sys_time_static.a to be used to link statically. The reason to put
 *   some sensitive functions in this .c file is to make it hard for making
 *   function calls out of the malicious purpose(e.g. tamper with the license
 *   expired time)
 *
 * 
 * NOTES:
 *
 *
 * HISTORY
 *    9/8/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sys_cpnt.h"

#if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE)
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "l_cvrt.h"

#include "sysrsc_mgr.h"
#include "l_math.h"
#include "l_bitmap.h"
#include "fs_type.h"
#include "fs.h"
#include "sys_time.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SYS_TIME_ACCUMULATED_SYS_UP_TIME_INFO_BLOCK_HEADER_MAGIC 0x3e7ab253UL
#define MAX_SEQ_NUM 0xFFFFFFFFUL

/* MACRO FUNCTION DECLARATIONS
 */
#define SYS_TIME_OM_ENTER_CRITICAL_SECTION() \
    SYSFUN_TakeSem(sys_time_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYS_TIME_OM_LEAVE_CRITICAL_SECTION() \
    SYSFUN_GiveSem(sys_time_om_sem_id)

#define INIT_ACCUMULATED_CONTROL_BLOCK_FOR_BRAND_NEW_DATA_BLOCK(ctl_block_p, accumulated_system_up_time_base, previous_bump_up_sys_up_tick, new_data_block_id, new_seq_num) \
    (ctl_block_p)->accumulated_sys_up_time_base = (accumulated_system_up_time_base); \
    (ctl_block_p)->bump_up_count=0; \
    (ctl_block_p)->prev_bump_up_sys_up_tick=(previous_bump_up_sys_up_tick); \
    (ctl_block_p)->cursor=sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T); \
    (ctl_block_p)->active_data_block_id=(new_data_block_id); \
    (ctl_block_p)->active_seq_num=(new_seq_num); \
    (ctl_block_p)->write_data=0x7F; \
    (ctl_block_p)->flip_bit_mask=0x80

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SYS_TIME_GetNextWriteBlockId(const SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p, const UI32_T *seq_num_ar_p, const UI8_T *valid_block_ar_p, UI32_T *next_write_block_id_p);
static BOOL_T SYS_TIME_GetNewestBlockId(const SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p, UI32_T *seq_num_ar_p, UI8_T *valid_block_ar_p, UI32_T *newest_block_id_p);
static BOOL_T SYS_TIME_AnalyzeSysTimeDataStorageDataStatus(const SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p, UI32_T *seq_num_ar_p, UI8_T  *valid_block_ar_p, BOOL_T *no_valid_block_exist_p);

/* STATIC VARIABLE DECLARATIONS
 */
static SYS_TIME_Shmem_Data_T *shmem_data_p=NULL;
static UI32_T sys_time_om_sem_id;

/* LOCAL INLINE SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_InitLocalStaticVariables
 * ------------------------------------------------------------------------
 * FUNCTION : This function initializes the local static variables declared
 *            in this file.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static inline void SYS_TIME_InitLocalStaticVariables(void)
{
    shmem_data_p = (SYS_TIME_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYS_TIME_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSTIME_OM, &sys_time_om_sem_id);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_TIME_GetAccumulatedSysUpCtlBlkPtr
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the pointer to accumulated system up time control block.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  Pointer to accumulated system up time control block
 * NOTES:   None
 *---------------------------------------------------------------------------------*/
static inline SYS_TIME_AccumulatedSysUpTimeControlBlock_T* SYS_TIME_GetAccumulatedSysUpCtlBlkPtr(void)
{
    if (shmem_data_p==NULL)
    {
        SYS_TIME_InitLocalStaticVariables();
    }

    return &(shmem_data_p->accumulated_sys_up_time_ctl_blk);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_TIME_FillAccumulatedSysUpTimeHeaderChecksum
 *---------------------------------------------------------------------------------
 * PURPOSE: Evaluate the checksum for the data in the specified header and fill
 *          the checksum into the header
 * INPUT:   header_p - fields other than "header_p->checksum" will be used to
 *                     evaulate the checksum
 * OUTPUT:  header_p->checksum -
 *                     The calculated checksum will be filled to this field.
 * RETUEN:  None
 * NOTES:   None
 *---------------------------------------------------------------------------------*/
static inline void SYS_TIME_FillAccumulatedSysUpTimeHeaderChecksum(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T *header_p)
{
    UI32_T checksum;

    header_p->checksum=0;
    checksum=L_MATH_CheckSum((void*)header_p, sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T));
    header_p->checksum=checksum;
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_InitBrandNewBlockHeader
 * ------------------------------------------------------------------------
 * FUNCTION : Initialize the fields of a brand new block header.
 * INPUT    : seq_num - the sequence number to be filled to the header
 *            accumulated_sys_up_time_base -
 *                      the accumulated system up time base to be filled to the
 *                      header
 * OUTPUT   : header_p -the pointer to the header to be initialized.
 * RETURN   : None.
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static inline void SYS_TIME_InitBrandNewBlockHeader(UI32_T seq_num, UI64_T accumulated_sys_up_time_base, SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T *header_p)
{
    header_p->magic=SYS_TIME_ACCUMULATED_SYS_UP_TIME_INFO_BLOCK_HEADER_MAGIC;
    header_p->seq_num=seq_num;
    header_p->accumulated_sys_up_time_base=accumulated_sys_up_time_base;
    memset(header_p->reserved, 0, sizeof(header_p->reserved));
    SYS_TIME_FillAccumulatedSysUpTimeHeaderChecksum(header_p);
}

/* EXTERN FUNCTION DECLARATIONS
 */
extern void SYS_TIME_GetSystemUpTimeByTick_Internal(BOOL_T take_sem, UI32_T *ticks); /* internal function for SYS_TIME only and is defined in sys_time.c */

/* EXPORTED SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_AccumulatedUpTime_Get
 * ------------------------------------------------------------------------
 * FUNCTION : This function gets the accumulated system up time(in minute).
 * INPUT    : None.
 * OUTPUT   : accumulated_time  -  The accumulated system up time(in minute)
 * RETURN   : TRUE  -  The accumulated system up time is output successfully.
 *            FALSE -  Error to get accumulated system up time.
 * NOTE     : The accumulated system up time will be kept counting whenever
 *            the system is up.
 * ------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_AccumulatedUpTime_Get(UI64_T *accumulated_time_p)
{
    SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p;
#if 0
    UI32_T current_sys_up_tick;
#endif

    if (accumulated_time_p==NULL)
    {
        printf("%s(%d)accumulated_time_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    block_p=SYS_TIME_GetAccumulatedSysUpCtlBlkPtr();
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *accumulated_time_p=(UI64_T)block_p->accumulated_sys_up_time_base + (UI64_T)(block_p->bump_up_count * SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE);

/* Do not count the accumulated system up time that is not written to fs data
 * storage yet. If count that time in, the system accumulated up time got by
 * ALU might be less than the time got in the previous time.
 * Consider the scenario described below:
 *   1. The device had been up for 15 minutes and the time had been got by ALU.
 *      But SYS_TIME write the up time tp fs data storage every 10 minutes, the
 *      info of the accumulated system up time in the data storage is 10 minutes
 *      at that moment.
 *   2. Power off the device and power on the device. After boot up the device,
 *      ALU will get the accumulated system up time as 10 minutes, which is
 *      less than the value(i.e. 15 minutes) got by ALU in the previous time.
 *
 * ALU will consider the scenario described above as error. So only output the
 * accumulated system up time which had been written to the fs data storage in
 * all time.
 */
#if 0 
    SYS_TIME_GetSystemUpTimeByTick_Internal(FALSE, &current_sys_up_tick);
    *accumulated_time_p += (UI64_T)(((I32_T)current_sys_up_tick - (I32_T)block_p->prev_bump_up_sys_up_tick)/(SYS_BLD_TICKS_PER_SECOND*60));
#endif
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
    
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_BumpUpAccumulatedSysUpTime
 * ------------------------------------------------------------------------
 * FUNCTION : This function bumps up the statistic of the accumulated systime
 *            up time.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : 1. The accumulated system up time will be kept counting whenever
 *               the system is up.
 *            2. The accumulated system up time will be increased by
 *               SYS_TIME_ACCUMULATED_SYS_UP_TIME_ONE_TIME_SLICE_IN_TICK for
 *               each call to this function.
 *            3. This function should be called by a thread periodically to
 *               update the accumulated system up time.
 * ------------------------------------------------------------------------
 */
void SYS_TIME_BumpUpAccumulatedSysUpTime(void)
{
    SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p;
    UI32_T elapsed_time_in_tick, current_sys_up_tick;

    block_p=SYS_TIME_GetAccumulatedSysUpCtlBlkPtr();

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    if (block_p->data_block_size==0) /* shared memory had not been initialized, should not happen. */
    {
        printf("%s(%d)Warning. Shared memory is not initialized at appropriate time.\r\n",
            __FUNCTION__, __LINE__);

        if (SYS_TIME_InitAccumulatedSysUpTimeContrlBlock()==FALSE)
        {
            printf("%s(%d)SYS_TIME_InitAccumulatedSysUpTimeContrlBlock error.\r\n",
                __FUNCTION__, __LINE__);
        }
        goto exit_2;
    }

    SYS_TIME_GetSystemUpTimeByTick_Internal(FALSE, &current_sys_up_tick);
    /* convert to signed value to handle the condition that sys tick wrap to 0
     * this evaluation would be correct as long as the difference is less than
     * 0x80000000 ticks.
     */
    elapsed_time_in_tick=(I32_T)current_sys_up_tick - (I32_T)(block_p->prev_bump_up_sys_up_tick);
    if (elapsed_time_in_tick < (SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE*60*SYS_BLD_TICKS_PER_SECOND))
    {
        goto exit_2;
    }
    block_p->prev_bump_up_sys_up_tick=current_sys_up_tick;

do_again:
    block_p->bump_up_count++;

    /* write data through FS API
     */
    if (FS_NonVolatileDataStorage_LowLevelWrite(FS_TYPE_DATA_STORAGE_ID_SYSTIME,
        block_p->active_data_block_id, block_p->cursor, 1, &(block_p->write_data))==FALSE)
    {
        printf("%s(%d)FS_NonVolatileDataStorage_LowLevelWrite failed(block_id=%lu, cursor=%lu)\r\n",
            __FUNCTION__, __LINE__, block_p->active_data_block_id, block_p->cursor);
        goto exit_2;
    }

    /* shift right 1 bit for next flip bit
     */
    block_p->flip_bit_mask>>=1;
    if (block_p->flip_bit_mask!=0)
    {
        /* turn off the bit in write_data according to mask
         */
        block_p->write_data^=block_p->flip_bit_mask;
    }
    else
    {
        /* increase cursor and reset flip_bit_mask and write_data to flip
         * the highest bit in write_data
         */
        block_p->flip_bit_mask=0x80;
        block_p->write_data=0x7F;
        block_p->cursor++;
    }

    if (block_p->cursor>=block_p->data_block_size)
    {
        UI32_T *seq_num_ar_p=(UI32_T*)NULL, next_write_block_id;
        UI8_T  *valid_block_ar_p=(UI8_T*)NULL;
        SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T *header_p=(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T*)NULL;
        BOOL_T is_error=FALSE, no_valid_block_exist;

        seq_num_ar_p=(UI32_T*)malloc(sizeof(UI32_T)*(block_p->data_block_total_num));
        if(seq_num_ar_p==NULL)
        {
            printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, sizeof(UI32_T)*(block_p->data_block_total_num));
            is_error=TRUE;
            goto exit_1;
        }

        valid_block_ar_p=(UI8_T*)malloc(sizeof(UI8_T)*(block_p->data_block_total_num));
        if(valid_block_ar_p==NULL)
        {
            printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, sizeof(UI8_T)*(block_p->data_block_total_num));
            is_error=TRUE;
            goto exit_1;
        }

        header_p=(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T*)malloc(sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T));
        if(header_p==NULL)
        {
            printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, (UI32_T)(sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T)));
            is_error=TRUE;
            goto exit_1;
        }
        
        if (SYS_TIME_AnalyzeSysTimeDataStorageDataStatus(block_p, seq_num_ar_p, valid_block_ar_p, &no_valid_block_exist)==FALSE)
        {
            printf("%s(%d)SYS_TIME_AnalyzeSysTimeDataStorageDataStatus error\r\n",
                __FUNCTION__, __LINE__);
            is_error=TRUE;
            goto exit_1;
        }

        /* evaluate the new time base according to the data in the control
         * block
         */
        block_p->accumulated_sys_up_time_base +=
            block_p->bump_up_count * SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE;

        if (no_valid_block_exist==FALSE)
        {
            SYS_TIME_GetNextWriteBlockId(block_p, seq_num_ar_p, valid_block_ar_p, &next_write_block_id);
            INIT_ACCUMULATED_CONTROL_BLOCK_FOR_BRAND_NEW_DATA_BLOCK(block_p,
                block_p->accumulated_sys_up_time_base,
                current_sys_up_tick, next_write_block_id, block_p->active_seq_num+1);
        }
        else /* if (no_valid_block_exist==FALSE) */
        {
            /* should not happen, because the first call to this function will
             * invoke SYS_TIME_InitAccumulatedSysUpTimeContrlBlock() and should
             * have at least one valid block
             */
            printf("%s(%d)Unknown data storage error.\r\n", __FUNCTION__, __LINE__);

            /* Init a brand new data block
             */
            INIT_ACCUMULATED_CONTROL_BLOCK_FOR_BRAND_NEW_DATA_BLOCK(block_p,
                block_p->accumulated_sys_up_time_base,
                current_sys_up_tick,
                0,
                0);
        } /* end of if (no_valid_block_exist==FALSE) */
        SYS_TIME_InitBrandNewBlockHeader(block_p->active_seq_num, block_p->accumulated_sys_up_time_base, header_p);

        /* perform low level erase upon the data block
         */
        if (FS_NonVolatileDataStorage_LowLevelErase(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block_p->active_data_block_id, 1)==FALSE)
        {
            printf("%s(%d)FS_NonVolatileDataStorage_LowLevelErase for block id %lu failed\r\n", __FUNCTION__, __LINE__, block_p->active_data_block_id);
            is_error=TRUE;
            goto exit_1;
        }

        /* write header to the beginning of the data block
         */
        if (FS_NonVolatileDataStorage_LowLevelWrite(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block_p->active_data_block_id,
            0, sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T), (UI8_T*) header_p)==FALSE)
        {
            printf("%s(%d)FS_NonVolatileDataStorage_LowLevelWrite for block id %lu failed\r\n", __FUNCTION__, __LINE__, block_p->active_data_block_id);
            is_error=TRUE;
            goto exit_1;
        }

exit_1:
        if (seq_num_ar_p)
            free(seq_num_ar_p);
        if (valid_block_ar_p)
            free(valid_block_ar_p);
        if (header_p)
            free(header_p);

        if (is_error==TRUE)
            goto exit_2;
    }

    elapsed_time_in_tick-=SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE*60*SYS_BLD_TICKS_PER_SECOND;
    if ((I32_T)elapsed_time_in_tick>=SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE*60*SYS_BLD_TICKS_PER_SECOND)
    {
        goto do_again;
    }

exit_2:
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetNextWriteBlockId
 * ------------------------------------------------------------------------
 * FUNCTION : This function output the new block id to be used for the next
 *            write operation for accumulated system up time bump up operation.
 * INPUT    : block_p - accumulated system up time control block
 *            seq_num_ar_p -
 *                      The array that contains the sequence number in the header
 *                      of each valid data block. The size of the array must be
 *                      "block_p->data_block_total_num". The element of the
 *                      array is referenced only when the corresponding element
 *                      in valid_block_ar_p is not 0.
 *            valid_block_ar_p -
 *                      The array that contains the valid status of each data block.
 *                      The size of the array must be "block_p->data_block_total_num".
 *                      The data block is valid when the corresponding element
 *                      value is not 0.
 * OUTPUT   : next_write_block_id_p -
 *                      The new block id to be used for the next write operation
 * RETURN   : None.
 * NOTE     : 1. Caller must hold semaphore before calling this function
 *            2. The input arguments "seq_num_ar_p" and "valid_block_ar_p" can
 *               be generated through the function
 *               SYS_TIME_AnalyzeSysTimeDataStorageDataStatus().
 * ------------------------------------------------------------------------
 */
static void SYS_TIME_GetNextWriteBlockId(const SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p, const UI32_T *seq_num_ar_p, const UI8_T *valid_block_ar_p, UI32_T *next_write_block_id_p)
{
    UI32_T block_id, total_valid_block_num;
    UI32_T oldest_block_id=0, oldest_seq_num=0, oldest_block_id_wrap=0, oldest_seq_num_wrap=0;
    BOOL_T is_max_seq_num_exist=FALSE;

    for (block_id=0, total_valid_block_num=0; block_id<block_p->data_block_total_num; block_id++)
    {
        if (valid_block_ar_p[block_id]==0)
        {
            /* found an unused data block
             */
            *next_write_block_id_p=block_id;
            return;
        }

        total_valid_block_num++;

        if (seq_num_ar_p[block_id]==MAX_SEQ_NUM)
        {
            is_max_seq_num_exist=TRUE;
        }

        if (total_valid_block_num==1)
        {
            oldest_seq_num=oldest_seq_num_wrap=seq_num_ar_p[block_id];
            oldest_block_id=oldest_block_id_wrap=block_id;
            continue;
        }

        if (oldest_seq_num > seq_num_ar_p[block_id])
        {
            oldest_seq_num=seq_num_ar_p[block_id];
            oldest_block_id=block_id;
        }

        if ((I32_T)oldest_seq_num > (I32_T)(seq_num_ar_p[block_id]))
        {
            oldest_seq_num_wrap=seq_num_ar_p[block_id];
            oldest_block_id_wrap=block_id;
        }
    }

    /* no unused data block, overwrite the oldest data block
     */
    if (is_max_seq_num_exist==TRUE)
    {
        *next_write_block_id_p=oldest_block_id_wrap;
    }
    else
    {
        *next_write_block_id_p=oldest_block_id;
    }
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetNewestBlockId
 * ------------------------------------------------------------------------
 * FUNCTION : This function output the newest block id from the given sequence
 *            number.
 * INPUT    : block_p - accumulated system up time control block
 *            seq_num_ar_p -
 *                      The array that contains the sequence number in the header
 *                      of each valid data block. The size of the array must be
 *                      "block_p->data_block_total_num". The element of the
 *                      array is referenced only when the corresponding element
 *                      in valid_block_ar_p is not 0.
 *            valid_block_ar_p -
 *                      The array that contains the valid status of each data block.
 *                      The size of the array must be "block_p->data_block_total_num".
 *                      The data block is valid when the corresponding element
 *                      value is not 0.
 * OUTPUT   : newest_block_id_p -
 *                      The block id of the newest block from the given sequence
 *                      number.
 * RETURN   : TRUE  - The block id of the newest block is found and output.
 *            FALSE - No valid block is found.
 * NOTE     : 1. Caller must hold semaphore before calling this function
 *            2. The input arguments "seq_num_ar_p" and "valid_block_ar_p" can
 *               be generated through the function
 *               SYS_TIME_AnalyzeSysTimeDataStorageDataStatus().
 * ------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_GetNewestBlockId(const SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p, UI32_T *seq_num_ar_p, UI8_T *valid_block_ar_p, UI32_T *newest_block_id_p)
{
    UI32_T block_id, total_valid_block_num;
    UI32_T newest_block_id=0, newest_seq_num, newest_block_id_wrap=0, newest_seq_num_wrap;
    BOOL_T is_max_seq_num_exist=FALSE;

    for (block_id=0, total_valid_block_num=0; block_id<block_p->data_block_total_num; block_id++)
    {
        if (valid_block_ar_p[block_id]==0)
        {
            continue;
        }

        total_valid_block_num++;

        if (seq_num_ar_p[block_id]==MAX_SEQ_NUM)
        {
            is_max_seq_num_exist=TRUE;
        }

        if (total_valid_block_num==1)
        {
            newest_seq_num=newest_seq_num_wrap=seq_num_ar_p[block_id];
            newest_block_id=newest_block_id_wrap=block_id;
            continue;
        }

        if (newest_seq_num < seq_num_ar_p[block_id])
        {
            newest_seq_num=seq_num_ar_p[block_id];
            newest_block_id=block_id;
        }

        if ((I32_T)newest_seq_num < (I32_T)(seq_num_ar_p[block_id]))
        {
            newest_seq_num_wrap=seq_num_ar_p[block_id];
            newest_block_id_wrap=block_id;
        }
    }

    if (total_valid_block_num==0)
    {
        /* no valid data block is found
         */
        return FALSE;
    }

    if (is_max_seq_num_exist==TRUE)
    {
        *newest_block_id_p=newest_block_id_wrap;
    }
    else
    {
        *newest_block_id_p=newest_block_id;
    }
    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_AnalyzeSysTimeDataStorageDataStatus
 * ------------------------------------------------------------------------
 * FUNCTION : Analyze the data in the SYS_TIME data storage to know the
 *            status of the data blocks.
 * INPUT    : block_p - accumulated system up time control block
 * OUTPUT   : seq_num_ar_p -
 *                      The array to output the sequence number in the header
 *                      of each valid data block. The size of the array must be
 *                      "block_p->data_block_total_num". Note that the element
 *                      of the array is valid only when the corresponding element
 *                      in valid_block_ar_p is not 0.
 *            valid_block_ar_p -
 *                      The array to output the valid status of each data block.
 *                      The size of the array must be "block_p->data_block_total_num".
 *                      The data block is valid when the corresponding element
 *                      value is not 0.
 *            no_valid_block_exist_p -
 *                      TRUE if all of the data block is invalid.
 * RETURN   : None.
 * NOTE     : 1. Caller should hold the sempahore before calling this function
 * ------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_AnalyzeSysTimeDataStorageDataStatus(const SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p, UI32_T *seq_num_ar_p, UI8_T  *valid_block_ar_p, BOOL_T *no_valid_block_exist_p)
{
    UI8_T *read_data_buf_p=(UI8_T*)NULL;
    UI32_T checksum_in_header, checksum;
    SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T *header_p=(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T*)NULL;
    UI8_T block;
    BOOL_T ret=TRUE;

    read_data_buf_p=(UI8_T*)malloc(block_p->data_block_size);
    if(read_data_buf_p==NULL)
    {
        printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, block_p->data_block_size);
        ret=FALSE;
        goto exit;
    }
    header_p=(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T*)read_data_buf_p;

    /* init output arguments
     */
    memset(seq_num_ar_p, 0, sizeof(UI32_T)*block_p->data_block_total_num);
    memset(valid_block_ar_p, 0, sizeof(UI8_T)*block_p->data_block_total_num);
    *no_valid_block_exist_p=TRUE;

    /* analyze the existing data in the data storage
     */
    for (block=0; block<block_p->data_block_total_num; block++)
    {
        if(FS_NonVolatileDataStorage_LowLevelRead(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block, read_data_buf_p)==FALSE)
        {
            printf("%s(%d)FS_NonVolatileDataStorage_LowLevelRead failed(block %hu)\r\n", __FUNCTION__, __LINE__, block);
            ret=FALSE;
            goto exit;
        }

        if (header_p->magic!=SYS_TIME_ACCUMULATED_SYS_UP_TIME_INFO_BLOCK_HEADER_MAGIC)
        {
            continue;
        }

        checksum_in_header=header_p->checksum;
        header_p->checksum=0; /* set header_p->checksum as 0 to evalute the checksum of the data in header */
        checksum=L_MATH_CheckSum((void*)(header_p), sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T));
        if (checksum!=checksum_in_header)
        {
            /* Illegal checksum
             */
#if 0
            printf("%s(%d)Illegal checksum in header.(checksum=0x%08lX, expected checksum=0x%08lX\r\n",
                __FUNCTION__, __LINE__, checksum_in_header, checksum);
#endif
            continue;
        }
        valid_block_ar_p[block]=1;
        seq_num_ar_p[block]=header_p->seq_num;
        *no_valid_block_exist_p=FALSE;
    }

exit:
    if (read_data_buf_p)
        free(read_data_buf_p);

    return ret;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_InitAccumulatedSysUpTimeContrlBlock
 * ------------------------------------------------------------------------
 * FUNCTION : Initialize the fields of the control block for accumulated
 *            system up time.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : This function should be called by SYSRSC_MGR.
 * ------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_InitAccumulatedSysUpTimeContrlBlock(void)
{
    UI32_T sys_up_time_in_tick;
    UI32_T *seq_num_ar_p=(UI32_T*)NULL, storage_total_size;
    UI8_T  *valid_block_ar_p=(UI8_T*)NULL;
    UI8_T  *read_data_buf_p=(UI8_T*)NULL;
    SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T *header_p;
    SYS_TIME_AccumulatedSysUpTimeControlBlock_T *block_p;
    BOOL_T ret=TRUE, no_valid_block_exist=TRUE;

    block_p=SYS_TIME_GetAccumulatedSysUpCtlBlkPtr();
    storage_total_size=FS_NonVolatileDataStorage_GetSize(FS_TYPE_DATA_STORAGE_ID_SYSTIME);
    if (storage_total_size==0)
    {
        printf("%s(%d)FS_NonVolatileDataStorage_GetSize error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (FS_NonVolatileDataStorage_LowLevelGetBlockSize(FS_TYPE_DATA_STORAGE_ID_SYSTIME, &(block_p->data_block_size))==FALSE)
    {
        printf("%s(%d)FS_NonVolatileDataStorage_LowLevelGetBlockSize error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    block_p->data_block_total_num=storage_total_size/(block_p->data_block_size);

    if (block_p->data_block_total_num==0)
    {
        printf("%s(%d)Invalid data block total num\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    seq_num_ar_p=(UI32_T*)malloc(sizeof(UI32_T)*(block_p->data_block_total_num));
    if(seq_num_ar_p==NULL)
    {
        printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, sizeof(UI32_T)*(block_p->data_block_total_num));
        return FALSE;
    }

    valid_block_ar_p=(UI8_T*)malloc(sizeof(UI8_T)*(block_p->data_block_total_num));
    if(valid_block_ar_p==NULL)
    {
        printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, sizeof(UI8_T)*(block_p->data_block_total_num));
        ret=FALSE;
        goto exit;
    }

    read_data_buf_p=(UI8_T*)malloc(block_p->data_block_size);
    if(read_data_buf_p==NULL)
    {
        printf("%s(%d)malloc %lu bytes failed\r\n", __FUNCTION__, __LINE__, block_p->data_block_size);
        ret=FALSE;
        goto exit;
    }
    header_p=(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T*)read_data_buf_p;

    if (SYS_TIME_AnalyzeSysTimeDataStorageDataStatus(block_p, seq_num_ar_p, valid_block_ar_p, &no_valid_block_exist)==FALSE)
    {
        printf("%s(%d)SYS_TIME_AnalyzeSysTimeDataStorageDataStatus error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto exit;
    }

    if (no_valid_block_exist==TRUE)
    {
        SYS_TIME_GetSystemUpTimeByTick_Internal(FALSE, &sys_up_time_in_tick);
        SYS_TIME_InitBrandNewBlockHeader(0, (UI64_T)((sys_up_time_in_tick)/(SYS_BLD_TICKS_PER_SECOND*60)), header_p);
        INIT_ACCUMULATED_CONTROL_BLOCK_FOR_BRAND_NEW_DATA_BLOCK(block_p, header_p->accumulated_sys_up_time_base, sys_up_time_in_tick, 0, 0);

        /* perform low level erase upon the data block
         */
        if (FS_NonVolatileDataStorage_LowLevelErase(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block_p->active_data_block_id, 1)==FALSE)
        {
            printf("%s(%d)FS_NonVolatileDataStorage_LowLevelErase for block id %lu failed\r\n", __FUNCTION__, __LINE__, block_p->active_data_block_id);
            ret=FALSE;
            goto exit;
        }

        /* write header to the beginning of the data block
         */
        if (FS_NonVolatileDataStorage_LowLevelWrite(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block_p->active_data_block_id,
            0, sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T), (UI8_T*) header_p)==FALSE)
        {
            printf("%s(%d)FS_NonVolatileDataStorage_LowLevelWrite for block id %lu failed\r\n", __FUNCTION__, __LINE__, block_p->active_data_block_id);
            ret=FALSE;
            goto exit;
        }
    }
    else
    {
        UI32_T newest_block_id=0, next_write_block_id=0, ui32_offset, bump_up_count, num_of_bit;
        UI32_T *ui32_ptr, non_aligned_sz;
        UI8_T bit_pos_list[32];
        BOOL_T found_non_zero_data;

        /* locate the active seq_num and its corresponding block id
         */
        if (SYS_TIME_GetNewestBlockId(block_p, seq_num_ar_p, valid_block_ar_p, &newest_block_id)==FALSE)
        {
            /* should not happen
             */
            printf("%s(%d)SYS_TIME_GetNewestBlockId failed\r\n", __FUNCTION__, __LINE__);
            ret=FALSE;
            goto exit;
        }

        if(FS_NonVolatileDataStorage_LowLevelRead(FS_TYPE_DATA_STORAGE_ID_SYSTIME, newest_block_id, read_data_buf_p)==FALSE)
        {
            printf("%s(%d)FS_NonVolatileDataStorage_LowLevelRead failed(block %lu)\r\n", __FUNCTION__, __LINE__, newest_block_id);
            ret=FALSE;
            goto exit;
        }
        block_p->accumulated_sys_up_time_base = header_p->accumulated_sys_up_time_base;
        block_p->active_data_block_id = newest_block_id;
        block_p->active_seq_num = seq_num_ar_p[newest_block_id];

        /* count the bump up bit by checking the number of zero bit
         * handle the data that is not aligned to UI32 natural boundary
         */
        bump_up_count=0;
        ui32_offset=L_ALIGN(sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T), 4);
        non_aligned_sz=ui32_offset-sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T);
        if(non_aligned_sz!=0)
        {
            UI32_T tmp=0xFFFFFFFFUL;

            memcpy(&tmp, read_data_buf_p+sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T),
                non_aligned_sz);
            num_of_bit=L_BITMAP_Get_BitPos_List(tmp, bit_pos_list);
            bump_up_count+=(32-num_of_bit);
        }

        /* count the bump up bit by checking the number of zero bit
         * for data that is aligned to UI32_T natural boundary
         */
        for(ui32_ptr=(UI32_T*)(read_data_buf_p+ui32_offset), found_non_zero_data=FALSE;
            ui32_offset<block_p->data_block_size;
            ui32_offset+=4, ui32_ptr++)
        {
            if (*ui32_ptr!=0)
            {
                found_non_zero_data=TRUE;
                num_of_bit=L_BITMAP_Get_BitPos_List(*ui32_ptr, bit_pos_list);
                bump_up_count+=(32-num_of_bit);
                break;
            }
            bump_up_count+=32;
        }
        block_p->bump_up_count=bump_up_count;

        /* initialize cursor, write_data and mask in the control block according
         * to bump_up_count
         */
        if (found_non_zero_data==FALSE) /* No more bit available for bump up mark */
        {
            SYS_TIME_GetSystemUpTimeByTick_Internal(FALSE, &sys_up_time_in_tick);

            SYS_TIME_GetNextWriteBlockId(block_p, seq_num_ar_p, valid_block_ar_p, &next_write_block_id);
            INIT_ACCUMULATED_CONTROL_BLOCK_FOR_BRAND_NEW_DATA_BLOCK(block_p,
                block_p->accumulated_sys_up_time_base + ((block_p->data_block_size-sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T))*8*SYS_TIME_ACCUMULATED_BUMP_UP_TIME_UNIT_IN_MINUTE),
                sys_up_time_in_tick,
                next_write_block_id,
                block_p->active_seq_num+1);
            SYS_TIME_InitBrandNewBlockHeader(block_p->active_seq_num, block_p->accumulated_sys_up_time_base, header_p);

            /* perform low level erase upon the data block
             */
            if (FS_NonVolatileDataStorage_LowLevelErase(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block_p->active_data_block_id, 1)==FALSE)
            {
                printf("%s(%d)FS_NonVolatileDataStorage_LowLevelErase failed(block %lu)\r\n", __FUNCTION__, __LINE__, block_p->active_data_block_id);
                ret=FALSE;
                goto exit;
            }

            /* write header to the beginning of the data block
             */
            if (FS_NonVolatileDataStorage_LowLevelWrite(FS_TYPE_DATA_STORAGE_ID_SYSTIME, block_p->active_data_block_id,
                0, sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T), (UI8_T*) header_p)==FALSE)
            {
                printf("%s(%d)FS_NonVolatileDataStorage_LowLevelWrite for block id %lu failed\r\n", __FUNCTION__, __LINE__, block_p->active_data_block_id);
                ret=FALSE;
                goto exit;
            }

        }
        else
        {
            UI8_T bump_up_bit_num_in_one_byte;

            block_p->cursor=sizeof(SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T) +
                bump_up_count/8;

            bump_up_bit_num_in_one_byte=bump_up_count % 8;

            switch(bump_up_bit_num_in_one_byte)
            {
                case 0:
                    block_p->flip_bit_mask=0x80;
                    block_p->write_data=0x7F;
                    break;
                case 1:
                    block_p->flip_bit_mask=0x40;
                    block_p->write_data=0x3F;
                    break;
                case 2:
                    block_p->flip_bit_mask=0x20;
                    block_p->write_data=0x1F;
                    break;
                case 3:
                    block_p->flip_bit_mask=0x10;
                    block_p->write_data=0x0F;
                    break;
                case 4:
                    block_p->flip_bit_mask=0x08;
                    block_p->write_data=0x07;
                    break;
                case 5:
                    block_p->flip_bit_mask=0x04;
                    block_p->write_data=0x03;
                    break;
                case 6:
                    block_p->flip_bit_mask=0x02;
                    block_p->write_data=0x01;
                    break;
                case 7:
                    block_p->flip_bit_mask=0x01;
                    block_p->write_data=0x00;
                    break;

                default:
                    /* should not happen */
                    break;
            }                
        }
    }

exit:
    if (seq_num_ar_p)
        free(seq_num_ar_p);
    if (valid_block_ar_p)
        free(valid_block_ar_p);
    if (read_data_buf_p)
        free(read_data_buf_p);

    return ret;
}

#endif /* end of #if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE) */

