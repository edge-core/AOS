/* Module Name: L_MM.C(Kernel space)
 * Purpose:
 *  MM (Memory Management/Memory Monitor) provides memory allocation/free functions
 *    with Monitor Information included at the head of each allocated buffer.
 *    Therefore the usage of the allocated buffer can be easily traced.
 *  MM also provides MREF (Memory Reference/Multiple Reference) for a single data block to be
 *    multiplely accessed by tasks without replicating or reproducing the data block.
 *    Therefore the overhead of memory allocation/free/copy can be reduced to enhance the whole
 *    system's performance.
 *
 * Notes:
 *   The allocated buffer must have the following format with the Mornitor Information at the head of
 *   The allocated buffer by K_L_MM_PtAlloc() is consisted of :
 *      buffer :=  K_L_MM_Monitor_T + Application's Data Block + trail Magic bytes(L_MM_TRAIL_MAGIC_BYTE_LEN bytes)
 *   The allocated buffer for K_L_MM_Mref_Construct() is consisted of:
 *      buffer :=  K_L_MM_Monitor_T + K_L_MM_Mref_T + Application's Data Block + trail Magic bytes(L_MM_TRAIL_MAGIC_BYTE_LEN bytes)
 *   L_MM depends on L_IPCMEM on linux, so it is necessary to call L_IPCMEM_Init() for
 *   L_MM to work properly.
 *
 *   It is found that an exception ever occurred on allocation of a MREF descriptor
 *   due to the corruption of MREF buffer pool. The TX buffer, Rx buffer and
 *   MREF buffer pool are all located in high memory region. Tx buffer is in the
 *   lowest address region, Rx buffer is next to Tx buffer region, and MREF
 *   buffer is in the highest address region. It is suspected that some code
 *   might get the pointer to Tx buffer or Rx buffer region but modify the
 *   content of area in MREF buffer region due to buffer overflow defect.
 *   In order to locate the suspected defect code easily, the content of MREF
 *   buffer is checked whenever K_L_MM_Mref_GetPdu() is called. The caller
 *   address and the caller task id will be kept when K_L_MM_Mref_GetPdu() is
 *   called. When MREF buffer error is detected, it will pass the caller address
 *   and the caller task id that called K_L_MM_Mref_GetPdu() previously to user
 *   space and log the information to flash. The MREF error check will impact
 *   the system performance, thus a debug flag is created to turn on or turn off
 *   the MREF error detection. The default value for the flag is off. It can be
 *   toggled by K_MM backdoor.
 *
 * History:
 *    2005/6       -- Ryan, Create
 *    2006/1/17    -- Charlie, add a API K_L_MM_AllocateTxBuffer() for buffer
 *                             allocation of packet transmission
 *    2006/10/13   -- Charlie, port to linux
 *    2008/7/3     -- Tiger Liu, remove unused function, rename functions L_MM_XXX to K_L_MM_XXX.
 *    2016/10/31   -- Charlie, add mref error detection when calling
 *                             K_L_MM_Mref_GetPdu()
 *
 * Copyright(C)      Accton Corporation, 2004-2016
 */

/* INCLUDE FILE DECLARATIONS
 */
/* linux specific include files */
#include <linux/string.h> /* for memset() memmove() */
#include <linux/kernel.h> /* for printk() */
#include <linux/slab.h>   /* for kmalloc() and kfree() */
#include <linux/spinlock.h>
#include <linux/sched.h>

#include <linux/version.h>
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(3,2,65) )
    #include <linux/uaccess.h>
#else
    #include <asm/uaccess.h>  /* for put_user() */
#endif

/* Accton include files */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_module.h"
#include "cmnlib_type.h"
#include "l_math.h"
#include "k_sysfun.h"
#include "k_l_ipcmem.h"
#include "k_l_mm.h"
#include "l_cvrt.h"
#include "l_ipcmem_type.h" // temporarily
#include "../../common/include/l_pbmp.h"

#define BACKDOOR_OPEN TRUE
#undef  TEST_BUFFER_CORRUPT

#define MAGIC_PATTERN_1B 0x55
#define MAGIC_PATTERN_4B (UI32_T)(MAGIC_PATTERN_1B | \
                          MAGIC_PATTERN_1B<<8      | \
                          MAGIC_PATTERN_1B<<16     | \
                          MAGIC_PATTERN_1B<<24)

/* NAMING CONSTANT DECLARACTION
 */

#define L_MM_BUFFER_TYPE_MREF_POOL     0

/* MACRO FUNCTION DECLARACTION
 */
#define L_MM_DEBUG_MSG(msg)                         \
    do {                                            \
        if(l_mm_backdoor_dbg_msg_flag==TRUE)        \
            printk("\r\n<6>%s():"msg, __FUNCTION__);\
    } while (0)

#define L_MM_ERROR_MSG(msg)                     \
    do {                                        \
        printk("\r\n<0>%s(%d):"msg, __FUNCTION__, __LINE__);\
    } while (0)

#define L_MM_ERROR_MSG_WITH_ARG(msg, ...) \
    do {                                       \
        printk("\r\n<0>%s(%d):"msg, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
    } while (0)

/* L_MM_GET_ONE_PT_REAL_SIZE
 * PURPOSE:
 *    While using L_MM_PT, users need to reserve extra space in addition to its own application.
 *    This macro is for ease of calculating REAL required size of each partition.
 *    This macro can help calculating the REAL size required by L_MM_Pt
 *    by giving each desired partition size.
 * INPUT:
 *    usr_partition_size -- The size of partition specified by user
 * RETURN: The real ocupied size for each partition of L_MM_Pt.
 *
 * NOTE:
 *    adjust partition length, must be a multiple of 4
 */
#define L_MM_GET_ONE_PT_REAL_SIZE(usr_partition_size) ((((usr_partition_size) + sizeof(K_L_MM_Monitor_T) + L_MM_TRAIL_MAGIC_BYTE_LEN + 3) / 4 ) * 4 )

#define L_MM_GET_PT_TOTAL_REQUIRED_SZ(usr_partition_size, partition_num) L_MM_GET_ONE_PT_REAL_SIZE(usr_partition_size) * partition_num

#define L_MM_GET_MONITOR_PART_FROM_MREF(mref_p) ((K_L_MM_Monitor_T*)mref_p-1)

#define L_MM_ENTER_CRITICAL_SECTION SYSFUN_EnterCriticalSection
#define L_MM_LEAVE_CRITICAL_SECTION SYSFUN_LeaveCriticalSection

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T K_L_MM_Operation(void* cmd, void* arg1, void* arg2, void* arg3, void* arg4);
static void   K_L_MM_InsertTo_MonitorList(K_L_MM_Monitor_T *mm_p);
static void   K_L_MM_RemoveFrom_MonitorList(K_L_MM_Monitor_T *mm_p);
static BOOL_T K_L_MM_Valid_Buf(void *buff_p);
static L_MM_Mref_Handle_T*  K_L_MM_Mref_Construct (void*               buf,
                                          UI16_T              buf_len,
                                          UI16_T              pdu_offset,
                                          UI16_T              pdu_len,
                                          UI16_T              type,
                                          void                *cookie,
                                          void                *cookie_params);

#if (BACKDOOR_OPEN==TRUE)
static BOOL_T K_L_MM_GetBufferInfo(L_MM_Backdoor_BufferInfo_T *buffer_info_p);
static BOOL_T K_L_MM_Mref_Corrupt(void);
static BOOL_T K_L_MM_GetMrefInfo(L_MM_Backdoor_MrefInfo_T *mref_info_p);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static K_L_MM_Monitor_T* l_mm_monitor_lst_head =  NULL;  /* for monitor list */

/* static variables for debug/backdoor
 */
static BOOL_T l_mm_backdoor_dbg_msg_flag = FALSE;        /* l_mm debug message flag */
static UI32_T l_mm_backdoor_buff_nbr = 0;                /* record total number of the allocated buffers */

#if (BACKDOOR_OPEN==TRUE)

static BOOL_T l_mm_backdoor_validate_free_flag = FALSE;  /* This flag can be changed by backdoor.
                                                          * If this flag is on, when L_MM_Free() is called,
                                                          * it will check that whether is any buffer in moniter
                                                          * list falls in the range of the buffer to be freed.
                                                          * An error message will be shown if this kind of buffer
                                                          * do exists. This error will occur if user of L_MM_Pt
                                                          * free the buffer block which is used to partition.
                                                          * In order to protect l_mm_monitor_lst_head from corruption,
                                                          * we will not free the specified buffer.
                                                          */

#endif

static L_MM_PtDesc_T mref_pool_desc;
static BOOL_T l_mm_backdoor_mref_error_detect_flag = FALSE; /* for MREF buffer pool error detection
                                                             * Will do MREF buffer pool error check when
                                                             * l_mm_backdoor_mref_error_detect_flag is TRUE.
                                                             */
static UI32_T total_number_of_mref_desc=0; /* for MREF error detection, used in K_L_MM_CheckMrefPool */
static UI8_T  *mref_pool_desc_bmp=NULL; /* for MREF error detection, used in K_L_MM_CheckMrefPool */
static uintptr_t previous_caller_addr=0; /* for MREF error detection to keep previous caller address */
static UI32_T previous_caller_tid=0;  /* for MREF error detection to keep previous caller tid */

static DEFINE_SPINLOCK(pt_desc_lock);
static DEFINE_SPINLOCK(mref_lock);
static DEFINE_SPINLOCK(mm_monitor_list_lock);

/* EXPORTED FUNCTION DECLARACTION
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_IsMrefPoolInit
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Check whether MREF descriptor buffer pool had been initialized.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -  MREF buffer pool had been initialized.
 *    FALSE -  MREF buffer pool had not been initialized correctly.
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static inline BOOL_T K_L_MM_IsMrefPoolInit(void)
{
    BOOL_T ret=TRUE;

    if(total_number_of_mref_desc==0)
    {
        L_MM_ERROR_MSG("total_number_of_mref_desc is 0.\r\n");
        ret=FALSE;
    }

    if(mref_pool_desc_bmp==NULL)
    {
        L_MM_ERROR_MSG("mref_pool_desc_bmp is 0.\r\n");

        ret=FALSE;
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_CheckMrefPool
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Check whether any error in MREF descriptor buffer pool.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -  No error is detected.
 *    FALSE -  At least one error is detected in MREF descriptor buffer pool.
 * NOTES:
 *    The caller shall use "pt_desc_lock" before calling this function to
 *    protect the critical section.
 *------------------------------------------------------------------------------
 */
static BOOL_T K_L_MM_CheckMrefPool(void)
{
    UI32_T real_part_size, i;
    K_L_MM_Monitor_T *ptr;
    UI8_T            *ptr_ui8;
    BOOL_T ret_val=TRUE, is_valid_magic_bytes;

    /* sanity check */
    if (K_L_MM_IsMrefPoolInit()==FALSE)
    {
        L_MM_ERROR_MSG("K_L_MM_IsMrefPoolInit() returns FALSE.\r\n");
        return FALSE;
    }

    real_part_size = L_MM_GET_ONE_PT_REAL_SIZE(sizeof(K_L_MM_Mref_T));

    /* clear all bits in mref_pool_desc_bmp */
    memset(mref_pool_desc_bmp, 0, (total_number_of_mref_desc+7)/8);

    /* check buffers in free list */
    for(ptr = mref_pool_desc.free_partition_list; ptr!=NULL; ptr = (K_L_MM_Monitor_T*)(ptr->next_p))
    {
        /* check ptr range */
        if (L_CVRT_PTR_TO_UINT(ptr) < L_CVRT_PTR_TO_UINT(mref_pool_desc.buffer) ||
            L_CVRT_PTR_TO_UINT(ptr) > L_CVRT_PTR_TO_UINT(mref_pool_desc.buffer + mref_pool_desc.buffer_len))
        {
            L_MM_ERROR_MSG_WITH_ARG("ptr=%p out of valid range.\r\n", ptr);
            ret_val=FALSE;
            goto fail_exit;
        }

        /* check ptr alignment */
        if ( ((L_CVRT_PTR_TO_UINT(ptr) - L_CVRT_PTR_TO_UINT(mref_pool_desc.buffer)) % real_part_size) != 0 )
        {
            L_MM_ERROR_MSG_WITH_ARG("ptr=%p alignment error.\r\n", ptr);
            ret_val=FALSE;
        }
        else
        {
            /* use L_PBMP macro to set the bitmap in mref_pool_desc_bmp to
             * mark the free buffers
             */
            L_PBMP_SET_PORT_IN_PBMP_ARRAY(mref_pool_desc_bmp, ((L_CVRT_PTR_TO_UINT(ptr) - L_CVRT_PTR_TO_UINT(mref_pool_desc.buffer)) / real_part_size)+1); /* port in L_PBMP Macro is 1 based */
        }

        /* check buffer_addr */
        if (ptr->buffer_addr!=NULL)
        {
            L_MM_ERROR_MSG_WITH_ARG("ptr=%p buffer_addr not NULL(%p)\r\n", ptr, ptr->buffer_addr);
            ret_val=FALSE;
        }

        /* check leading magic byte */
        L_MM_IS_VALID_LEADING_MAIC_BYTES(&is_valid_magic_bytes, ptr);
        if(is_valid_magic_bytes==FALSE)
        {
            L_MM_ERROR_MSG_WITH_ARG("ptr validation fail through leading magic byte check(ptr=%p)\r\n", ptr);
            ret_val=FALSE;
        }

        /* check trail magic byte */
        L_MM_IS_VALID_TRAIL_MAGIC_BYTES(&is_valid_magic_bytes, ptr);
        if (is_valid_magic_bytes==FALSE)
        {
            L_MM_ERROR_MSG_WITH_ARG("ptr validation fail through trail magic byte check(ptr=%p)\r\n", ptr);
            ret_val=FALSE;
        }
    }

fail_exit:
    /* check allocated buffers */
    for (ptr_ui8 = mref_pool_desc.buffer, i=0;
         i<total_number_of_mref_desc;
         ptr_ui8 += real_part_size, i++)
    {
        /* Use L_PBMP macro to check whether the bit had been set. It means the
         * buffer is in the free list if the bit is set. Here only check the
         * buffers that are not in free list.
         */
        if (!(L_PBMP_GET_PORT_IN_PBMP_ARRAY(mref_pool_desc_bmp, i+1)))
        {
            /* Validate the allocated buffer through the function
             * K_L_MM_Valid_Buf.
             */
            if (K_L_MM_Valid_Buf(ptr_ui8 + sizeof(K_L_MM_Monitor_T))==FALSE)
            {
                L_MM_ERROR_MSG_WITH_ARG("validate buffer failed. ptr=%p\r\n",
                    ptr_ui8);
                ret_val=FALSE;
            }
        }
    }

    return ret_val;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize L_MM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void K_L_MM_Init (void)
{
#if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE && L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL == FALSE)
    if (K_L_IPCMEM_GetStartOfMemBufferAddr() == NULL)
        return;
#endif

    if (!K_L_IPCMEM_GetMRefBufStartAddress((void**)(&(mref_pool_desc.buffer))))
    {
        SYSFUN_LogUrgentMsg("K_L_MM_Init: Failed to init mref_pool_desc\r\n");
        return;
    }

    mref_pool_desc.buffer_len = L_MM_GET_PT_TOTAL_REQUIRED_SZ(sizeof(K_L_MM_Mref_T), SYS_ADPT_MAX_NBR_OF_MREF_DESC_BLOCK);
    mref_pool_desc.buffer_type = L_MM_BUFFER_TYPE_MREF_POOL;
    mref_pool_desc.partition_len = sizeof(K_L_MM_Mref_T);

    if(FALSE==K_L_MM_PtCreate(&mref_pool_desc))
    {
        SYSFUN_LogUrgentMsg("K_L_MM_Init: Failed to init mref_pool_desc\r\n");
    }
    else
    {
        if (total_number_of_mref_desc==0)
        {
            total_number_of_mref_desc=mref_pool_desc.nbr_of_free_partition;
            mref_pool_desc_bmp = kmalloc((total_number_of_mref_desc+7)/8, GFP_KERNEL);
            if (mref_pool_desc_bmp==NULL)
            {
                L_MM_ERROR_MSG_WITH_ARG("kmalloc %lu bytes failed!\r\n", (unsigned long)((total_number_of_mref_desc+7)/8));
            }
        }
    }

} /* end of void K_L_MM_Init (void) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - K_L_MM_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void K_L_MM_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_L_MM, K_L_MM_Operation);
} /* end of K_L_MM_Create_InterCSC_Relation */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_GetBufferType
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the user buffer type of the given buffer.
 * INPUT:
 *    buf_p  --  pointer to a buffer allocated by l_mm(L_MM_Malloc() or K_L_MM_PtAlloc())
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    Return User buffer type if sucess.
 *    Return L_MM_ERROR_RETURN_VAL if fail.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T K_L_MM_GetBufferType(void* buf_p)
{
    K_L_MM_Monitor_T *mm_p;

    if(buf_p==NULL)
    {
        L_MM_ERROR_MSG("buf_p is NULL\r\n");
        return L_MM_ERROR_RETURN_VAL;
    }

    /* validate buffer */
    if(K_L_MM_Valid_Buf(buf_p) == FALSE)
    {
        L_MM_ERROR_MSG("invalid buffer\r\n");
        return  L_MM_ERROR_RETURN_VAL;
    }

    mm_p = (K_L_MM_Monitor_T*)buf_p - 1;

    return L_MM_GET_APP_BUFTYPE(mm_p->buffer_type);
} /* end of K_L_MM_GetBufferType */

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_PtGetTotalRequiredSize
 *------------------------------------------------------------------------------
 * PURPOSE:  Get the total required size to create a partition according to
 *           the given one-partition size and the number of partition.
 * INPUT:    partition_size  -  size of a partition
 *           partition_num   -  number of partition
 * OUTPUT:   None.
 * RETURN:   required buffer size to create partitions
 *           return 0 if error
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
UI32_T K_L_MM_PtGetTotalRequiredSize(UI32_T partition_size, UI32_T partition_num)
{
    if((partition_size==0) || (partition_num==0))
        return 0;

    return L_MM_GET_PT_TOTAL_REQUIRED_SZ(partition_size, partition_num);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_PtCreate
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Create a memory partition
 *
 * INPUT:
 *  desc -
 *     buffer        -- pointer to the memory area
 *     buffer_len    -- size of the memory area (in bytes, must be got from
 *                                               K_L_MM_PtGetTotalRequiredSize)
 *     partition_len -- size of each partition
 *     buffer_type   -- type of buffer (to indicate which kinds of buffers are
 *                                      used, valid value is from 0 to 63)
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  FALSE -- can't create the partition successfully.
 *  TRUE --  the partition is created successfully
 *
 * NOTES:
 *  1. The buffer will be divided into partitions,
 *      partition_len <= buffer_len
 *  2. partition_len means the length of partition which is transparent to user
 *     Real occupied partition size can be got from L_MM_GET_PT_REAL_PT_SZ.
 *  3. buffer_type will be stored in K_L_MM_Monitor_T field
 *  4. each partition is consisted of:
 *     [K_L_MM_Monitor_T] + [user_partition_buff] + [L_MM_TRAIL_MAGIC_BYTES]
 *     only user_partition_buff is transparent to user
 *  4. leading and trailing magic bytes are initialized for each partition
 *     when this api is called.
 *  5. The two MSB bits of buffer_type is reserved for internal use.
 *     Application can only use the other 6 bits.
 *  6. K_L_MM_Monitor_T.buffer_addr will only be initialized when the buffer is
 *     allocated.
 *  7. Partition is not allwed to be destroyed once it is created.
 *     In other words, the argument desc.buffer should never be freed.
 *------------------------------------------------------------------------------
 */
BOOL_T K_L_MM_PtCreate (L_MM_PtDesc_T *desc)
{
    UI32_T  i, buf_nbr;
    UI8_T   *next_position_p;
    UI32_T  real_part_len;
    K_L_MM_Monitor_T *mm_p = NULL;

    /* BODY */

    /* validate buffer_type */
    if((desc==NULL) ||
       (desc->buffer_type > L_MM_MAX_USER_BUFFER_TYPE))
    {
        L_MM_ERROR_MSG("invalid buffer type\r\n");
        return FALSE;
    }

    if(desc->partition_len > desc->buffer_len)
    {
        L_MM_ERROR_MSG_WITH_ARG("invalid partition_len or buffer_len setting. partition_len=%d, buffer_len=%d\r\n",
                   (int)desc->partition_len,
                   (int)desc->buffer_len);
        return FALSE;
    }

    real_part_len = L_MM_GET_ONE_PT_REAL_SIZE(desc->partition_len);
    desc->free_partition_list = 0;

    buf_nbr = desc->buffer_len / real_part_len;

    next_position_p = (UI8_T*) (desc->free_partition_list = desc->buffer);

    for (i = 0; i < buf_nbr; i++)
    {
        mm_p = (K_L_MM_Monitor_T*) next_position_p;
        next_position_p += real_part_len;
        mm_p->buffer_type = L_MM_CONVERT_TO_INTERNAL_BUFTYPE(L_MM_PT_BUFFER_TYPE) | desc->buffer_type; /* store the buffer type in Monitor field */
        mm_p->buffer_addr = NULL; /* this field is only filled when the buffer is allocated */
        mm_p->next_p = (K_L_MM_Monitor_T*)next_position_p;
        mm_p->buf_len = real_part_len;
        L_MM_FILL_LEADING_MAGIC_BYTE(mm_p);
        L_MM_FILL_TRAIL_MAGIC_BYTE(mm_p);
#ifdef TEST_BUFFER_CORRUPT
        memset(mm_p+1, MAGIC_PATTERN_1B, desc->partition_len);
#endif
    }
    mm_p->next_p = NULL;

    desc->nbr_of_free_partition = buf_nbr;

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_PtAlloc
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Allocate a buffer from the specified partition pool with K_L_MM_Monitor_T
 *   field prefixed to Application's Data Block
 *
 * INPUT:
 *  desc    -- partition descriptor
 *  user_id -- assigned to monitor structure which is prefixed to Application's
 *             Data Block
 *             user_id = trace_id(6) + pool_id(2) + module_id(8)
 *             pool_id is at user's own usage/knowledge.
 *             It can be used to stand for buffer being created by K_L_MM_PtCreate
 *             or L_MM_Malloc or 1st PtCreate and 2nd PtCreate...
 *             or it can be combined with trace_id to form a different trace
 *             information for user's own trace purpose.
 *             user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *             or L_MM_USER_ID2(modul_id,ext_trace_id)
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  address of the allocated data block for Application (pointer to Application's Data Block).
 *  Return null if no more data block is available.
 *
 * NOTES:
 *   1. The allocated buffer has K_L_MM_Monitor_T field at its head but it's NOT
 *      the starting address of K_L_MM_Monitor_T field is returned.
 *   2. Compared to L_MM_Malloc which is for varied-sized buffer allocation,
 *       K_L_MM_PtAlloc is a fixed-sized buffer allocation from a pre-defined partition
 *   3. L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *------------------------------------------------------------------------------
 */
void *K_L_MM_PtAlloc(L_MM_PtDesc_T *desc, UI16_T user_id)
{
    SYSFUN_IntMask_T oint;
    K_L_MM_Monitor_T *mm_p;
#ifdef TEST_BUFFER_CORRUPT
    UI32_T   i, *data_p;
#endif

    if(desc==NULL)
    {
        L_MM_ERROR_MSG("desc is NULL\r\n");
        return NULL;
    }

    L_MM_ENTER_CRITICAL_SECTION(&pt_desc_lock, oint);

    if ((mm_p =  (K_L_MM_Monitor_T*)desc->free_partition_list) != NULL)
    {
        desc->free_partition_list = (void *) mm_p->next_p;
        desc->nbr_of_free_partition --;
        mm_p->buffer_addr = mm_p;
    }

    L_MM_LEAVE_CRITICAL_SECTION(&pt_desc_lock, oint);

    if (mm_p == NULL)
    {
        L_MM_DEBUG_MSG("no more partition can be allocated\r\n");
        return NULL;
    }

    mm_p->task_id = SYSFUN_TaskIdSelf();
    mm_p->allocation_time = SYSFUN_GetSysTick();
    mm_p->uid = user_id;
    mm_p->buffer_type = mm_p->buffer_type;

    /* insert the monitor information to the monitor link list */
    K_L_MM_InsertTo_MonitorList(mm_p);

#ifdef TEST_BUFFER_CORRUPT
    for(data_p=(UI32_T*)(mm_p+1), i=0; i<desc->partition_len/sizeof(UI32_T); i++, data_p++)
    {
        if (*data_p!=(UI32_T)MAGIC_PATTERN_4B)
        {
            printk("<0>%s: desc=0x%p. partition_len=%u, buffer_type: %x, alloc: 0x%p, i: %u , pid: %u, data: %u Buffer corrupt!\n",
                __FUNCTION__, desc, desc->partition_len, mm_p->buffer_type, mm_p->buffer_addr, i, mm_p->task_id, *data_p);
        }
    }
#endif

    return (void*)((UI8_T*)mm_p + sizeof(K_L_MM_Monitor_T));
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_PtFree
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Free the buffer which is allocated by K_L_MM_PtAlloc
 *
 * INPUT:
 *  desc    -- partition descriptor
 *  buffer  -- pointer to Application's Data Block
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  TRUE  - Free sucessfully
 *  FALSE - Fail to free
 *
 * NOTES:
 *   1. This function will free the buffer starts at the input buffer address - sizeof(K_L_MM_Monitor_T)
 *   2. Buffer address and leading and trail magic bytes will be verified.
 *   3. L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *------------------------------------------------------------------------------
 */
BOOL_T K_L_MM_PtFree(L_MM_PtDesc_T *desc, void *buffer)
{
    SYSFUN_IntMask_T oint;
    UI32_T real_part_len;
    K_L_MM_Monitor_T *mm_p;

    if ( (desc==NULL) || (buffer == NULL) )
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
       L_MM_ERROR_MSG("desc or buffer is NULL\r\n");
       return FALSE;
    }

    real_part_len = L_MM_GET_ONE_PT_REAL_SIZE(desc->partition_len);
    mm_p = (K_L_MM_Monitor_T*)((UI8_T*)buffer - sizeof(K_L_MM_Monitor_T));

    /* check range */
    if ( ( (UI8_T*)mm_p < desc->buffer) || /* check boundary of front parition */
         ( ((UI8_T*)mm_p + real_part_len) > (desc->buffer+desc->buffer_len) ) ) /* check boundary of rear parition */
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        L_MM_ERROR_MSG("buffer to be freed doesn't belong to this desc\r\n");
        L_MM_ERROR_MSG_WITH_ARG("mm_p=0x%px, real_part_len=%d\r\n", mm_p, (int)real_part_len);
        L_MM_DEBUG_DUMP_MONITOR_INFO(printk, mm_p);
        return FALSE;
    }

    /* alignment check */
    if ( ((UI8_T*)mm_p - (UI8_T*)desc->buffer) % real_part_len )
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        L_MM_ERROR_MSG("buffer to be freed has alignment error\r\n");
        L_MM_ERROR_MSG_WITH_ARG("mm_p=0x%p, real_part_len=%d", mm_p, (int)real_part_len);
        L_MM_DEBUG_DUMP_MONITOR_INFO(printk, mm_p);
        return FALSE;
    }

    /* validate buffer */
    if(K_L_MM_Valid_Buf(buffer) == FALSE)
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        L_MM_ERROR_MSG("invalid buffer\r\n");
        return  FALSE;
    }

    /* remove from the monitor link list */
    K_L_MM_RemoveFrom_MonitorList(mm_p);

    /* return to partition's free list */
    L_MM_ENTER_CRITICAL_SECTION(&pt_desc_lock, oint);
    mm_p->buffer_addr = NULL; /* invalidate buffer_addr to avoid free the same mm_p twice */
    mm_p->next_p = (K_L_MM_Monitor_T*)desc->free_partition_list;
    desc->free_partition_list = (void*)mm_p;
    desc->nbr_of_free_partition++;
#ifdef TEST_BUFFER_CORRUPT
    memset(mm_p+1, MAGIC_PATTERN_1B, desc->partition_len);
#endif
    L_MM_LEAVE_CRITICAL_SECTION(&pt_desc_lock, oint);

    return TRUE;
} /* end of K_L_MM_PtFree */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_AllocateTxBuffer
 *------------------------------------------------------------------------------
 * PURPOSE: This function is devised for transmission exclusively.
 *          In this function, the buffer will be allocated and a mref handle
 *          will be constructed. The pdu offset will be set as
 *          SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN to guarantee that lower
 *          level module can use this buffer to transmit directly.
 *          Callers should write data to buffer from pdu pointer which could be
 *          retrieved via K_L_MM_Mref_GetPdu().
 *
 * INPUT:
 *        tx_pdu_size           - The size of the buffer for pdu
 *        user_id               - used to monitor allocated buffer which is stored in K_L_MM_Monitor_T field
 *                                user_id = trace_id(6) + pool_id(2) + module_id(8)
 *                                pool_id is at user's own usage/knowledge.
 *                                It can be used to stand for buffer being created by K_L_MM_AllocateTxBuffer
 *                                or 1st K_L_MM_AllocateTxBuffer, 2nd K_L_MM_AllocateTxBuffer, etc.
 *                                Or it can be combined with trace_id to form a different trace information
 *                                for user's own trace purpose.
 *                                user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *                                or or L_MM_USER_ID2(modul_id,ext_trace_id)
 *
 * OUTPUT:
 *          None
 *
 * RETURN:  The MREF handle for packet transmission
 *          NULL if fail.
 *
 * NOTES:
 *      1. Cares should be taken for SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN
 *         to ensure that the value of the constant always equal to the maximum
 *         reserved header length while transmitting packet in the system.
 *      2. user_id will be used once K_L_IPCMEM_Allocate() be able to accept user_id.
 *      3. ATTENTION! K_L_MM_FreeTxBuffer() MUST be called if user_defined_free_fun
 *         is provided!
 *------------------------------------------------------------------------------
 */
L_MM_Mref_Handle_T* K_L_MM_AllocateTxBuffer(UI32_T              tx_pdu_size,
                                            UI16_T              user_id)
{
    void*               buffer_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              actual_buf_size;

    actual_buf_size= tx_pdu_size + SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN;

    buffer_p = K_L_IPCMEM_Allocate(actual_buf_size, user_id);

    if(buffer_p == NULL)
    {
        /* This is considered as critical issue.
         * Show error message directly here
         */
        L_MM_DEBUG_MSG("K_L_IPCMEM_Allocate fail!\r\n");
        return NULL;
    }

    mref_handle_p = K_L_MM_Mref_Construct(buffer_p, actual_buf_size, SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN,
                                        tx_pdu_size,
                                        L_MM_MREF_FREE_FUN_TX_BUFFER, NULL, NULL);

    return mref_handle_p;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_FreeTxBuffer
 *------------------------------------------------------------------------------
 * PURPOSE:  This is the free function for MREF handle created through
 *           K_L_MM_AllocateTxBuffer.
 * INPUT:
 *           buf    - buffer to be freed
 *           cookie - The passed argument when K_L_MM_AllocateTxBuffer() is called
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:
 *     1. This function must be called in the user-defined free function which
 *        is passed to K_L_MM_AllocateTxBuffer().
 *------------------------------------------------------------------------------
 */
void K_L_MM_FreeTxBuffer(void* buf, void* cookie)
{
    if(buf == NULL)
    {
        L_MM_ERROR_MSG("buf is NULL\r\n");
        return;
    }

    K_L_IPCMEM_Free(buf);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_AllocateTxBufferFromDedicatedBufPool
 *------------------------------------------------------------------------------
 * PURPOSE: Allocate buffer for transimition from the specified buffer pool.
 *
 * INPUT:
 *          buffer_pool_id      - buffer pool id where the buffer will be allocated
 *          user_id             - used to monitor allocated buffer which is stored in K_L_MM_Monitor_T field
 *                                user_id = trace_id(6) + pool_id(2) + module_id(8)
 *                                pool_id is at user's own usage/knowledge.
 *                                It can be used to stand for buffer being created by K_L_MM_AllocateTxBuffer
 *                                or 1st K_L_MM_AllocateTxBuffer, 2nd K_L_MM_AllocateTxBuffer, etc.
 *                                Or it can be combined with trace_id to form a different trace information
 *                                for user's own trace purpose.
 *                                user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *                                or or L_MM_USER_ID2(modul_id,ext_trace_id)
 *
 * OUTPUT:
 *          None
 *
 * RETURN:  The MREF handle for packet transmission
 *          NULL if fail.
 *
 * NOTES:
 *          None
 *------------------------------------------------------------------------------
 */
static L_MM_Mref_Handle_T *K_L_MM_AllocateTxBufferFromDedicatedBufPool(UI32_T buffer_pool_id, UI32_T user_id)
{
    void*               buffer_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              buf_type;
    UI32_T              tx_pdu_size, actual_buf_size;

    switch (buffer_pool_id)
    {
        case L_MM_TX_BUF_POOL_ID_STKTPLG_HBT1:
            buf_type = L_IPCMEM_DEDICATED_BUF_TYPE_STKTPLG_HBT1;
            break;
#if (SYS_CPNT_DHCP == TRUE)
        case L_MM_TX_BUF_POOL_ID_DHCP_TX:
            buf_type = L_IPCMEM_DEDICATED_BUF_TYPE_DHCP_TX;
            break;
#endif
        default:
            L_MM_ERROR_MSG_WITH_ARG("buffer_pool_id:%d\r\n", (int)buffer_pool_id);
            return NULL;
    }

    buffer_p = K_L_IPCMEM_AllocateFromDedicatedBufPool(buf_type, user_id, &actual_buf_size);

    if(buffer_p == NULL)
    {
        /* This is considered as critical issue.
         * Show error message directly here
         */
        L_MM_DEBUG_MSG("K_L_IPCMEM_AllocateFromDedicatedBufPool fail!\r\n");
        return NULL;
    }

    tx_pdu_size = actual_buf_size - SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN;
    mref_handle_p = K_L_MM_Mref_Construct(buffer_p, actual_buf_size, SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN,
                                        tx_pdu_size,
                                        L_MM_MREF_FREE_FUN_TX_BUFFER, NULL, NULL);

    return mref_handle_p;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_Mref_GetMrefPoolRequiredSize
 *------------------------------------------------------------------------------
 * PURPOSE:  Get the total required size for mref pool.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   required buffer size to create partitions
 *           return 0 if error
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
UI32_T K_L_MM_Mref_GetMrefPoolRequiredSize(void)
{
    return L_MM_GET_PT_TOTAL_REQUIRED_SZ(sizeof(K_L_MM_Mref_T), SYS_ADPT_MAX_NBR_OF_MREF_DESC_BLOCK);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_Construct
 *------------------------------------------------------------------------------
 * PURPOSE: Initiate the mref information in raw-buffer
 *
 * INPUT:
 *        buf            - the address of data block, received packet block.
 *        buf_len        - the size of data block, not including reserved length(L_MM_MREF_PREFIX_RESERVED_SIZE),
 *                         this is used for range checking.
 *        pdu_offset     - the position of pdu, the offset(byte count) from the
 *                         caller usable buf
 *        pdu_len        - pdu length
 *        type           - the type of free function. the free function will be called
 *                         when ref_count reaches 0
 *                         L_MM_MREF_FREE_FUN_TX_BUFFER : free function for tx buffer.
 *                         L_MM_MREF_FREE_FUN_RX_BUFFER : free function for rx buffer.
 *        cookie         - cookie of free function (0 means no cookie)
 *
 * OUTPUT:
 *        None
 *
 * RETURN:  pointer to handle of L_MM_Mref if success
 *          or return NULL if fail
 *
 * NOTES:
 *      1. ref_count will be set to 1 in K_L_MM_Mref_Construct
 *      2. the buffer must be TX/RX buffer.
 *------------------------------------------------------------------------------
 */
static L_MM_Mref_Handle_T*  K_L_MM_Mref_Construct (void*               buf,
                                          UI16_T              buf_len,
                                          UI16_T              pdu_offset,
                                          UI16_T              pdu_len,
                                          UI16_T              type,
                                          void                *cookie,
                                          void                *cookie_params)
{
    K_L_MM_Mref_T*    new_mref_p;
    K_L_MM_Monitor_T* monitor_p;

    new_mref_p = (K_L_MM_Mref_T*)K_L_MM_PtAlloc(&mref_pool_desc,
                                            L_MM_USER_ID2(SYS_MODULE_CMNLIB,
                                                          CMNLIB_TYPE_TRACE_ID_L_MM_MREF_CONSTRUCT));

    if(new_mref_p==NULL)
    {
        L_MM_DEBUG_MSG("failed to alloc new_mref_p\r\n");
        return NULL;
    }

    monitor_p = (K_L_MM_Monitor_T*)new_mref_p - 1;

    /* overwrite buffer type */
    monitor_p->buffer_type = L_MM_CONVERT_TO_INTERNAL_BUFTYPE(L_MM_MREF_BUFFER_TYPE);

    /* initialized new_mref_p */
    new_mref_p->ref_count = 1;
    new_mref_p->pdu_offset = pdu_offset;
    new_mref_p->pdu_len = pdu_len;
    new_mref_p->buf_len = buf_len;
    new_mref_p->free_func_type = type;
    new_mref_p->cookie = cookie;
    new_mref_p->cookie_params = cookie_params;
    new_mref_p->buffer_addr = buf;
    memset(&(new_mref_p->user_handle), 0, sizeof(L_MM_Mref_Handle_T));

    return (L_MM_Mref_Handle_T*)new_mref_p;
} /* end of K_L_MM_Mref_Construct() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_AddRefCount
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Add ref_count by 1
 *
 * INPUT:
 *  mref_handle_p  -- pointer to handle of L_MM_Mref
 *  inc_count      -- number of reference count to be increased
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  TRUE  -- done
 *  FALSE -- invalid mref
 *
 * NOTES:
 *   L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *------------------------------------------------------------------------------
 */
BOOL_T K_L_MM_Mref_AddRefCount (L_MM_Mref_Handle_T* mref_handle_p, UI32_T inc_count)
{
    SYSFUN_IntMask_T  oint;
    K_L_MM_Mref_T*    mref_p;

    if( (mref_p=(K_L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_ERROR_MSG("mref_handle_p is NULL\r\n");
        return FALSE;
    }

    /* validate buffer */
    if(K_L_MM_Valid_Buf((void*)mref_p)==FALSE)
    {
        L_MM_ERROR_MSG("invalid mref_handle_p\r\n");
        return  FALSE;
    }

    L_MM_ENTER_CRITICAL_SECTION(&mref_lock, oint);
    mref_p->ref_count+=inc_count;
    L_MM_LEAVE_CRITICAL_SECTION(&mref_lock, oint);

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_SetFreeFuncTypeAndCookie
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Set the free function type and cookie for the given mref.
 *
 * INPUT:
 *  mref_handle_p   -- pointer to handle of L_MM_Mref
 *  free_fun_type   -- the type of free function. the free function will be called
 *                     when ref_count reaches 0
 *                     L_MM_MREF_FREE_FUN_TX_BUFFER : free function for tx buffer.
 *                     L_MM_MREF_FREE_FUN_RX_BUFFER : free function for rx buffer.
 *  cookie          -- this cookie will pass to free_func
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  TRUE  -- done
 *  FALSE -- can't set the free function and its cookie,
 *           possible reason: 1. invalid mref
 *                            2. cookie has already set.
 *                            3. ref_count is larger than 1
 *
 * NOTES:
 *  1. Free function and its cookie should only be allowed to be changed
 *     in the same CSC(Note that the same CSC doesn't need to be the same C file)
 *  2. For avoid misuse, free function and its cookie can only be changed when
 *     a) cookie is NULL
 *     b) reference count is not greater than 1
 *  3. L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *------------------------------------------------------------------------------
 */
BOOL_T K_L_MM_Mref_SetFreeFuncTypeAndCookie( L_MM_Mref_Handle_T* mref_handle_p, UI32_T free_fun_type, void *cookie)
{
    SYSFUN_IntMask_T oint;
    K_L_MM_Mref_T    *mref_p;

    if( (mref_p=(K_L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_ERROR_MSG("mref_handle_p is NULL\r\n");
        return FALSE;
    }

    /* validate buffer */
    if(K_L_MM_Valid_Buf((void*)mref_p) == FALSE)
    {
        L_MM_ERROR_MSG("invalid mref_handle_p\r\n");
        return  FALSE;
    }

    /* is free function and its cookie allowed to change?
     * only allow to change when:
     * (cookie must be NULL) AND (ref count must less or equal to 1)
     */
    if( (mref_p->cookie==NULL) && (mref_p->ref_count<=1) )
    {
        L_MM_ENTER_CRITICAL_SECTION(&mref_lock, oint);
        mref_p->free_func_type = free_fun_type;
        mref_p->cookie = cookie;
        L_MM_LEAVE_CRITICAL_SECTION(&mref_lock, oint);
        return TRUE;
    }

    L_MM_ERROR_MSG("Not allow to replace free function and cookie\r\n");
    return FALSE;

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_MovePdu
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Move pdu pointer forward or backword and automatically adjust the pdu length
 *
 * INPUT:
 *  mref_handle_p -- pointer to handle of L_MM_Mref
 *  offset        -- the postive value means moving pointer toward the PDU's rear;
 *                   the negative value means moving pointer toward the PDU's
 *                   front.
 *
 * OUTPUT:
 *  pdu_len_p     -- The length of pdu after this function is returned will be
 *                   kept in this variable.
 *
 * RETURN:
 *  Sucess  -- Return pointer to the pdu when pdu is moved successfully.
 *  Fail    -- Return NULL when failure
 *
 * NOTES:
 *   L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *------------------------------------------------------------------------------
 */
void* K_L_MM_Mref_MovePdu(L_MM_Mref_Handle_T* mref_handle_p, I32_T offset, UI32_T *pdu_len_p)
{
    SYSFUN_IntMask_T oint;
    K_L_MM_Mref_T    *mref_p;

    if( ((mref_p=(K_L_MM_Mref_T*)mref_handle_p) == NULL) ||
        (pdu_len_p==NULL))
    {
        L_MM_ERROR_MSG("mref_handle_p or pdu_len is NULL.\r\n");
        return NULL;
    }

    /* validate buffer */
    if(K_L_MM_Valid_Buf((void*)mref_p) == FALSE)
    {
        L_MM_ERROR_MSG("Invalid mref_handle_p\r\n");
        return  NULL;
    }

    /* boundary check */
    if ( (mref_p->pdu_offset + offset) > mref_p->buf_len)
    {
        if(l_mm_backdoor_dbg_msg_flag==TRUE)
        {
            printk("\r\n<4>%s() fails: the specified moving offset "
                   "exceeds the end of the buffer\r\nmoving offset=%d\r\n",
                   __FUNCTION__, (int)offset);
            L_MM_DEBUG_DUMP_MONITOR_INFO(printk, ((K_L_MM_Monitor_T*)mref_p-1));
        }
        return NULL;
    }

    /* offset can't be negative */
    if ((I32_T)mref_p->pdu_offset + offset < 0)
    {
        if(l_mm_backdoor_dbg_msg_flag==TRUE)
        {
            printk("\r\n<4>%s() fails: pdu_offset becomes negative value"
                   "mref->pdu_offset=%u,moving offset=%d\r\n",
                   __FUNCTION__, mref_p->pdu_offset, (int)offset);
            L_MM_DEBUG_DUMP_MONITOR_INFO(printk, ((K_L_MM_Monitor_T*)mref_p-1));
        }
        return NULL;
    }

    L_MM_ENTER_CRITICAL_SECTION(&mref_lock, oint);
    /* move pdu pointer & adjust pdu length */
    *pdu_len_p = mref_p->pdu_len = mref_p->pdu_len - offset;
    mref_p->pdu_offset = (UI16_T)(mref_p->pdu_offset + offset);
    L_MM_LEAVE_CRITICAL_SECTION(&mref_lock, oint);

    return ((UI8_T*)(mref_p->buffer_addr) + mref_p->pdu_offset);
} /* end of K_L_MM_Mref_MovePdu */

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_Mref_ShiftPdu
 *------------------------------------------------------------------------------
 * PURPOSE:  Shift the pdu block toward the end of the buffer.
 *
 * INPUT:    mref_handle_p  -  pointer to handle of L_MM_Mref
 *           shift_offset   -  the byte count to shift from original place
 *
 * OUTPUT:   None.
 *
 * RETURN:   TRUE  -- Sucess
 *           FALSE -- Fail
 *
 * NOTE:     The shift operation will only sucess when the left buffer size
 *           behind current pdu block is large enough
 *------------------------------------------------------------------------------
 */
BOOL_T K_L_MM_Mref_ShiftPdu(L_MM_Mref_Handle_T* mref_handle_p, UI32_T shift_offset)
{
    K_L_MM_Mref_T *mref_p;
    UI8_T       *pdu;

    if( (mref_p=(K_L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_ERROR_MSG("mref_handle_p is NULL.\r\n");
        return FALSE;
    }

    /* check whether the buffer behind pdu is large enough to do pdu shifting */
    if( (mref_p->buf_len - mref_p->pdu_offset - mref_p->pdu_len) < shift_offset)
    {
        L_MM_ERROR_MSG("The left buffer size behind pdu is not enough\r\n");
        return FALSE;
    }

    /* shift pdu */
    pdu = (UI8_T*)mref_p->buffer_addr + mref_p->pdu_offset;
    memmove(pdu + shift_offset, pdu, mref_p->pdu_len);

    /* update pdu offset */
    mref_p->pdu_offset += shift_offset;

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_GetPdu
 *------------------------------------------------------------------------------
 * PURPOSE: get PDU address and PDU length
 *
 * INPUT:
 *  mref_handle_p  - pointer to handle of L_MM_Mref
 *  caller_addr    - the caller address (for MREF error log)
 *
 * OUTPUT:
 *  pdu_len_p          - length of pdu
 *  prev_caller_addr_p - the previous caller address (for MREF error log)
 *                       The output value is zero when no MREF error is detected.
 *                       The output value is non-zero when MREF error is detected.
 *  prev_caller_tid_p  - the previous caller task id (for MREF error log)
 *                       The output value only valid when prev_caller_addr_p is
 *                       not zero.
 *
 * RETURN: pdu address or NULL if fail
 *
 * NOTES:
 *   1. L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *   2. When MREF descriptor buffer corruption is detected, the prev_caller_addr_p
 *      will be non-zero value. Otherwise, prev_caller_addr_p will be zero.
 *------------------------------------------------------------------------------
 */
void *K_L_MM_Mref_GetPdu (L_MM_Mref_Handle_T* mref_handle_p, void *caller_addr_p,
    UI32_T *pdu_len_p, uintptr_t *prev_caller_addr_p, UI32_T *prev_caller_tid_p)
{
    #define MAX_COUNT_OF_REPORT_ERR 10

    SYSFUN_IntMask_T oint;
    K_L_MM_Mref_T    *mref_p;
    void             *pdu_ptr;
    static int    report_err_count=0;

    if (l_mm_backdoor_mref_error_detect_flag==TRUE)
    {
        L_MM_ENTER_CRITICAL_SECTION(&pt_desc_lock, oint);
        if (K_L_MM_CheckMrefPool()==FALSE && (report_err_count<MAX_COUNT_OF_REPORT_ERR))
        {
            /* output prev_caller_addr_p when check mref pool return false */
            *prev_caller_addr_p = previous_caller_addr;
            *prev_caller_tid_p = previous_caller_tid;
            report_err_count++;
        }
        else
        {
            *prev_caller_addr_p=0;
        }
        /* update previus caller addr */
        previous_caller_addr = L_CVRT_PTR_TO_UINT(caller_addr_p);
        previous_caller_tid = current->pid;
        L_MM_LEAVE_CRITICAL_SECTION(&pt_desc_lock, oint);
    }

    if( (mref_p=(K_L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_DEBUG_MSG("mref_handle_p is NULL.\r\n");
        return NULL;
    }

    /* validate buffer */
    if(K_L_MM_Valid_Buf((void*)mref_p)==FALSE)
    {
        L_MM_ERROR_MSG("invalid mref_handle_p\r\n");
        return NULL;
    }

    L_MM_ENTER_CRITICAL_SECTION(&mref_lock, oint);
    *pdu_len_p= mref_p->pdu_len;
    pdu_ptr=((UI8_T*)(mref_p->buffer_addr) + mref_p->pdu_offset);
    L_MM_LEAVE_CRITICAL_SECTION(&mref_lock, oint);

    return pdu_ptr;
} /* end of K_L_MM_Mref_GetPdu */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_Release
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Free the mref, if reference count is 0 then free the associated buffer.
 * INPUT:
 *  mref_handle_pp  -- pointer to pointer of handle of L_MM_Mref
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *   Sucess - reference count left, if return 0 mean the buffer is freed.
 *   Fail   - Return L_MM_ERROR_RETURN_VAL if fail
 *
 * NOTE:
 *   1.L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 *   2.mref_handle_pp will be set as NULL when reference count equals zero
 *     after this function is returned.
 *------------------------------------------------------------------------------
 */
UI32_T  K_L_MM_Mref_Release (L_MM_Mref_Handle_T** mref_handle_pp)
{
    SYSFUN_IntMask_T oint;
    UI32_T           ref_count;
    K_L_MM_Mref_T    *mref_p;

    if( (mref_handle_pp) == NULL)
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        L_MM_ERROR_MSG("mref_handle_p is NULL\r\n");
        return L_MM_ERROR_RETURN_VAL;
    }

    mref_p=(K_L_MM_Mref_T*)(*mref_handle_pp);

    /* validate buffer */
    if(K_L_MM_Valid_Buf((void*)mref_p)==FALSE)
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        L_MM_ERROR_MSG("invalid mref_handle_p\r\n");
        return L_MM_ERROR_RETURN_VAL;
    }

    L_MM_ENTER_CRITICAL_SECTION(&mref_lock, oint);
    ref_count = --(mref_p->ref_count);
    L_MM_LEAVE_CRITICAL_SECTION(&mref_lock, oint);

    if (ref_count == 0)
    {
        *mref_handle_pp=NULL;

        if (mref_p->free_func_type == L_MM_MREF_FREE_FUN_TX_BUFFER)
            K_L_MM_FreeTxBuffer(mref_p->buffer_addr, mref_p->cookie);

        if(K_L_MM_PtFree(&mref_pool_desc, mref_p)==FALSE)
        {
           /* This is considered as critical issue.
            * Show error message directly here
            */
            L_MM_ERROR_MSG("Fail to free mref descriptor\r\n");
            L_MM_ERROR_MSG_WITH_ARG("mref_pool_desc:buffer=0x%p, buffer_len=%d, partition_len =%d, nbr_of_free_partition=%d, buffer_type=0x%x\r\n",
                mref_pool_desc.buffer, (int)mref_pool_desc.buffer_len, (int)mref_pool_desc.partition_len,
                (int)mref_pool_desc.nbr_of_free_partition, mref_pool_desc.buffer_type);
        }
    }

    return ref_count;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_Operation
 *------------------------------------------------------------------------------
 * PURPOSE:
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static UI32_T K_L_MM_Operation(void* cmd, void* arg1, void* arg2, void* arg3, void* arg4)
{
    UI32_T ret=L_MM_ERROR_RETURN_VAL;
    int    cmd_val;

    SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
    cmd_val=(int)cmd;
    SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
    switch(cmd_val)
    {
        case L_MM_CMD_ALLOCATE_TX_BUFFER:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: tx_pdu_size
             * arg2: user_id
             * arg3: NULL
             * arg4: NULL
             * ret : offset to L_MM_Mref_Handle_T* / L_MM_ERROR_RETURN_VAL
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = K_L_MM_AllocateTxBuffer((UI32_T)arg1, (UI16_T)arg2);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            ret = K_L_IPCMEM_GetOffset(mref_handle_p);
            break;
        }
        case L_MM_CMD_ALLOCATE_TX_BUFFER_FROM_DEDICATED_BUFFER:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: buf_type
             * arg2: user_id
             * arg3: NULL
             * arg4: NULL
             * ret : offset to L_MM_Mref_Handle_T*
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = K_L_MM_AllocateTxBufferFromDedicatedBufPool((UI32_T)arg1, (UI32_T)arg2);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            ret = K_L_IPCMEM_GetOffset(mref_handle_p);
            break;
        }
        case L_MM_CMD_CONSTRUCT:
        {
            L_MM_Mref_ConstructArg_T arg;
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: L_MM_Mref_ConstructArg_T*
             * arg2: NULL
             * arg3: NULL
             * arg4: NULL
             * ret : L_MM_Mref_Handle_T*
             */
            SYSFUN_CopyFromUser(&arg, (void *)arg1, sizeof(arg));
            if(arg.free_func_type==L_MM_MREF_FREE_FUN_CUSTOM)
                mref_handle_p = K_L_MM_Mref_Construct((void*)(arg.buf_p), arg.buf_len, arg.pdu_offset, arg.pdu_len, arg.free_func_type, arg.cookie, arg.cookie_params);
            else
                mref_handle_p = K_L_MM_Mref_Construct(K_L_IPCMEM_GetPtr((UI32_T)arg.buf_offset), arg.buf_len, arg.pdu_offset, arg.pdu_len, arg.free_func_type, arg.cookie, arg.cookie_params);
            ret = (UI32_T)K_L_IPCMEM_GetOffset(mref_handle_p);
            break;
        }
        case L_MM_CMD_ADD_REF_COUNT:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: inc_count
             * arg3: NULL
             * arg4: NULL
             * ret : TRUE/FALSE
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = (L_MM_Mref_Handle_T*)K_L_IPCMEM_GetPtr((UI32_T)arg1);
            ret = (UI32_T)K_L_MM_Mref_AddRefCount(mref_handle_p, (UI32_T)arg2);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            break;
        }
        case L_MM_CMD_SET_FREE_FUNC_TYPE_AND_COOKIE:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: free_fun_type
             * arg3: cookie
             * arg4: NULL
             * ret : TRUE/FALSE
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = (L_MM_Mref_Handle_T*)K_L_IPCMEM_GetPtr((UI32_T)arg1);
            ret = (UI32_T)K_L_MM_Mref_SetFreeFuncTypeAndCookie(mref_handle_p, (UI32_T)arg2, arg3);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);

            break;
        }
        case L_MM_CMD_MOVE_PDU:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            void*              new_pdu_p;
            UI32_T             pdu_len;
            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: offset
             * arg3: pdu_len_p
             * arg4: NULL
             * ret : offset to new_pdu_p / L_MM_ERROR_RETURN_VAL
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = (L_MM_Mref_Handle_T*)K_L_IPCMEM_GetPtr((UI32_T)arg1);
            new_pdu_p = K_L_MM_Mref_MovePdu(mref_handle_p, (I32_T)arg2, &pdu_len);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);

            SYSFUN_CopyToUser(arg3, &pdu_len, sizeof(pdu_len));
            if(((K_L_MM_Mref_T*)mref_handle_p)->free_func_type!=L_MM_MREF_FREE_FUN_CUSTOM)
                ret = (UI32_T)K_L_IPCMEM_GetOffset(new_pdu_p);
            else
                ret = (UI32_T)L_CVRT_GET_OFFSET(((K_L_MM_Mref_T*)mref_handle_p)->buffer_addr, new_pdu_p);
            break;
        }
        case L_MM_CMD_SHIFT_PDU:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: shift_offset
             * arg3: NULL
             * arg4: NULL
             * ret : TRUE/FALSE
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = (L_MM_Mref_Handle_T*)K_L_IPCMEM_GetPtr((UI32_T)arg1);
            ret = (UI32_T)K_L_MM_Mref_ShiftPdu(mref_handle_p, (UI32_T)arg2);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            break;
        }
        case L_MM_CMD_RELEASE:
        {
            L_MM_Mref_Handle_T *mref_handle_p;
            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: NULL
             * arg3: NULL
             * arg4: NULL
             * ret : reference count / L_MM_ERROR_RETURN_VAL
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_handle_p = (L_MM_Mref_Handle_T *)K_L_IPCMEM_GetPtr((UI32_T)arg1);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            ret = K_L_MM_Mref_Release(&mref_handle_p);
            break;
        }

        case L_MM_CMD_GET_PDU:
        {
            K_L_MM_Mref_T *mref_p;
            UI32_T pdu_len;
            UI32_T prev_caller_tid=0;
			uintptr_t prev_caller_addr=0;

            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: caller addr
             * arg3: previous caller addr(OUTPUT)
             * arg4: previous caller tid(OUTPUT)
             * ret: the offset of buffer_addr
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_p = (K_L_MM_Mref_T *)K_L_IPCMEM_GetPtr((UI32_T)arg1);
			SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            K_L_MM_Mref_GetPdu ((L_MM_Mref_Handle_T*)mref_p, arg2, &pdu_len, &prev_caller_addr, &prev_caller_tid);
            SYSFUN_CopyToUser((void *)arg3, &prev_caller_addr, sizeof(prev_caller_addr));
            SYSFUN_CopyToUser((void *)arg4, &prev_caller_tid, sizeof(prev_caller_tid));
            ret = (UI32_T)K_L_IPCMEM_GetOffset(mref_p->buffer_addr);
            break;
        }

        case L_MM_CMD_GET_BUFFER_OFFSET:
        {
            K_L_MM_Mref_T *mref_p;
            /* arg1: offset to L_MM_Mref_Handle_T*
             * arg2: NULL
             * arg3: NULL
             * arg4: NULL
             * ret : the offset of buffer_addr
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            mref_p = (K_L_MM_Mref_T *)K_L_IPCMEM_GetPtr((UI32_T)arg1);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            ret = (UI32_T)K_L_IPCMEM_GetOffset(mref_p->buffer_addr);
            break;
        }
        case L_MM_CMD_BACKDOOR:
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            cmd_val=(int)arg1;
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
            switch(cmd_val)
            {
                case L_MM_BACKDOOR_CMD_GETBUFFERINFO:
                {
                    /* arg2: L_MM_Backdoor_BufferInfo_T*
                     * ret : TRUE  -- get an entry sucessfully
                     *       FALSE -- no more entry to get
                     */
                    L_MM_Backdoor_BufferInfo_T backdoor_buf_info;
                    if(0!=copy_from_user(&backdoor_buf_info, arg2, sizeof(backdoor_buf_info)))
                    {
                        L_MM_ERROR_MSG("copy_from_user fail\r\n");
                        return FALSE;
                    }
                    ret = K_L_MM_GetBufferInfo(&backdoor_buf_info);
                    if(0!=copy_to_user(arg2, &backdoor_buf_info, sizeof(backdoor_buf_info)))
                    {
                        L_MM_ERROR_MSG("copy_to_user fail\r\n");
                        return FALSE;
                    }
                    break;
                }
                case L_MM_BACKDOOR_CMD_GETMREFINFO:
                {
                    /* arg2: L_MM_Backdoor_MrefInfo_T*
                     * ret : TRUE  -- get an entry sucessfully
                     *       FALSE -- no more entry to get
                     */
                    L_MM_Backdoor_MrefInfo_T backdoor_mref_info;
                    if(0!=copy_from_user(&backdoor_mref_info, arg2, sizeof(backdoor_mref_info)))
                    {
                        L_MM_ERROR_MSG("copy_from_user fail\r\n");
                        return FALSE;
                    }
                    ret = K_L_MM_GetMrefInfo(&backdoor_mref_info);
                    if(0!=copy_to_user(arg2, &backdoor_mref_info, sizeof(backdoor_mref_info)))
                    {
                        L_MM_ERROR_MSG("copy_to_user fail\r\n");
                        return FALSE;
                    }
                    break;
                }
                case L_MM_BACKDOOR_CMD_GETTOTALNBROFALLOCATEDBUFFER:
                    /* ret : total number of allocated buffer
                     */
                     ret = l_mm_backdoor_buff_nbr;
                    break;
                case L_MM_BACKDOOR_CMD_TOGGLEDBGMSGFLAG:
                    /* ret : result of toggle debug message flag */
                    l_mm_backdoor_dbg_msg_flag = !l_mm_backdoor_dbg_msg_flag;
                    ret = l_mm_backdoor_dbg_msg_flag;
                    break;
                case L_MM_BACKDOOR_CMD_TOGGLEVALIDATEFREEFLAG:
                    /* ret : result of validate free flag
                     */
                    l_mm_backdoor_validate_free_flag = (l_mm_backdoor_validate_free_flag==TRUE) ? FALSE : TRUE;
                    ret = l_mm_backdoor_validate_free_flag;
                    break;
                case L_MM_BACKDOOR_CMD_TOGGLEMREFERRDETECTFLAG:
                    /* ret : result of mref error detect flag
                     */
                    l_mm_backdoor_mref_error_detect_flag = (l_mm_backdoor_mref_error_detect_flag==TRUE) ? FALSE : TRUE;
                    ret = l_mm_backdoor_mref_error_detect_flag;
                    break;
                case L_MM_BACKDOOR_CMD_MREF_CORRUPT:
                    /* ret : result of mref corrupt operation
                     */
                    ret = K_L_MM_Mref_Corrupt();
                    break;
                default:
                    L_MM_ERROR_MSG_WITH_ARG("Unknown L_MM Backdoor cmd(%d)\r\n", cmd_val);
                    ret = FALSE;
            }
            break;
        default:
            printk("\r\n<4>%s:Unknown cmd(%d).", __FUNCTION__, cmd_val);
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_InsertTo_MonitorList
 *------------------------------------------------------------------------------
 * PURPOSE:  Insert a monitor node into monitor list
 * INPUT:    mm_p  -  pointer to monitor node to be inserted
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     1. Caller of this function must guarntee that the input argument is
 *              valid.
 *------------------------------------------------------------------------------
 */
static void K_L_MM_InsertTo_MonitorList(K_L_MM_Monitor_T *mm_p)
{
    SYSFUN_IntMask_T oint;

    L_MM_ENTER_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
    mm_p->next_p = l_mm_monitor_lst_head;
    mm_p->prev_p = NULL;

    if  (l_mm_monitor_lst_head != NULL)
    {
        l_mm_monitor_lst_head->prev_p = mm_p;
    }
    l_mm_monitor_lst_head = mm_p;
    l_mm_backdoor_buff_nbr++;

    L_MM_LEAVE_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - L_MM_RemoveFrom_MonitorList
 *------------------------------------------------------------------------------
 * PURPOSE:  Remove a monitor node into monitor list
 * INPUT:    mm_p  -  pointer to monitor node to be removed
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     1. Caller of this function must guarntee that the input argument is
 *              valid.
 *------------------------------------------------------------------------------
 */
static void K_L_MM_RemoveFrom_MonitorList(K_L_MM_Monitor_T *mm_p)
{
    SYSFUN_IntMask_T oint;

    L_MM_ENTER_CRITICAL_SECTION(&mm_monitor_list_lock, oint);

    if (mm_p == l_mm_monitor_lst_head)
    {
        l_mm_monitor_lst_head = mm_p->next_p;
        if(l_mm_monitor_lst_head!=NULL)
            l_mm_monitor_lst_head->prev_p = NULL;
    }
    else
    {
        mm_p->prev_p->next_p = mm_p->next_p;
        if(mm_p->next_p!=NULL)
            mm_p->next_p->prev_p = mm_p->prev_p;
    }
    l_mm_backdoor_buff_nbr--;

    L_MM_LEAVE_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - K_L_MM_Valid_Buf
 *------------------------------------------------------------------------------
 * PURPOSE:  For validating buffer prefixed with K_L_MM_Monitor_T and suffixed with
 *           trail magic bytes.
 * INPUT:    buff_p  -  pointer to buffer being validated
 * OUTPUT:   None.
 * RETURN:   TRUE  -- Valid buffer
 *           FALSE -- Invalid buffer
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static BOOL_T K_L_MM_Valid_Buf(void *buff_p)
{
    K_L_MM_Monitor_T *mm_p;
    BOOL_T is_valid_magic_bytes;
    BOOL_T ret_val=TRUE;

    mm_p = (K_L_MM_Monitor_T*)((UI8_T*)buff_p - sizeof(K_L_MM_Monitor_T));

    /* check buffer address */
    if (mm_p->buffer_addr != mm_p)
    {
         L_MM_ERROR_MSG("buffer validation fails through buffer_addr check\r\n");
         L_MM_ERROR_MSG_WITH_ARG("mm_p=0x%p\r\n", mm_p);
         ret_val=FALSE;
    }

    /* check leading magic byte */
    L_MM_IS_VALID_LEADING_MAIC_BYTES(&is_valid_magic_bytes, mm_p);
    if(is_valid_magic_bytes==FALSE)
    {
        L_MM_ERROR_MSG("buffer validation fails through leading magic byte check\r\n");
        L_MM_ERROR_MSG_WITH_ARG("mm_p=0x%p\r\n", mm_p);
        ret_val=FALSE;
    }

    /* check trail magic byte */
    L_MM_IS_VALID_TRAIL_MAGIC_BYTES(&is_valid_magic_bytes, mm_p);
    if (is_valid_magic_bytes==FALSE)
    {
        L_MM_ERROR_MSG("buffer validation fails through trail magic byte check\r\n");
        L_MM_ERROR_MSG_WITH_ARG("mm_p=0x%p\r\n", mm_p);
        ret_val=FALSE;
    }

    return ret_val;
}

#if (BACKDOOR_OPEN==TRUE)

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_GetBufferInfo
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get buffer info from the monitor list.
 *------------------------------------------------------------------------------
 * INPUT:
 * buffer_info_p->last_ref_monitor_p       --  Keep the pointer to last referenced
 *                                             monitor_p. Callers should set this
 *                                             field as NULL to retrieve the first
 *                                             entry.
 *
 * OUTPUT:
 * buffer_info_p->uid                      --  user id. See description of L_MM_USER_ID
 *                                             in L_Mm.h
 * buffer_info_p->buf_len                  --  the size of the buffer
 * buffer_info_p->buffer_type              --  buffer type. See description of L_MM_USER_ID
 *                                             in L_Mm.h
 * buffer_info_p->is_head_magic_bytes_ok   --  Is the head magic bytes of the buffer right?
 * buffer_info_p->is_trail_magic_bytes_ok  --  Is the tail magic bytes of the buffer right?
 * buffer_info_p->task_id                  --  the task id of the task who request this buffer
 * buffer_info_p->allocation_time          --  the time that the buffer was allocated
 * buffer_info_p->buffer_addr              --  the address of the buffer in kernel
 *
 * RETURN:
 *  TRUE  --  An entry is retrieved sucessfully.
 *  FALSE --  No more entry can be retrieved.
 *
 * NOTES:
 *   1.L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 */
static BOOL_T K_L_MM_GetBufferInfo(L_MM_Backdoor_BufferInfo_T *buffer_info_p)
{
    SYSFUN_IntMask_T oint;
    K_L_MM_Monitor_T *monitor_p;

    if(buffer_info_p->last_ref_monitor_p==NULL)
        monitor_p = l_mm_monitor_lst_head;
    else
        monitor_p = ((K_L_MM_Monitor_T*)(buffer_info_p->last_ref_monitor_p))->next_p;

    if(monitor_p==NULL)
        return FALSE;

    buffer_info_p->last_ref_monitor_p = monitor_p;

    L_MM_ENTER_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
    /* check whether the monitor_p is still valid(i.e.has not been freed)
     */
    if(monitor_p->buffer_addr==NULL)
    {
        L_MM_LEAVE_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
        return FALSE;
    }

    /* monitor_p is valid, start filling buffer_info_p
     */
    buffer_info_p->task_id = monitor_p->task_id;
    buffer_info_p->allocation_time = monitor_p->allocation_time;
    buffer_info_p->buffer_addr = monitor_p->buffer_addr;
    buffer_info_p->last_ref_monitor_p = monitor_p;
    buffer_info_p->uid            = monitor_p->uid;
    buffer_info_p->buf_len        = monitor_p->buf_len;
    buffer_info_p->buffer_type    = monitor_p->buffer_type;

    L_MM_IS_VALID_LEADING_MAIC_BYTES(&(buffer_info_p->is_valid_leading_magic_byte), monitor_p);
    L_MM_IS_VALID_TRAIL_MAGIC_BYTES(&(buffer_info_p->is_valid_trail_magic_byte), monitor_p);
    L_MM_LEAVE_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_Mref_Corrupt
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Corrupt a MREF descriptor buffer.
 *------------------------------------------------------------------------------
 * INPUT:
 *  None.
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  TRUE  --  A MREF descriptor buffer is corrupted sucessfully.
 *  FALSE --  Failed to corrupt a MREF descriptor buffer.
 *
 * NOTES:
 *  None.
 */
static BOOL_T K_L_MM_Mref_Corrupt(void)
{
    K_L_MM_Mref_T*    new_mref_p;
    K_L_MM_Monitor_T* monitor_p;

    new_mref_p = (K_L_MM_Mref_T*)K_L_MM_PtAlloc(&mref_pool_desc,
                                                L_MM_USER_ID2(SYS_MODULE_CMNLIB,
                                                4));

    if(new_mref_p==NULL)
    {
        L_MM_ERROR_MSG("failed to alloc new_mref_p\r\n");
        return FALSE;
    }

    monitor_p = (K_L_MM_Monitor_T*)new_mref_p - 1;

    /* overwrite buffer type */
    monitor_p->buffer_type = L_MM_CONVERT_TO_INTERNAL_BUFTYPE(L_MM_MREF_BUFFER_TYPE);
    monitor_p->magic[0]=0;

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_GetMrefInfo
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get mref info from the monitor list.
 *------------------------------------------------------------------------------
 * INPUT:
 * mref_info_p->buffer_info.last_ref_monitor_p --  Keep the pointer to last referenced
 *                                                   monitor_p. Callers should set this
 *                                                   field as NULL to retrieve the first
 *                                                   entry.
 *
 * OUTPUT:
 * mref_info_p->buffer_info.uid                      --  user id. See description of L_MM_USER_ID
 *                                             in L_Mm.h
 * mref_info_p->buffer_info.buf_len                  --  the size of the buffer
 * mref_info_p->buffer_info.buffer_type              --  buffer type. See description of L_MM_USER_ID
 *                                             in L_Mm.h
 * mref_info_p->buffer_info.is_head_magic_bytes_ok   --  Is the head magic bytes of the buffer right?
 * mref_info_p->buffer_info.is_trail_magic_bytes_ok  --  Is the tail magic bytes of the buffer right?
 * mref_info_p->buffer_info.task_id                  --  the task id of the task who request this buffer
 * mref_info_p->buffer_info.allocation_time          --  the time that the buffer was allocated
 * mref_info_p->buffer_info.buffer_addr              --  the address of the buffer in kernel
 * mref_info_p->current_usr_id                       --  id of the current user of MREF
 * mref_info_p->next_usr_id                          --  id of the next user of MREF
 * mref_info_p->ref_count                            --  reference count
 * mref_info_p->pdu_offset                           --  pdu offset
 * mref_info_p->pdu_len                              --  pdu length
 *
 * RETURN:
 *  TRUE  --  An entry is retrieved sucessfully.
 *  FALSE --  No more entry can be retrieved.
 *
 * NOTES:
 *   1.L_MM_ENTER_CRITICAL_SECTION/L_MM_LEAVE_CRITICAL_SECTION are needed in this procedure
 */
static BOOL_T K_L_MM_GetMrefInfo(L_MM_Backdoor_MrefInfo_T *mref_info_p)
{
    SYSFUN_IntMask_T oint;
    K_L_MM_Monitor_T *monitor_p;
    K_L_MM_Mref_T    *mref_p;

    if((mref_info_p->buffer_info).last_ref_monitor_p==NULL)
        monitor_p = l_mm_monitor_lst_head;
    else
        monitor_p = ((K_L_MM_Monitor_T*)((mref_info_p->buffer_info).last_ref_monitor_p))->next_p;

    if(monitor_p==NULL)
        return FALSE;

    /* locate mref buffer in the monitor list
     */
    L_MM_ENTER_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
    while(L_MM_GET_INTERNAL_BUFTYPE(monitor_p->buffer_type)!=L_MM_MREF_BUFFER_TYPE)
    {
        monitor_p = monitor_p->next_p;
        if(monitor_p==NULL)
        {
            L_MM_LEAVE_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
            return FALSE;
        }
    }

    mref_p = (K_L_MM_Mref_T*)(monitor_p+1);

    /* monitor_p is valid, start filling mref_info_p->buffer_info
     */
    (mref_info_p->buffer_info).task_id = monitor_p->task_id;
    (mref_info_p->buffer_info).allocation_time = monitor_p->allocation_time;
    (mref_info_p->buffer_info).buffer_addr = monitor_p->buffer_addr;
    (mref_info_p->buffer_info).last_ref_monitor_p = monitor_p;
    (mref_info_p->buffer_info).uid            = monitor_p->uid;
    (mref_info_p->buffer_info).buf_len        = monitor_p->buf_len;
    (mref_info_p->buffer_info).buffer_type    = monitor_p->buffer_type;
    /* filling other mref_p fields
     */
    mref_info_p->current_usr_id = mref_p->user_handle.current_usr_id;
    mref_info_p->next_usr_id = mref_p->user_handle.next_usr_id;
    mref_info_p->ref_count = mref_p->ref_count;
    mref_info_p->pdu_offset = mref_p->pdu_offset;
    mref_info_p->pdu_len = mref_p->pdu_len;

    L_MM_IS_VALID_LEADING_MAIC_BYTES(&((mref_info_p->buffer_info).is_valid_leading_magic_byte), monitor_p);
    L_MM_IS_VALID_TRAIL_MAGIC_BYTES(&((mref_info_p->buffer_info).is_valid_trail_magic_byte), monitor_p);
    L_MM_LEAVE_CRITICAL_SECTION(&mm_monitor_list_lock, oint);
    (mref_info_p->buffer_info).last_ref_monitor_p = monitor_p;
    return TRUE;
}

#endif

