/* Module Name: L_MM.C(User space)
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
 *   The allocated buffer by L_MM_Malloc(), L_MM_PtAlloc() is consisted of :
 *      buffer :=  L_MM_Monitor_T + Application's Data Block + trail Magic bytes(L_MM_TRAIL_MAGIC_BYTE_LEN bytes)
 *   The allocated buffer for L_MM_Mref_Construct() is consisted of:
 *      buffer :=  L_MM_Monitor_T + L_MM_Mref_T + Application's Data Block + trail Magic bytes(L_MM_TRAIL_MAGIC_BYTE_LEN bytes)
 *   L_MM depends on L_IPCMEM on linux, so it is necessary to call L_IPCMEM_Init() for
 *   L_MM to work properly.
 * History:
 *    2005/6       -- Ryan, Create
 *    2006/1/17    -- Charlie, add a API L_MM_AllocateTxBuffer() for buffer
 *                             allocation of packet transmission
 *    2006/10/13   -- Charlie, port to linux
 *    2008/7/3     -- Tiger Liu, remove unused function, rename functions L_MM_XXX to K_L_MM_XXX.
 * Copyright(C)      Accton Corporation, 2004, 2005, 2006
 */

/* INCLUDE FILE DECLARATIONS
 */
/* standard C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Accton include files */

#include "sys_type.h"
#include "sysfun.h"
#include "l_mm.h"
#include "l_mm_type.h"
#include "l_ipcmem.h"
#include "l_math.h"
#include "l_cvrt.h"
#include "backdoor_mgr.h"

#define BACKDOOR_OPEN TRUE

/* NAMING CONSTANT DECLARACTION
 */
#define L_MM_LEADING_MAGIC_BYTE_LEN    1
#define L_MM_TRAIL_MAGIC_BYTE_LEN      4 /* should be multiple of 4 */

#define L_MM_MAGIC_BYTE_VALUE          0xAA

#define L_MM_MAX_USER_BUFFER_TYPE      0x3F

#define EXECUTE_MREF_LOG_SYSTEM_SHELL_BUF_LEN 48

/* MACRO FUNCTION DECLARACTION
 */
#define L_MM_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(l_mm_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define L_MM_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(l_mm_sem_id)

#define L_MM_DEBUG_MSG(msg) \
    if(l_mm_backdoor_dbg_msg_flag==TRUE)\
    {\
        BACKDOOR_MGR_Printf("%s():"msg"\r\n", __FUNCTION__);\
    }

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
#define L_MM_GET_ONE_PT_REAL_SIZE(usr_partition_size) ((((usr_partition_size) + sizeof(L_MM_Monitor_T) + L_MM_TRAIL_MAGIC_BYTE_LEN + 3) / 4 ) * 4 )

#define L_MM_GET_PT_TOTAL_REQUIRED_SZ(usr_partition_size, partition_num) (L_MM_GET_ONE_PT_REAL_SIZE(usr_partition_size) * (partition_num))

#if (BACKDOOR_OPEN==TRUE)
static BOOL_T L_MM_CheckFreedBuf(L_MM_Monitor_T* mm_p);
#define L_MM_DEBUG_DO_CHECK_FREED_BUFFER() \
    if(l_mm_backdoor_validate_free_flag==TRUE) \
    { \
        if(L_MM_CheckFreedBuf(mm_p)==FALSE) \
        { \
            return FALSE; \
        } \
    }
#else
#define L_MM_DEBUG_DO_CHECK_FREED_BUFFER()
#endif

#if (L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER==TRUE)
    /* For debug purpose: to measure that is the buffer allocated by L_MM_Malloc()
     * has been used over a specified period of time(the coding rule is 1 sec now)
     */
    #define L_MM_DEBUG_MAX_USE_TIME_OF_MALLOC_BUFFER    100

    /* for checking elapsed time
     */
    #define L_MM_DEBUG_DO_CHECK_ELLAPSED_TIME_OF_MALLOC_BUFFER() \
        if(l_mm_backdoor_check_elapsed_time_flag==TRUE) \
        { \
          \
            if(TRUE==L_MATH_TimeOut32(SYSFUN_GetSysTick(), mm_p->allocation_time+L_MM_DEBUG_MAX_USE_TIME_OF_MALLOC_BUFFER)) \
            { \
                BACKDOOR_MGR_Printf("L_MM_Free():Elapsed time of the buffer is over 1 sec. Tid=%lu.module_id=%hu.ext_trace_id=%hu.size=%hu.elapsed_time=%luticks.\r\n", \
                    SYSFUN_TaskIdSelf(), L_MM_GET_MODULEID_FROM_USERID(mm_p->uid), L_MM_GET_EXT_TRACEID_FROM_USERID(mm_p->uid), mm_p->buf_len, curr_tick-mm_p->allocation_time, NULL); \
            } \
        }
    /* for showing backdoor menu item
     */
    #define L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM() \
        BACKDOOR_MGR_Print(" 8. Toggle malloc buffer elapsed time check flag\r\n");

    /* for toggling the flag for checking elapsed time of malloc buffer
     */
    #define L_MM_DEBUG_TOGGLE_FLAG_OF_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER() \
        l_mm_backdoor_check_elapsed_time_flag=!l_mm_backdoor_check_elapsed_time_flag; \
    BACKDOOR_MGR_Printf("Flag %s\r\n", l_mm_backdoor_check_elapsed_time_flag ? "On" : "Off");

    #define L_MM_DEBUG_GET_CURRENT_TICK() \
        UI32_T curr_tick = SYSFUN_GetSysTick();
#else
    #define L_MM_DEBUG_DO_CHECK_ELLAPSED_TIME_OF_MALLOC_BUFFER()
    #define L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM()
    #define L_MM_DEBUG_TOGGLE_FLAG_OF_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER()
    #define L_MM_DEBUG_GET_CURRENT_TICK()
#endif

#define L_MM_GET_MONITOR_PART_FROM_MREF(mref_p) ((L_MM_Monitor_T*)mref_p-1)

/* for define RX free function
 */
extern BOOL_T DEV_NICDRV_MREF_MemFree(void *buf, UI32_T cookie, void *cookie_params);
#define L_MM_FreeRxBuffer   DEV_NICDRV_MREF_MemFree

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void   L_MM_InsertTo_MonitorList(L_MM_Monitor_T *mm_p);
static void   L_MM_RemoveFrom_MonitorList(L_MM_Monitor_T *mm_p);
static BOOL_T L_MM_Valid_Buf(void *buff_p);
static void   L_MM_Mref_InitPktInfo(L_MM_Mref_T *mref_p);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T l_mm_sem_id;
static L_MM_Monitor_T* l_mm_monitor_lst_head =  NULL;  /* for monitor list */

/* static variables for debug/backdoor
 */
static UI32_T l_mm_backdoor_buff_nbr = 0;                /* record total number of the allocated buffers */
static BOOL_T l_mm_backdoor_dbg_msg_flag = FALSE;        /* l_mm debug message flag */
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
#if (L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER==TRUE)
static BOOL_T l_mm_backdoor_check_elapsed_time_flag = FALSE;
#endif

/* EXPORTED FUNCTION DECLARACTION
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Init
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
void L_MM_Init (void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &l_mm_sem_id) != SYSFUN_OK)
    {
        SYSFUN_LogUrgentMsg("L_MM_Init: Failed to init semaphore");
    }
} /* end of void L_MM_Init (void) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - L_MM_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void L_MM_Create_InterCSC_Relation(void)
{
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Malloc
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Allocate a buffer making use of malloc with L_MM_Monitor_T field prefixed
 *   to Application's Data Block internally
 *
 * INPUT:
 *  size    -- the size of the buffer to allocate for Application's Data Block
 *             (not including L_MM_Monitor_T)
 *  user_id -- used to monitor allocated buffer which is stored in L_MM_Monitor_T field
 *             user_id = trace_id(6) + pool_id(2) + module_id(8)
 *             pool_id is at user's own usage/knowledge.
 *             It can be used to stand for buffer being created by L_MM_PtCreate
 *             or L_MM_Malloc or 1st PtCreate and 2nd PtCreate...
 *             or it can be combined with trace_id to form a different trace information
 *             for user's own trace purpose.
 *             user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *             or L_MM_USER_ID2(modul_id,ext_trace_id)
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  address of the allocated data block for Application (pointer to Application's Data Block).
 *
 * NOTES:
 *   1. This function will use malloc to allocate N bytes where
 *      N = size + sizeof(L_MM_Monitor_T) + L_MM_TRAIL_MAGIC_BYTE_LEN
 *   2. Compared to L_MM_PtAlloc which is for fixed-sized buffer allocation from
 *      predefined partition, L_MM_Malloc is a varied-sized buffer allocation
 *   3. SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 *------------------------------------------------------------------------------
 */
void *L_MM_Malloc (UI32_T size, UI16_T user_id)
{
    L_MM_Monitor_T *mm_p;
    UI32_T real_alloc_sz = size + sizeof(L_MM_Monitor_T) + L_MM_TRAIL_MAGIC_BYTE_LEN;

    if ((mm_p =  (L_MM_Monitor_T*)malloc(real_alloc_sz)) == NULL)
    {
        L_MM_DEBUG_MSG("fails to allocate buffer");
        return NULL;
    }

    memset(mm_p, 0, sizeof(L_MM_Monitor_T));

    mm_p->buffer_type = L_MM_CONVERT_TO_INTERNAL_BUFTYPE(L_MM_DEFAULT_BUFFER_TYPE); /* default buffer_type */
    mm_p->buffer_addr = mm_p;
    mm_p->task_id = SYSFUN_TaskIdSelf();
    mm_p->allocation_time = SYSFUN_GetSysTick();
    mm_p->uid = user_id;
    mm_p->buf_len = real_alloc_sz;

    /* fill leading magic byte */
    L_MM_FILL_LEADING_MAGIC_BYTE(mm_p);
    /* fill trail magic byte */
    L_MM_FILL_TRAIL_MAGIC_BYTE(mm_p);
    /* insert the monitor information to the monitor link list */
    L_MM_InsertTo_MonitorList(mm_p);

    return (void*)((UI8_T*)mm_p + sizeof(L_MM_Monitor_T));
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Free
 *------------------------------------------------------------------------------
 * PURPOSE:
 *   Free the buffer which is allocated by L_MM_Malloc
 *
 * INPUT:
 *  buffer  -- pointer to Application's Data Block
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  None.
 *
 * NOTES:
 *   1. This function will free the buffer starts at the input buffer address - sizeof(L_MM_Monitor_T)
 *   2. SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 *------------------------------------------------------------------------------
 */
BOOL_T _L_MM_Free (void *buffer)
{
    L_MM_Monitor_T *mm_p;
    L_MM_DEBUG_GET_CURRENT_TICK();

    if ( buffer == NULL )
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        printf("%s():buffer is NULL\r\n", __FUNCTION__);
        return FALSE;
    }

    mm_p = (L_MM_Monitor_T*)((UI8_T*)buffer - sizeof(L_MM_Monitor_T));

    /* validate buffer */
    if(L_MM_Valid_Buf(buffer) == FALSE)
    {
       /* This is considered as critical issue.
        * Show error message directly here
        */
        printf("%s():invalid buffer\r\n", __FUNCTION__);
        return FALSE;
    }

    L_MM_DEBUG_DO_CHECK_FREED_BUFFER();

    L_MM_DEBUG_DO_CHECK_ELLAPSED_TIME_OF_MALLOC_BUFFER();

    /* remove from the monitor link list */
    L_MM_RemoveFrom_MonitorList(mm_p);

    free((void*)mm_p);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_AllocateTxBuffer
 *------------------------------------------------------------------------------
 * PURPOSE: This function is devised for transmission exclusively.
 *          In this function, the buffer will be allocated and a mref handle
 *          will be constructed. The pdu offset will be set as
 *          SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN to guarantee that lower
 *          level module can use this buffer to transmit directly.
 *          Callers should write data to buffer from pdu pointer which could be
 *          retrieved via L_MM_Mref_GetPdu().
 *
 * INPUT:
 *        tx_pdu_size           - The size of the buffer for pdu
 *        user_id               - used to monitor allocated buffer which is stored in L_MM_Monitor_T field
 *                                user_id = trace_id(6) + pool_id(2) + module_id(8)
 *                                pool_id is at user's own usage/knowledge.
 *                                It can be used to stand for buffer being created by L_MM_AllocateTxBuffer
 *                                or 1st L_MM_AllocateTxBuffer, 2nd L_MM_AllocateTxBuffer, etc.
 *                                Or it can be combined with trace_id to form a different trace information
 *                                for user's own trace purpose.
 *                                user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *                                or or L_MM_USER_ID2(modul_id,ext_trace_id)
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
 *------------------------------------------------------------------------------
 */
L_MM_Mref_Handle_T* _L_MM_AllocateTxBuffer(UI32_T tx_pdu_size, UI16_T user_id)
{
    I32_T              offset_to_mref_handle_p;
    L_MM_Mref_Handle_T* mref_handle_p;

    offset_to_mref_handle_p = (I32_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_ALLOCATE_TX_BUFFER), L_CVRT_UINT_TO_PTR(tx_pdu_size), L_CVRT_UINT_TO_PTR(user_id), NULL, NULL);

    if(offset_to_mref_handle_p < 0)
        mref_handle_p = NULL;
    else
        mref_handle_p = L_IPCMEM_GetPtr((UI32_T)offset_to_mref_handle_p);

    if (mref_handle_p)
        L_MM_Mref_InitPktInfo((L_MM_Mref_T *)mref_handle_p);

    return mref_handle_p;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_AllocateTxBufferForPktForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to allocate Tx Buffer for forwarding a received packet.
 *
 * INPUT:
 *        rcv_mref_handle_p     - L_MM_Mref of received packet
 *        tx_pdu_size           - The size of the buffer for pdu
 *        user_id               - used to monitor allocated buffer which is stored in L_MM_Monitor_T field
 *
 * OUTPUT:
 *          None
 *
 * RETURN:  The MREF handle for packet transmission
 *          NULL if fail.
 *
 * NOTES:
 *          see L_MM_AllocateTxBuffer for detail
 *------------------------------------------------------------------------------
 */
L_MM_Mref_Handle_T *_L_MM_AllocateTxBufferForPktForwarding(L_MM_Mref_Handle_T *rcv_mref_handle_p, UI32_T tx_pdu_size, UI16_T user_id)
{
    L_MM_Mref_Handle_T *mref_handle_p;

    mref_handle_p = L_MM_AllocateTxBuffer(tx_pdu_size, user_id);

    if (mref_handle_p == NULL)
    {
        return NULL;
    }

    if (rcv_mref_handle_p)
    {
        mref_handle_p->pkt_info = rcv_mref_handle_p->pkt_info;
    }

    return mref_handle_p;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_AllocateTxBufferFromDedicatedBufPool
 *------------------------------------------------------------------------------
 * PURPOSE: Allocate buffer for transimition from the specified buffer pool.
 *
 * INPUT:
 *          buffer_pool_id - buffer pool id where the buffer will be allocated
 *          user_id             - used to monitor allocated buffer which is stored in L_MM_Monitor_T field
 *                                user_id = trace_id(6) + pool_id(2) + module_id(8)
 *                                pool_id is at user's own usage/knowledge.
 *                                It can be used to stand for buffer being created by L_MM_AllocateTxBuffer
 *                                or 1st L_MM_AllocateTxBuffer, 2nd L_MM_AllocateTxBuffer, etc.
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
 *------------------------------------------------------------------------------
 */
L_MM_Mref_Handle_T *_L_MM_AllocateTxBufferFromDedicatedBufPool(UI32_T buffer_pool_id, UI32_T user_id)
{
    I32_T              offset_to_mref_handle_p;
    L_MM_Mref_Handle_T* mref_handle_p;

    offset_to_mref_handle_p = (I32_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_ALLOCATE_TX_BUFFER_FROM_DEDICATED_BUFFER), L_CVRT_UINT_TO_PTR(buffer_pool_id), L_CVRT_UINT_TO_PTR(user_id), NULL, NULL);
    if(offset_to_mref_handle_p < 0)
        mref_handle_p = NULL;
    else
        mref_handle_p = L_IPCMEM_GetPtr((UI32_T)offset_to_mref_handle_p);

    if (mref_handle_p)
        L_MM_Mref_InitPktInfo((L_MM_Mref_T *)mref_handle_p);

    return mref_handle_p;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_Construct
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
 *                         L_MM_MREF_FREE_FUN_CUSTOM    : user-specified free function. user must specify free function in 'cookie'.
 *        cookie         - cookie of free function (0 means no cookie)
 *                         if type is L_MM_MREF_FREE_FUN_CUSTOM, it indicates the free function.
 *
 * OUTPUT:
 *        None
 *
 * RETURN:  pointer to handle of L_MM_Mref if success
 *          or return NULL if fail
 *
 * NOTES:
 *      1. ref_count will be set to 1 in L_MM_Mref_Construct
 *      2. the buffer must be TX/RX buffer.
 *      3. When type is L_MM_MREF_FREE_FUN_CUSTOM, the given buf 
 *         must be able to referenced by the free function pointer.
 *         (The given buf and free function pointer must be valid in
 *          the context of the caller of L_MM_Mref_Release())
 *------------------------------------------------------------------------------
 */
L_MM_Mref_Handle_T*  _L_MM_Mref_Construct (void*               buf,
                                          UI16_T              buf_len,
                                          UI16_T              pdu_offset,
                                          UI16_T              pdu_len,
                                          UI16_T              type,
                                          void                *cookie,
                                          void                *cookie_params)
{
    L_MM_Mref_ConstructArg_T arg;
    I32_T               offset_to_mref_handle_p;
    L_MM_Mref_Handle_T* mref_handle_p;

    if(buf==NULL)
    {
        L_MM_DEBUG_MSG("invalid buf");
        return NULL;
    }

    if (type==L_MM_MREF_FREE_FUN_CUSTOM)
    {
         /* should not use IPCMEM offset if the buf is not in IPCMEM
          */
         arg.buf_p = buf;
    }
    else
    {
        arg.buf_offset = L_IPCMEM_GetOffset(buf);
        if(arg.buf_offset < 0)
        {
            L_MM_DEBUG_MSG("invalid ipcmem buf");
            return NULL;
        }

    }

    arg.buf_len = buf_len;
    arg.pdu_offset = pdu_offset;
    arg.pdu_len = pdu_len;
    arg.free_func_type = type;
    arg.cookie = cookie;
    arg.cookie_params = cookie_params;

    offset_to_mref_handle_p = (I32_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_CONSTRUCT), &arg, NULL, NULL, NULL);
    if (offset_to_mref_handle_p < 0)
        return NULL;

    mref_handle_p = L_IPCMEM_GetPtr(offset_to_mref_handle_p);

    L_MM_Mref_InitPktInfo((L_MM_Mref_T *)mref_handle_p);

    return mref_handle_p;
} /* end of L_MM_Mref_Construct() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_AddRefCount
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
 *   SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 *------------------------------------------------------------------------------
 */
BOOL_T _L_MM_Mref_AddRefCount (L_MM_Mref_Handle_T* mref_handle_p, UI32_T inc_count)
{
    I32_T               offset_to_mref_handle_p;

    if(mref_handle_p==NULL)
        return FALSE;

    offset_to_mref_handle_p = L_IPCMEM_GetOffset(mref_handle_p);
    if(offset_to_mref_handle_p<0)
        return FALSE;

    return (BOOL_T)(SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_ADD_REF_COUNT), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), L_CVRT_UINT_TO_PTR(inc_count), NULL, NULL));
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_SetFreeFuncTypeAndCookie
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
 *                     L_MM_MREF_FREE_FUN_CUSTOM    : user-specified free function.
 *  cookie          -- cookie of free function (0 means no cookie)
 *                     if type is L_MM_MREF_FREE_FUN_CUSTOM, it indicates the free function.
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
 *  3. SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 *------------------------------------------------------------------------------
 */
BOOL_T _L_MM_Mref_SetFreeFuncTypeAndCookie( L_MM_Mref_Handle_T* mref_handle_p, UI32_T free_fun_type, void *cookie)
{
    I32_T               offset_to_mref_handle_p;

    if(mref_handle_p==NULL)
        return FALSE;

    offset_to_mref_handle_p = L_IPCMEM_GetOffset(mref_handle_p);
    if(offset_to_mref_handle_p<0)
        return FALSE;

    return (BOOL_T)(SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_SET_FREE_FUNC_TYPE_AND_COOKIE), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), L_CVRT_UINT_TO_PTR(free_fun_type), cookie, NULL));
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_MovePdu
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
 *   SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 *------------------------------------------------------------------------------
 */
void* _L_MM_Mref_MovePdu(L_MM_Mref_Handle_T* mref_handle_p, I32_T offset, UI32_T *pdu_len_p)
{
    I32_T               offset_to_mref_handle_p;
    I32_T               offset_to_new_pdu_p;
    void*               new_pdu_p;

    if(mref_handle_p==NULL)
        return NULL;

    offset_to_mref_handle_p = L_IPCMEM_GetOffset(mref_handle_p);
    if(offset_to_mref_handle_p<0)
        return NULL;

    offset_to_new_pdu_p = (I32_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_MOVE_PDU), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), L_CVRT_UINT_TO_PTR(offset), pdu_len_p, NULL);
    if((((L_MM_Mref_T*)mref_handle_p)->free_func_type)!=L_MM_MREF_FREE_FUN_CUSTOM)
    {
        if(offset_to_new_pdu_p < 0)
            new_pdu_p = NULL;
        else
            new_pdu_p = L_IPCMEM_GetPtr((UI32_T)offset_to_new_pdu_p);
    }
    else
        new_pdu_p = L_CVRT_GET_PTR(((L_MM_Mref_T*)mref_handle_p)->buffer_addr, offset_to_new_pdu_p);

    return new_pdu_p;
} /* end of L_MM_Mref_MovePdu */

/*------------------------------------------------------------------------------
 * ROUTINE NAME - L_MM_Mref_ShiftPdu
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
BOOL_T _L_MM_Mref_ShiftPdu(L_MM_Mref_Handle_T* mref_handle_p, UI32_T shift_offset)
{
    I32_T               offset_to_mref_handle_p;

    if(mref_handle_p==NULL)
        return FALSE;

    offset_to_mref_handle_p = L_IPCMEM_GetOffset(mref_handle_p);
    if(offset_to_mref_handle_p<0)
        return FALSE;

    return (BOOL_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_SHIFT_PDU), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), L_CVRT_UINT_TO_PTR(shift_offset), NULL, NULL);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - L_MM_Mref_SetPduLen
 *------------------------------------------------------------------------------
 * PURPOSE:  Set the length of pdu.
 *
 * INPUT:    mref_handle_p  -  pointer to handle of L_MM_Mref
 *           pdu_len        -  length of pdu
 *
 * OUTPUT:   None.
 *
 * RETURN:   TRUE  -- Sucess
 *           FALSE -- Fail
 *
 * NOTE:
 *    For IUC client STKTPLG, it will use IUC to send packet with size 18,
 *    plus the two header in front of it as follows:
 *               XGS ether header, length = 18
 *               IUC header, length       = 8
 *    (the payload size 18 will store in IUC header)
 *
 *    The total size will be 18+18+8 = 44, but the minimal packet size is 64,
 *    so padding of size 20 will be add before send by chip driver.
 *    At the remote unit, this packet will be receive with size 64.
 *
 *    In remote IUC, its pdu will move 18+8=26 bytes and dispatch to STKTPLG,
 *    but this will cause problem because the payload become 64-26 = 38 bytes
 *    instead of 18 which it originally be. Thus the pdu length should be set
 *    as 18 before passing mref to STKTPLG.
 *------------------------------------------------------------------------------
 */
BOOL_T _L_MM_Mref_SetPduLen(L_MM_Mref_Handle_T* mref_handle_p, UI32_T pdu_len)
{
    L_MM_Mref_T *mref_p;

    if( (mref_p=(L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_DEBUG_MSG("mref_handle_p is NULL.");
        return FALSE;
    }

    /* check whether pdu_len falls in legal range
     */
    if(mref_p->pdu_offset + pdu_len > mref_p->buf_len)
    {
        L_MM_DEBUG_MSG("pdu_len is beyond legal range.");
        return FALSE;
    }

    mref_p->pdu_len = pdu_len;
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_GetPdu
 *------------------------------------------------------------------------------
 * PURPOSE: get PDU address and PDU length
 *
 * INPUT:
 *  mref_handle_p  - pointer to handle of L_MM_Mref
 *
 * OUTPUT:
 *  pdu_len - length of pdu
 *
 * RETURN: pdu address or NULL if fail
 *
 * NOTES:
 *  1. When MREF error detect flag is on and a MREF descriptor corrupted is
 *     detected, the caller address and caller task id which calls this function
 *     previous time will be logged to flash through "/etc/mref_log.sh".
 *------------------------------------------------------------------------------
 */
void  *_L_MM_Mref_GetPdu (L_MM_Mref_Handle_T* mref_handle_p, UI32_T *pdu_len)
{
    I32_T               offset_to_mref_handle_p;
    uintptr_t           previous_caller_addr=0;
    UI32_T              previous_caller_tid=0;
    L_MM_Mref_T *mref_p;
    void *buffer_addr;

    if( (mref_p=(L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_DEBUG_MSG("mref_handle_p is NULL.");
        return NULL;
    }

    *pdu_len= mref_p->pdu_len;

    if(mref_p->free_func_type != L_MM_MREF_FREE_FUN_CUSTOM)
    {
        offset_to_mref_handle_p = L_IPCMEM_GetOffset(mref_handle_p);
        if(offset_to_mref_handle_p<0)
            return FALSE;

        buffer_addr = L_IPCMEM_GetPtr(SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_GET_PDU), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), __builtin_return_address(0), &previous_caller_addr, &previous_caller_tid));

        if (previous_caller_addr!=0)
        {
            char cmdbuf[EXECUTE_MREF_LOG_SYSTEM_SHELL_BUF_LEN];

            /* When previous_caller_addr is not 0, it means MREF descriptor error
             * is detected.
             */
            snprintf(cmdbuf, sizeof(cmdbuf), "/etc/mref_err_log.sh %08lX %lu", previous_caller_addr, (unsigned long)previous_caller_tid);
            cmdbuf[EXECUTE_MREF_LOG_SYSTEM_SHELL_BUF_LEN-1]=0;
            SYSFUN_ExecuteSystemShell(cmdbuf);
        }

        /* validate buffer */
        if(L_MM_Valid_Buf((void*)mref_p)==FALSE)
        {
            L_MM_DEBUG_MSG("invalid mref_handle_p");
            return NULL;
        }
        return ((UI8_T*)(buffer_addr) + mref_p->pdu_offset);
    }
    return ((UI8_T*)(mref_p->buffer_addr) + mref_p->pdu_offset);
} /* end of L_MM_Mref_GetPdu */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - L_MM_Mref_GetAvailSzBeforePdu
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the available buffer size before current pdu
 * INPUT:    mref_handle_p  -  pointer to handle of L_MM_Mref
 * OUTPUT:   None.
 * RETURN:   Available size(byte count) before pdu
 *           Or return L_MM_ERROR_RETURN_VAL if fail
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T _L_MM_Mref_GetAvailSzBeforePdu(L_MM_Mref_Handle_T* mref_handle_p)
{
    L_MM_Mref_T *mref_p;

    if( (mref_p=(L_MM_Mref_T*)mref_handle_p) == NULL)
    {
        L_MM_DEBUG_MSG("mref_handle_p is NULL.");
        return L_MM_ERROR_RETURN_VAL;
    }

    return mref_p->pdu_offset;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_Release
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
 *   1.SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 *   2.mref_handle_pp will be set as NULL when reference count equals zero
 *     after this function is returned.
 *------------------------------------------------------------------------------
 */
UI32_T  _L_MM_Mref_Release (L_MM_Mref_Handle_T** mref_handle_pp)
{
    I32_T               offset_to_mref_handle_p;
    UI32_T              ref_count;
    L_MM_Mref_T         *mref_p;
    UI16_T              free_func_type;
    void                *buffer_addr, *cookie, *cookie_params;

    if(mref_handle_pp==NULL)
        return L_MM_ERROR_RETURN_VAL;

    if (*mref_handle_pp==NULL)
        return L_MM_ERROR_RETURN_VAL;

    offset_to_mref_handle_p = L_IPCMEM_GetOffset(*mref_handle_pp);
    if(offset_to_mref_handle_p<0)
        return L_MM_ERROR_RETURN_VAL;

    mref_p = (L_MM_Mref_T *)*mref_handle_pp;
    free_func_type = mref_p->free_func_type;

    /* If buffer is for rx,
     * store buffer_addr and cookie for release.
     */
    buffer_addr = cookie = NULL;
    if (free_func_type != L_MM_MREF_FREE_FUN_TX_BUFFER)
    {
        if(free_func_type == L_MM_MREF_FREE_FUN_CUSTOM)
            buffer_addr = mref_p->buffer_addr;
        else
            buffer_addr = L_IPCMEM_GetPtr(SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_GET_BUFFER_OFFSET), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), NULL, NULL, NULL));

        cookie = mref_p->cookie;
        cookie_params = mref_p->cookie_params;

    }

    ref_count = SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, L_CVRT_UINT_TO_PTR(L_MM_CMD_RELEASE), L_CVRT_UINT_TO_PTR(offset_to_mref_handle_p), NULL, NULL, NULL);

    if ( ref_count < 0)
    {
        BACKDOOR_MGR_Printf("Double free!! offset: %u, ref_count: %u\r\n", offset_to_mref_handle_p, ref_count);
	}

    if(ref_count==0)
    {
        *mref_handle_pp=NULL;

        if (free_func_type == L_MM_MREF_FREE_FUN_RX_BUFFER)
        {
            /* Turn off gcc warning here because the cookie is indeed UI32_T
             * when calling L_MM_FreeRxBuffer
             */
            SYSFUN_GCC_DIAG_OFF(pointer-to-int-cast);
            L_MM_FreeRxBuffer(buffer_addr, (UI32_T)cookie, cookie_params);
            SYSFUN_GCC_DIAG_ON(pointer-to-int-cast);
        }
        else if (free_func_type == L_MM_MREF_FREE_FUN_CUSTOM)
        {
            if(NULL == cookie_params)
                ((void (*)())cookie)(buffer_addr);
            else
                ((void (*)())cookie)(cookie_params);
        }
    }

    return ref_count;
}

/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME - L_MM_InsertTo_MonitorList
 *------------------------------------------------------------------------------
 * PURPOSE:  Insert a monitor node into monitor list
 * INPUT:    mm_p  -  pointer to monitor node to be inserted
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     1. Caller of this function must guarntee that the input argument is
 *              valid.
 *------------------------------------------------------------------------------
 */
static void L_MM_InsertTo_MonitorList(L_MM_Monitor_T *mm_p)
{
    L_MM_ENTER_CRITICAL_SECTION();

    mm_p->next_p = l_mm_monitor_lst_head;
    mm_p->prev_p = NULL;

    if  (l_mm_monitor_lst_head != NULL)
    {
        l_mm_monitor_lst_head->prev_p = mm_p;
    }
    l_mm_monitor_lst_head = mm_p;
    l_mm_backdoor_buff_nbr++;

    L_MM_LEAVE_CRITICAL_SECTION();
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
static void L_MM_RemoveFrom_MonitorList(L_MM_Monitor_T *mm_p)
{
    L_MM_ENTER_CRITICAL_SECTION();

    mm_p->buffer_addr = NULL; /* invalidate buffer_addr to avoid free the same mm_p twice */

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

    L_MM_LEAVE_CRITICAL_SECTION();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - L_MM_Valid_Buf
 *------------------------------------------------------------------------------
 * PURPOSE:  For validating buffer prefixed with L_MM_Monitor_T and suffixed with
 *           trail magic bytes.
 * INPUT:    buff_p  -  pointer to buffer being validated
 * OUTPUT:   None.
 * RETURN:   TRUE  -- Valid buffer
 *           FALSE -- Invalid buffer
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static BOOL_T L_MM_Valid_Buf(void *buff_p)
{
    L_MM_Monitor_T *mm_p;
    BOOL_T is_valid_magic_bytes;
    BOOL_T ret=TRUE;

    mm_p = (L_MM_Monitor_T*)((UI8_T*)buff_p - sizeof(L_MM_Monitor_T));

    /* check buffer address
     * buffer_addr with buffer type equal to L_MM_MREF_BUFFER_TYPE
     * contains kernel space addr, there is no way to check this addr in user
     * space
     */
    if (L_MM_GET_INTERNAL_BUFTYPE(mm_p->buffer_type) != L_MM_MREF_BUFFER_TYPE &&
        mm_p->buffer_addr != mm_p)
    {
         L_MM_DEBUG_MSG("buffer validation fails through buffer_addr check");
         ret = FALSE;
    }

    /* check leading magic byte */
    L_MM_IS_VALID_LEADING_MAIC_BYTES(&is_valid_magic_bytes, mm_p);
    if(is_valid_magic_bytes==FALSE)
    {
        L_MM_DEBUG_MSG("buffer validation fails through leading magic byte check");
        printf("%s: TaskId: %d   buff_p: 0x%p\r\n", __FUNCTION__, (int)SYSFUN_TaskIdSelf(), buff_p);
        DBG_DumpHex("", sizeof(*mm_p), (char *)mm_p);
        ret = FALSE;
    }

    /* check trail magic byte */
    L_MM_IS_VALID_TRAIL_MAGIC_BYTES(&is_valid_magic_bytes, mm_p);
    if (is_valid_magic_bytes==FALSE)
    {
        L_MM_DEBUG_MSG("buffer validation fails through trail magic byte check");
        ret = FALSE;
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_Mref_InitPktInfo
 *------------------------------------------------------------------------------
 * PURPOSE  : This function is used for pkt_info initialize
 * INPUT    : mref_p
 * OUTPUT   : mref_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static void L_MM_Mref_InitPktInfo(L_MM_Mref_T *mref_p)
{
    L_MM_Mref_PktInfo_T *pkt_info_p = &((L_MM_Mref_Handle_T *)mref_p)->pkt_info;

    memset(pkt_info_p, 0, sizeof(*pkt_info_p));
}

#if (BACKDOOR_OPEN==TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_CheckFreedBuf
 *------------------------------------------------------------------------------
 * PURPOSE  : This function is used to check whether the given buffer is allowed
 *            to be freed. If one of the address of the buffer in monitor list
 *            falls in the given buffer region, the given buffer can't be freed
 *            in order to prevent monitor list from corruption.
 *
 * INPUT    : mm_p  -  The monitor header of the buffer which has internal buffer
 *                     type L_MM_PT_BUFFER_TYPE
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *            1.This function will only be called by L_MM_Free when
 *              l_mm_backdoor_validate_free_flag is TRUE.
 *            2.l_mm_backdoor_validate_free_flag can only be set through BACKDOOR.
 *            3.This function is primary for debug.
 *            4.Caller of this function should guarantee the input argument is
 *              valid.
 *-----------------------------------------------------------------------------*/
static BOOL_T L_MM_CheckFreedBuf(L_MM_Monitor_T* mm_p)
{
    UI8_T          *part_front_p, *part_rear_p;
    L_MM_Monitor_T *monitor_node;

    part_front_p = (UI8_T*)mm_p + sizeof(L_MM_Monitor_T);
    part_rear_p  = (UI8_T*)mm_p + mm_p->buf_len - L_MM_TRAIL_MAGIC_BYTE_LEN;

    /* check that is there any address of node in monitor list falls in
     * this buffer range
     */
    monitor_node = l_mm_monitor_lst_head;
    do
    {
        if(((UI8_T*)monitor_node>=part_front_p) &&
           ((UI8_T*)monitor_node< part_rear_p))
        {
            BACKDOOR_MGR_Print("Error: An allocated buffer fall in the region of the buffer to be freed\r\n");
            L_MM_DEBUG_DUMP_MONITOR_INFO(BACKDOOR_MGR_Printf, monitor_node);
            return FALSE;
        }
        else
        {
            monitor_node = monitor_node->next_p;
        }
    }while(monitor_node!=NULL);

    return TRUE;
} /* end of L_MM_CheckFreedBuf */
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_GetBufferInfo
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
 * buffer_info_p->buffer_addr              --  the address of the buffer
 *
 * RETURN:
 *  TRUE  --  An entry is retrieved sucessfully.
 *  FALSE --  No more entry can be retrieved.
 *
 * NOTES:
 *
 */
BOOL_T L_MM_GetBufferInfo(L_MM_Backdoor_BufferInfo_T *buffer_info_p)
{
    L_MM_Monitor_T *monitor_p;

    if(buffer_info_p->last_ref_monitor_p==NULL)
        monitor_p = l_mm_monitor_lst_head;
    else
        monitor_p = ((L_MM_Monitor_T*)(buffer_info_p->last_ref_monitor_p))->next_p;

    if(monitor_p==NULL)
        return FALSE;

    buffer_info_p->last_ref_monitor_p = monitor_p;
    
    L_MM_ENTER_CRITICAL_SECTION(); 
    /* check whether the monitor_p is still valid(i.e.has not been freed)
     */
    if(monitor_p->buffer_addr==NULL)
    {
        L_MM_LEAVE_CRITICAL_SECTION();
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
    L_MM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_GetTotalNBofAllocatedBuffer 
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get total number of allocated buffer.
 *------------------------------------------------------------------------------
 * INPUT:None
 * OUTPUT:None
 * RETURN:
 * l_mm_backdoor_buff_nbr
 * NOTES:
 *
 */
UI32_T L_MM_GetTotalNBofAllocatedBuffer(void)
{
    return l_mm_backdoor_buff_nbr;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_ToggleDebugMsgFlag
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Toggle Debug Message Flag.
 *------------------------------------------------------------------------------
 * INPUT:None
 * OUTPUT:None
 * RETURN:
 * l_mm_backdoor_dbg_msg_flag
 * NOTES:
 *
 */
BOOL_T L_MM_ToggleDebugMsgFlag(void)
{
    l_mm_backdoor_dbg_msg_flag = !l_mm_backdoor_dbg_msg_flag;
    return l_mm_backdoor_dbg_msg_flag;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_ToggleValidateFreeFlag
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Toggle Validate Free Flag.
 *------------------------------------------------------------------------------
 * INPUT:None
 * OUTPUT:None
 * RETURN:
 * l_mm_backdoor_validate_free_flag
 * NOTES:
 *
 */
BOOL_T L_MM_ToggleValidateFreeFlag(void)
{
    l_mm_backdoor_validate_free_flag = !l_mm_backdoor_validate_free_flag;
    return l_mm_backdoor_validate_free_flag;
}
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_ToggleCheckElapsedTimeFlag
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Toggle Check Elapsed Time Flag.
 *------------------------------------------------------------------------------
 * INPUT:None
 * OUTPUT:None
 * RETURN:None
 * NOTES:
 *
 */
void L_MM_ToggleCheckElapsedTimeFlag(void)
{
    L_MM_DEBUG_TOGGLE_FLAG_OF_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_DebugPrintf
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Display debug message to console
 *------------------------------------------------------------------------------
 * INPUT:
 *  msg     -- display message string (can't be NULL)
 *  func    -- function name (can be NULL)
 *  line    -- line number
 * OUTPUT:None
 * RETURN:None
 * NOTES:
 *  The message will only display when l_mm_backdoor_dbg_msg_flag == TRUE
 */
void L_MM_DebugPrintf(const char *msg, const char *func, I32_T line)
{
    if (msg == NULL)
        return;

    if (l_mm_backdoor_dbg_msg_flag == TRUE)
    {
        if (func)
            BACKDOOR_MGR_Printf("\r\n%s() line %d: %s\r\n",func,line,msg);
        else
            BACKDOOR_MGR_Printf("\r\n%s\r\n",msg);
    }
}

