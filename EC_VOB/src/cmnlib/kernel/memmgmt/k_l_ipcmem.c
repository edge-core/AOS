/* Module Name: K_L_IPCMEM.C
 * Purpose:
 *
 *  L_IPCMEM provides a pool for whole system allocated to prevent memory fragment,
 *
 * Notes:
 *  In developing stage we need to know the peak amount be used. We define _IN_TARGET
 *  macro to include debugging environment, UC_MGR in Mercury.
 *
 * History:
 *       Date       -- Modifier,        Reason
 *  1.1 2001.12.19  -- William Chiang,  Add peak counter to track the maximun amount
 *                                      used in run time.
 *  1.2 2002.01.17  -- William Chiang,  Modify the amount of blocks in memory pool.
 *                                      Define _IN_TARGET for keeping tracking information
 *                                      in Un-clear Memory, not complete yet, so not recorded in
 *                                      unclear memory.
 *  1.3 2002.03.07  -- William Chiang,  Add BACKDOOR_OPEN mechanism.
 *  1.4 2004.04.21  -- Penny Chang,     Enlarge size 1600 to size 1800
 *  1.5 2005.10.11  -- Chralie Chen,    Use L_MM_Pt to partition buffer pool and
 *                                      do partition allocation and free.
 *  1.6 2006.10.17  -- Charlie Chen,    Port to linux
 *  2.0 2008.07.03  -- Tiger Liu,       remove unused function, rename functions L_IPCMEM_XXX to K_L_IPCMEM_XXX.
 * Copyright(C)      Accton Corporation, 1999-2006
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef _DEBUG
#include <malloc.h>
#include <memory.h>
#include <string.h>
#endif
/* Linux specific include files
 */
#include <linux/mm.h> /* for high_memory */
#include <asm/page.h> /* for __pa() */
#include <asm/io.h> /* for ioremap(), phys_to_virt() */
#include <linux/linkage.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

#include <linux/version.h>
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(3,2,65) )
    #include <linux/uaccess.h>
#endif

/* accton include files
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "l_cvrt.h"
#include "k_sysfun.h"
#include "k_l_mm.h"
#include "k_l_ipcmem.h"
#include "l_ipcmem_type.h"

#ifdef MARVELL_CPSS
#include <asm/arch/pssBspApisInline.h>
#endif

#ifdef  BACKDOOR_OPEN
//#include "backdoor_mgr.h"
#endif

/* include constant declarations for rx buffer.
 */
#include "dev_nicdrv_rx_buf_type.h"

/* include constant declarations for dedicated buffer.
 */
#include "stktplg_tx_type.h"
#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_txrx_type.h"
#endif

#if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE)
extern void* kmalloc_giant(int sz);
void kfree_giant(void* ptr);
extern int lkbde_get_dma_info(uint32_t *pbase, uint32_t *size);
extern int lkbde_get_dma_vbase(void **vbase);
#endif
#ifdef MARVELL_CPSS
void *bspCacheDmaMalloc(size_t bytes);
unsigned int bspCacheDmaFree(void *pBuf);
#endif

/* NAMING CONSTANT DEFINITION
 */

#define   BACKDOOR_OPEN          /* interact with backdoor function of L_IPCMEM
                                 * if no need to inact with L_IPCMEM, do not define BACKDOOR_OPEN
                                 */
#define  _IN_TARGET      1      /* active tracking counter of each memory pool, this
                                 * mechanism waste some calculation time.
                                 * 1 : active, 0 : inactive.
                                 */
#define  _TMZ            1      /* the space to separate over range */

/* Buffer Number definition */
#define ADPT_NUMBER_OF_BUF_64       SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_64
#define ADPT_NUMBER_OF_BUF_128      SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_128
#define ADPT_NUMBER_OF_BUF_256      SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_256
#define ADPT_NUMBER_OF_BUF_512      SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_512
#define ADPT_NUMBER_OF_BUF_1800     SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1800

#define L_IPCMEM_NUMBER_OF_BUF_TYPE_LOCAL 5

#if L_IPCMEM_NUMBER_OF_BUF_TYPE != L_IPCMEM_NUMBER_OF_BUF_TYPE_LOCAL
#error "L_IPCMEM_NUMBER_OF_BUF_TYPE defined in l_ipcmem_type.h doesn't match L_IPCMEM_NUMBER_OF_BUF_TYPE_LOCAL"
#endif

#define L_IPCMEM_BUF_TYPE_64        0
#define L_IPCMEM_BUF_TYPE_128       1
#define L_IPCMEM_BUF_TYPE_256       2
#define L_IPCMEM_BUF_TYPE_512       3
#define L_IPCMEM_BUF_TYPE_1800      4

/* Rx buffer size */
#define L_IPCMEM_SIZE_OF_RX_BUFFER  DEV_NICDRV_TOTAL_RESERVED_RX_BUFFER_SIZE

/* Dedicated buffer pool.
 * NOTE: the size doesn't include SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN
 */
#define L_IPCMEM_SIZE_OF_DEDICATED_BUF_STKTPLG_HBT1     STKTPLG_TX_TYPE_HBT1_DEDICATED_BUFFER_POOL_PARTITION_SIZE
#define L_IPCMEM_NUMBER_OF_DEDICATED_BUF_STKTPLG_HBT1   STKTPLG_TX_TYPE_HBT1_DEDICATED_BUFFER_POOL_PARTITION_NUM

#if (SYS_CPNT_DHCP == TRUE)
#define L_IPCMEM_SIZE_OF_DEDICATED_BUF_DHCP_TX          DHCP_TXRX_TYPE_TX_DEDICATED_BUFFER_POOL_PARTITION_SIZE
#define L_IPCMEM_NUMBER_OF_DEDICATED_BUF_DHCP_TX        DHCP_TXRX_TYPE_TX_DEDICATED_BUFFER_POOL_PARTITION_NUM
#endif

#define L_IPCMEM_NUMBER_OF_DEDICATED_BUF_TYPE  (sizeof(dedicated_buf_pool_descs)/sizeof(L_MM_PtDesc_T))

/* MACRO FUNCTION DECLARATIONS
 */
#define L_IPCMEM_ENTER_CRITICAL_SECTION SYSFUN_EnterCriticalSection
#define L_IPCMEM_LEAVE_CRITICAL_SECTION SYSFUN_LeaveCriticalSection

/* DATA TYPE DECLARATIONS
 */

#if _IN_TARGET
typedef struct L_IPCMEM_COUNTER_REC_S
{
    UI32_T    peak_left_amount; /* keep the lowest value of left blocks */
    UI32_T    alloc_counter;
    UI32_T    alloc_fail_counter;
    UI32_T    free_counter;
} L_IPCMEM_COUNTER_REC_T;
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T K_L_IPCMEM_Operation(void* arg0, void* arg1, void* arg2, void* arg3, void* arg4);
static void K_L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo(void *k_virtual_base_address, void *k_physical_base_address);

#ifdef  BACKDOOR_OPEN
static void K_L_IPCMEM_GetBackdoorInfo(L_Mem_BackdoorInfoEntry_T backdoor_info[]);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if     _TMZ
char    l_mem_tmz_front[MEM_TMZ_SIZE];
#endif

static void   *k_mem_buffer_physical_base_addr = NULL; /* physical address of memory owned by L_IPCMEM */
/* k_mem_buffer_size:
 *     On Broadcom: total dma buffer size
 *     On Marvell : buffer size for tx and mref
 */
static UI32_T  k_mem_buffer_size               = 0;
static void   *k_mem_buffer_base_addr          = NULL; /* virtual address for ptr/offset translation */
static void   *k_mem_buffer_end_addr           = NULL; /* virtual address of end of tx buf memory */
static void   *k_dedicated_buffer_start_addr   = NULL; /* virtual address of starting dedicated tx buf memory(i.e. dedicated_buf_pool_descs) */
static void   *k_mem_buffer_real_start_addr    = NULL; /* virtual address of starting memory owned by L_IPCMEM */

/* The major use of L_IPCMEM is to allocate buffer for packet transmission.
 * The original 64-byte is enlarged by SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN
 * to ensure that the user of L_IPCMEM that needs to transmit less-than-64-byte
 * packet can use this buffer type
 */
static L_MM_PtDesc_T buf_pool_descs[] =
{
    {NULL, 0, 64+SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN, ADPT_NUMBER_OF_BUF_64,   NULL, L_IPCMEM_BUF_TYPE_64},
    {NULL, 0, 128,                                           ADPT_NUMBER_OF_BUF_128,  NULL, L_IPCMEM_BUF_TYPE_128},
    {NULL, 0, 256,                                           ADPT_NUMBER_OF_BUF_256,  NULL, L_IPCMEM_BUF_TYPE_256},
    {NULL, 0, 600,                                           ADPT_NUMBER_OF_BUF_512,  NULL, L_IPCMEM_BUF_TYPE_512},
    {NULL, 0, 1800,                                          ADPT_NUMBER_OF_BUF_1800, NULL, L_IPCMEM_BUF_TYPE_1800}
};

static const UI32_T num_of_block_for_each_buf_type[L_IPCMEM_NUMBER_OF_BUF_TYPE] =
{
    ADPT_NUMBER_OF_BUF_64,
    ADPT_NUMBER_OF_BUF_128,
    ADPT_NUMBER_OF_BUF_256,
    ADPT_NUMBER_OF_BUF_512,
    ADPT_NUMBER_OF_BUF_1800
};

/* The dedicated buffer pool.
 */
static L_MM_PtDesc_T dedicated_buf_pool_descs[] =
{
    {NULL, 0, L_IPCMEM_SIZE_OF_DEDICATED_BUF_STKTPLG_HBT1+SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN, 0, NULL, L_IPCMEM_DEDICATED_BUF_TYPE_STKTPLG_HBT1},
#if (SYS_CPNT_DHCP == TRUE)
    {NULL, 0, L_IPCMEM_SIZE_OF_DEDICATED_BUF_DHCP_TX+SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN,      0, NULL, L_IPCMEM_DEDICATED_BUF_TYPE_DHCP_TX},
#endif
};

static const UI32_T num_of_block_for_each_dedicated_buf_type[L_IPCMEM_NUMBER_OF_DEDICATED_BUF_TYPE] =
{
    L_IPCMEM_NUMBER_OF_DEDICATED_BUF_STKTPLG_HBT1,
#if (SYS_CPNT_DHCP == TRUE)
    L_IPCMEM_NUMBER_OF_DEDICATED_BUF_DHCP_TX,
#endif
};

#if _IN_TARGET
static  L_IPCMEM_COUNTER_REC_T  access_counter[L_IPCMEM_NUMBER_OF_BUF_TYPE];
static  UI32_T                  allocate_fail_counter=0;
#endif

#if     _TMZ
static  char    l_mem_tmz_rear[MEM_TMZ_SIZE];
#endif

static UI32_T l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_MAX];
static UI32_T l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_MAX];
static UI32_T total_size_of_buf_pool = 0;

#if L_IPCMEM_DEBUG
static int k_l_ipcmem_initialize_call_counter = 1;
#endif

static DEFINE_SPINLOCK(ipcmem_lock);

/* EXPORTED FUNCTIONS BODY
 */
void K_L_IPCMEM_Initialize(void)
{
   /* LOCAL VARIABLES
    */
    UI32_T i;
    UI8_T* buf_p;

    if((sizeof(buf_pool_descs)/sizeof(L_MM_PtDesc_T))!=L_IPCMEM_NUMBER_OF_BUF_TYPE_LOCAL)
    {
        panic("The number of buffer type mismatches between user and kernel space.");
    }

    K_L_IPCMEM_DEBUG_PRINT("%s(): be called %d times.\n", __FUNCTION__, k_l_ipcmem_initialize_call_counter++);

    /* BODY
     */
#if     _TMZ
    for (i = 0; i < MEM_TMZ_SIZE; i++)
    {
        l_mem_tmz_rear[i]  = MEM_TMZ_PATTERN;
        l_mem_tmz_front[i] = MEM_TMZ_PATTERN;
    }
#endif

    /* calculate total required size for TX buffer pool
     */
    l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_TX] = 0;

    for(i = 0; i < L_IPCMEM_NUMBER_OF_BUF_TYPE; i++)
    {
        buf_pool_descs[i].buffer_len = K_L_MM_PtGetTotalRequiredSize(buf_pool_descs[i].partition_len,
                                                                   num_of_block_for_each_buf_type[i]);
        l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_TX] += buf_pool_descs[i].buffer_len;
    }

    for(i = 0; i < L_IPCMEM_NUMBER_OF_DEDICATED_BUF_TYPE; i++)
    {
        //dedicated_buf_pool_descs[i].partition_len += SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN;
        dedicated_buf_pool_descs[i].buffer_len = K_L_MM_PtGetTotalRequiredSize(dedicated_buf_pool_descs[i].partition_len,
                                                                             num_of_block_for_each_dedicated_buf_type[i]);
        l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_TX] += dedicated_buf_pool_descs[i].buffer_len;
    }

    /* calculate required size for each type of buffer.
     */
#ifdef MARVELL_CPSS
    l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_RX] = 2 * SYS_TYPE_1M_BYTES;
#else
    l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_RX] = L_IPCMEM_SIZE_OF_RX_BUFFER;
#endif
    l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_MREF] = K_L_MM_Mref_GetMrefPoolRequiredSize();
    /*  l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_UC] = SYS_HWCFG_UC_MEM_SIZE; */

    /* calculate total required size
     */
    total_size_of_buf_pool = 0;
    for (i = 0; i < L_IPCMEM_MEM_ID_MAX; i++)
    {
        #ifdef MARVELL_CPSS
        if (i==L_IPCMEM_MEM_ID_RX)
        {
            /* total_size_of_buf_pool should only count the memory size managed
             * by IPCMEM in Marvell solution
             * L_IPCMEM_MEM_ID_RX is managed by Marvell SDK,
             * do not count it in
             */
            continue;
        }
        #endif

        total_size_of_buf_pool += l_ipcmem_mem_size_ar[i];
        printk(KERN_INFO "%s: mem%d: size:0x%08x\n", __FUNCTION__, (int)i, (unsigned int)l_ipcmem_mem_size_ar[i]);
    }
    printk(KERN_INFO "%s: total_size_of_buf_pool:0x%08x\n", __FUNCTION__, (unsigned int)total_size_of_buf_pool);
#ifndef MARVELL_CPSS
    /* k_mem_buffer_real_start_addr might not be algined to the page boundary
     * reserve one more page so that the k_mem_buffer_real_start_addr can be
     * moved to a page aligned address
     */
    total_size_of_buf_pool = L_ALIGN(total_size_of_buf_pool, PAGE_SIZE) + PAGE_SIZE;
    printk(KERN_INFO "%s: total_size_of_buf_pool(aligned):0x%08x\n", __FUNCTION__, (unsigned int)total_size_of_buf_pool);
#endif

    /* get start address of himem
     */
#if defined(BROADCOM)
    #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE)
    if (k_mem_buffer_physical_base_addr == NULL)
    {
        uint32_t pbase, dmasize;
        void *vbase;

        if (lkbde_get_dma_info(&pbase, &dmasize) != 0)
        {
            printk(KERN_EMERG "%s: lkbde_get_dma_info() failed.\n", __FUNCTION__);
            return;
        }

        if (lkbde_get_dma_vbase(&vbase) != 0)
        {
            printk(KERN_EMERG "%s: lkbde_get_dma_vbase() failed.\n", __FUNCTION__);
            return;
        }

        K_L_IPCMEM_DEBUG_PRINT("%s: vbase=%p, pbase=%08x, size=%08x\n", __FUNCTION__, vbase, pbase, dmasize);

        k_mem_buffer_base_addr = vbase;
        k_mem_buffer_size = (UI32_T)dmasize;
        k_mem_buffer_physical_base_addr = L_CVRT_UINT_TO_PTR(pbase);

        K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_physical_base_addr = %p\n", __FUNCTION__, k_mem_buffer_physical_base_addr);
        K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_base_addr          = %p\n", __FUNCTION__, k_mem_buffer_base_addr);
        K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_size               = %08x\n", __FUNCTION__, k_mem_buffer_size);
    }

        #if (L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL == TRUE)
        if (k_mem_buffer_real_start_addr == NULL &&
            (k_mem_buffer_real_start_addr = kmalloc_giant(4 * SYS_TYPE_1M_BYTES)) == NULL)
        {
            printk(KERN_WARNING "%s: kmalloc_giant() failed.\n", __FUNCTION__);
            return;
        }
        #else
        if (k_mem_buffer_real_start_addr == NULL)
            return;
        #endif /* L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL */
    #endif /* end of #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE) */

#elif defined(MARVELL_CPSS) /* #if defined(BROADCOM) */
    /* value of k_mem_buffer_size should be equal to
     * highmem_reserved_by_accton, which will be used by L_IPCMEM_MEM_ID_TX and
     * L_IPCMEM_MEM_ID_MREF
     *
     * L_IPCMEM_TYPE_MARVELL_CPSS_DFLT_TX_MREF_SZ must be defined as
     * exactly the same value as MV_ACCTON_MREF_RESERVE in
     * linux-2.6.22.18/arch/arm/kernel/setup.c
     */
    k_mem_buffer_size = L_IPCMEM_TYPE_MARVELL_CPSS_DFLT_TX_MREF_SZ;
    if (k_mem_buffer_real_start_addr == NULL)
    {
        printk(KERN_INFO "%s: Call bspCacheDmaMalloc() to allocate %d bytes for TX and MREF buf.\n",
            __FUNCTION__, k_mem_buffer_size);
        k_mem_buffer_real_start_addr = bspCacheDmaMalloc(k_mem_buffer_size);
        k_mem_buffer_physical_base_addr = (void *)bspVirt2Phys((UI32_T)k_mem_buffer_real_start_addr);
        printk(KERN_INFO "%s: k_mem_buffer_real_start_addr=0x%08lX k_mem_buffer_physical_base_addr=0x%08lX\n",
            __FUNCTION__, k_mem_buffer_real_start_addr, k_mem_buffer_physical_base_addr);
    }
    if (k_mem_buffer_real_start_addr == NULL)
    {
        printk(KERN_EMERG "%s: bspCacheDmaMalloc() failed.\n", __FUNCTION__);
        return;
    }
    k_mem_buffer_base_addr = k_mem_buffer_real_start_addr;

#elif defined(MEDIATEK)
    /* k_mem_buffer_physical_base_addr will be set through
     * K_L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo() in DEV_SWDRV_ChipInit()
     */
    if (k_mem_buffer_physical_base_addr==NULL)
    {
        printk(KERN_INFO "%s: k_mem_buffer_physical_base_addr has not been initialized yet.\n",
            __FUNCTION__);
        return;
    }
#else
    /* this code snippet is not used by any project for now
     */
    k_mem_buffer_size = 4 * SYS_TYPE_1M_BYTES;
    k_mem_buffer_physical_base_addr = (void *)__pa(high_memory);
    if (k_mem_buffer_real_start_addr == NULL)
    {
        k_mem_buffer_real_start_addr = ioremap((int)k_mem_buffer_physical_base_addr, k_mem_buffer_size);
    }
    if (k_mem_buffer_real_start_addr == NULL)
    {
        printk(KERN_WARNING "%s: ioremap() failed.\n", __FUNCTION__);
        return;
    }
    k_mem_buffer_base_addr = k_mem_buffer_real_start_addr;
#endif /* L_IPCMEM_ALLOC_MEM_FROM_BCM */

    K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_physical_base_addr = 0x%p\n", __FUNCTION__, k_mem_buffer_physical_base_addr);
    K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_base_addr          = 0x%p\n", __FUNCTION__, k_mem_buffer_base_addr);
    K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_size               = 0x%08lx\n", __FUNCTION__, k_mem_buffer_size);
    K_L_IPCMEM_DEBUG_PRINT("%s: k_mem_buffer_real_start_addr    = 0x%p\n", __FUNCTION__, k_mem_buffer_real_start_addr);

    /* calculate offset for each type of memory
     */
    l_ipcmem_mem_offset_ar[0] = K_L_IPCMEM_GetOffset(k_mem_buffer_real_start_addr);/* set the offset of L_IPCMEM_MEM_ID_TX */
    if (l_ipcmem_mem_offset_ar[0]==(UI32_T)-1)
    {
        printk(KERN_EMERG "%s:Fatal Error!\n", __FUNCTION__);
        printk(KERN_EMERG "k_mem_buffer_base_addr=0x%p\n", k_mem_buffer_base_addr);
        printk(KERN_EMERG "k_mem_buffer_real_start_addr=0x%p\n", k_mem_buffer_real_start_addr);
        panic("Fatal error in K_L_IPCMEM_Initialize!\n");
        return;
    }

    K_L_IPCMEM_DEBUG_PRINT("%s: mem%d: size:0x%08x, offset:0x%08x\n", __FUNCTION__, 0, (unsigned int)l_ipcmem_mem_size_ar[0], (unsigned int)l_ipcmem_mem_offset_ar[0]);

#ifndef MARVELL_CPSS /* anzhen.zheng, 2009-05-19, 15:21:09 */
    for (i = 1; i < L_IPCMEM_MEM_ID_MAX; i++)
    {
        l_ipcmem_mem_offset_ar[i] = l_ipcmem_mem_offset_ar[i-1] + l_ipcmem_mem_size_ar[i-1];
        K_L_IPCMEM_DEBUG_PRINT(KERN_INFO "%s: mem%d: size:0x%08x, offset:0x%08x\n", __FUNCTION__, (int)i, (unsigned int)l_ipcmem_mem_size_ar[i], (unsigned int)l_ipcmem_mem_offset_ar[i]);
    }
#else
    /* for the high memory layout in Mavell solution:
     *  ~ low memory |  high memory
     *  ~ -----------+--------------------+---------------------+----+--------------------
     *    ...        | L_IPCMEM_MEM_ID_TX | L_IPCMEM_MEM_ID_MREF|    | L_IPCMEM_MEM_ID_RX
     *               |                                          |    |
     *               |                                          |    +--> The virtual address mmap in user space must be SYS_ADPT_DMA_VIRTUAL_ADDR(consistent with the setting in Marvell SDK)
     *               |                                          |    |
     *               |                                          |<-->|
     *               |                         Unused reserved buffer|
     *               |                                               |
     *               |<--  Size = k_mem_buffer_size               -->|
     *
     *  Note that the region of L_IPCMEM_MEM_ID_RX is managed by Marvell SDK.
     *  The regions of L_IPCMEM_MEM_ID_TX and L_IPCMEM_MEM_ID_MREF are managed by IPCMEM.
     */
    l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_RX] = l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_TX] + k_mem_buffer_size;
    l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_MREF] = l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_TX] + l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_TX];
#endif
    K_L_IPCMEM_DEBUG_PRINT(KERN_INFO "%s: total_size_of_buf_pool: 0x%08x\n", __FUNCTION__, (unsigned int)total_size_of_buf_pool);
#ifdef MARVELL_CPSS
    /* Marvell cpss environment is now hard coded as 2MB for IPC memory used by Accton
     */
    if(total_size_of_buf_pool > k_mem_buffer_size)
        panic("Total size of IPCMEM buffer(%lu bytes) exceeds the reserved memory size(%lu bytes)!\n", total_size_of_buf_pool, k_mem_buffer_size);
#endif


    /* initialize buf_pool_descs
     */
    buf_p = K_L_IPCMEM_GetPtr(l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_TX]);

    for(i = 0; i < L_IPCMEM_NUMBER_OF_BUF_TYPE; i++)
    {
        buf_pool_descs[i].buffer = buf_p;
        if(FALSE==K_L_MM_PtCreate(&(buf_pool_descs[i])))
        {
            char *urgent_msg_p = "[0]K_L_IPCMEM_Initialize: Failed to init buf_pool_descs.";

            urgent_msg_p[1] = '0' + i; /* replace 0 with the failed index */
            SYSFUN_LogUrgentMsg(urgent_msg_p);
        }

#if _IN_TARGET
        /* init access_counter
         */
        memset(&(access_counter[i]), 0, sizeof(L_IPCMEM_COUNTER_REC_T));
        access_counter[i].peak_left_amount = num_of_block_for_each_buf_type[i];
#endif
        /* move to beginning addr for next buf type
         */
        buf_p += buf_pool_descs[i].buffer_len;
    }

    k_dedicated_buffer_start_addr = buf_p;

    for(i = 0; i < L_IPCMEM_NUMBER_OF_DEDICATED_BUF_TYPE; i++)
    {
        dedicated_buf_pool_descs[i].buffer = buf_p;
        if(FALSE==K_L_MM_PtCreate(&(dedicated_buf_pool_descs[i])))
        {
            char *urgent_msg_p = "[0]K_L_IPCMEM_Initialize: Failed to init dedicated_buf_pool_descs.";

            urgent_msg_p[1] = '0' + i; /* replace 0 with the failed index */
            SYSFUN_LogUrgentMsg(urgent_msg_p);
        }

        /* move to beginning addr for next buf type
         */
        buf_p += dedicated_buf_pool_descs[i].buffer_len;
    }

    k_mem_buffer_end_addr = buf_p;

    return;
} /* End of L_IPCMEM_Initial */

void K_L_IPCMEM_Uninitialize(void)
{
    int rc = 0;

    if (k_mem_buffer_real_start_addr != NULL)
    {
#if defined(BROADCOM)
    #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE)
        #if (L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL == TRUE)
        kfree_giant(k_mem_buffer_real_start_addr);
        #endif
    #endif /* #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE) */
#elif defined(MARVELL_CPSS) /* #if defined(BROADCOM) */
        /* NOTE:
         *   bspCacheDmaFree always return 1 (MV_FAIL)
         *   that is harmless,
         *   it means we dont need to reallocate it.
         */
        rc = bspCacheDmaFree(k_mem_buffer_real_start_addr);
#elif defined(MEDIATEK)
        /* do nothing */
#else /* #if defined(BROADCOM) */
        iounmap(k_mem_buffer_real_start_addr);
#endif /* #if defined(BROADCOM) */
    }

#ifndef MEDIATEK
    if (rc == 0)
    {
        k_mem_buffer_real_start_addr = NULL;
    }
#endif

}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - K_L_IPCMEM_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void K_L_IPCMEM_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_L_IPCMEM, K_L_IPCMEM_Operation);
}

/*  FUNCTION NAME : K_L_IPCMEM_Allocate
 *  PURPOSE:
 *      This function is primary used for allocate buffer for packet transmission.
 *
 *  INPUT:
 *   size    -- the size of the buffer to be allocated.
 *   user_id -- this id will be passed to L_MM for the memory monitor information.
 *              user_id = trace_id(6) + pool_id(2) + module_id(8)
 *              pool_id is at user's own usage/knowledge.
 *              user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *              or L_MM_USER_ID2(modul_id,ext_trace_id)
 *
 *  OUTPUT:
 *   None.
 *
 *  RETURN:
 *   Address of the allocated buffer
 *   Return null if no more buffer is available.
 *
 *  NOTES:
 *   The allocated buffer can be monitored through l_mm backdoor.
 */
void *K_L_IPCMEM_Allocate(UI32_T size, UI16_T user_id)
{
    /* LOCAL VARIABLES
     */
    void  *buf_p=NULL;
    UI32_T buf_type;
    SYSFUN_IntMask_T oint;
#if _IN_TARGET
    UI32_T best_fit_buf_type;
#endif

    if (size> L_IPCMEM_MAX_SIZE)
    {
        printk(KERN_WARNING "%s: Request size too big(%lu)",__FUNCTION__, (unsigned long)size);
        allocate_fail_counter++;
        return 0;
    } /* end if */


    /* Find suitable min. size buffer */
    for ( buf_type=0; buf_type<L_IPCMEM_NUMBER_OF_BUF_TYPE; buf_type++)
    {
        if ( buf_pool_descs[buf_type].partition_len >= size ) break;
    }

#if _IN_TARGET
    best_fit_buf_type=buf_type;
#endif

    /* critical region
     */
    L_IPCMEM_ENTER_CRITICAL_SECTION(&ipcmem_lock, oint);

    /* get buffer block from buf_pool_descs by buf_type
     */
    for ( ; buf_type< L_IPCMEM_NUMBER_OF_BUF_TYPE; buf_type++)
    {
        buf_p=K_L_MM_PtAlloc(&(buf_pool_descs[buf_type]), user_id);
        if(buf_p != NULL)
        {
#if _IN_TARGET
            access_counter[buf_type].alloc_counter++;
            if(access_counter[buf_type].peak_left_amount > buf_pool_descs[buf_type].nbr_of_free_partition)
            {
                access_counter[buf_type].peak_left_amount = buf_pool_descs[buf_type].nbr_of_free_partition;
            }
#endif
            break;
        }

    } /* end for */

#if _IN_TARGET
    if(best_fit_buf_type!=buf_type)
    {
        access_counter[best_fit_buf_type].alloc_fail_counter++;
    }
#endif

    if(buf_p==NULL)
    {
        allocate_fail_counter++;
    }
    L_IPCMEM_LEAVE_CRITICAL_SECTION(&ipcmem_lock, oint);

   return buf_p;
} /* K_L_IPCMEM_Allocate */

/*  FUNCTION NAME : K_L_IPCMEM_Free
 *  PURPOSE:
 *      Free memory block to memory pool.
 *  INPUT:
 *      buf_p -- the pointer points to the memory block to be freed.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      TRUE    -- Sucess.
 *      FALSE   -- Fail.
 *
 *  NOTES:
 *      None.
 */
BOOL_T K_L_IPCMEM_Free(void *ptr)
{
    /* LOCAL VARIABLES
     */
    L_MM_PtDesc_T *desc_p;
    UI32_T buf_type;

    /* BODY
     */
    if (ptr == 0 )
    {
        SYSFUN_LogMsg("K_L_IPCMEM_Free: null pointer.\n", 0,0,0,0,0,0);
        return FALSE;
    }

    if((buf_type = K_L_MM_GetBufferType(ptr))==L_MM_ERROR_RETURN_VAL)
    {
        SYSFUN_LogMsg("K_L_IPCMEM_Free: invalid pointer\n", 0,0,0,0,0,0);
        return FALSE;
    }

    if (ptr < k_dedicated_buffer_start_addr)
        desc_p = &(buf_pool_descs[buf_type]);
    else
        desc_p = &(dedicated_buf_pool_descs[buf_type]);

    /* free buffer according to buffer type */
    if(K_L_MM_PtFree(desc_p, ptr)==FALSE)
    {
        SYSFUN_LogMsg("K_L_IPCMEM_Free : Fail to free type %lu buffer", buf_type,0,0,0,0,0);
        return FALSE;
    }

#if _IN_TARGET
    access_counter[buf_type].free_counter++;
#endif

    return    TRUE;
} /* K_L_IPCMEM_Free */

/*  FUNCTION NAME : K_L_IPCMEM_AllocateFromDedicatedBufPool
 *  PURPOSE:
 *      This function is used for allocate dedicated buffer for packet transmission.
 *
 *  INPUT:
 *   buf_type -- the type of the buffer to be allocated.
 *   user_id -- this id will be passed to L_MM for the memory monitor information.
 *              user_id = trace_id(6) + pool_id(2) + module_id(8)
 *              pool_id is at user's own usage/knowledge.
 *              user_id is got from macro L_MM_USER_ID (modul_id, pool_id, trace_id)
 *              or L_MM_USER_ID2(modul_id,ext_trace_id)
 *
 *  OUTPUT:
 *   buf_size_p -- allocated size of buffer.
 *
 *  RETURN:
 *   Address of the allocated buffer
 *   Return null if no more buffer is available.
 *
 *  NOTES:
 *   The allocated buffer can be monitored through l_mm backdoor.
 */
void *K_L_IPCMEM_AllocateFromDedicatedBufPool(UI32_T buf_type, UI32_T user_id, UI32_T *buf_size_p)
{
    void *buf_p;

    if (buf_type >= L_IPCMEM_NUMBER_OF_DEDICATED_BUF_TYPE)
        return NULL;

    buf_p = K_L_MM_PtAlloc(&(dedicated_buf_pool_descs[buf_type]), user_id);

    if (buf_p != NULL && buf_size_p != NULL)
    {
        *buf_size_p = dedicated_buf_pool_descs[buf_type].partition_len;
#if _IN_TARGET
        access_counter[buf_type].alloc_counter++;
#endif
    }

    return buf_p;
}

/*  FUNCTION NAME : K_L_IPCMEM_GetStartOfMemBufferAddr
 *  PURPOSE:
 *      This function will return the start address(This address includes
 *      IPCMEM memory buffer) of the IPCMEM memory buffer.
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      The buffer of the start address of the IPCMEM memory buffer.
 *  NOTES:
 *      None.
 */
void* K_L_IPCMEM_GetStartOfMemBufferAddr(void)
{
#ifdef MARVELL_CPSS
    return k_mem_buffer_base_addr;
#else
    return k_mem_buffer_real_start_addr;
#endif
}

/*  FUNCTION NAME : K_L_IPCMEM_GetEndOfMemBufferAddr
 *  PURPOSE:
 *      This function will return the end address(This address is not included
 *      in L_IPCMEM memory buffer) of the L_IPCMEM memory buffer.
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      The buffer of the end address of the L_IPCMEM memory buffer.
 *  NOTES:
 *      Behind the IPCMEM memory buffer is the buffer for NIC Rx buffer.
 */
void* K_L_IPCMEM_GetEndOfMemBufferAddr(void)
{
    return k_mem_buffer_end_addr;
}

/*  FUNCTION NAME : K_L_IPCMEM_GetOffset
 *  PURPOSE:
 *      Get the offset to the beginning of IPCMEM memory address of
 *      the given buffer.
 *  INPUT:
 *      buf_p -- the buffer allocated by K_L_IPCMEM_Allocate
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- the offset to the beginning of IPCMEM memory address.
 *      Fail   -- (-1) will be returned if buf_p is invalid or buf_p is less than the base address.
 *
 *  NOTES:
 *
 */
UI32_T K_L_IPCMEM_GetOffset(void* buf_p)
{
    I32_T offset = (I32_T)((UI8_T*)buf_p - (UI8_T*)K_L_IPCMEM_GetStartOfMemBufferAddr());

    if(buf_p==NULL)
        return -1;

    if(offset < 0)
    {
        printk(KERN_WARNING "%s():Invalid buf_p\n", __FUNCTION__);
        return -1;
    }
    return (UI32_T)offset;
}

/*  FUNCTION NAME : K_L_IPCMEM_GetPtr
 *  PURPOSE:
 *      This function will convert the offset to the real address.
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- The logical address of the specified offset.
 *      Fail   -- NULL
 *  NOTES:
 *      None.
 */
void* K_L_IPCMEM_GetPtr(UI32_T offset)
{
    return L_CVRT_GET_PTR(K_L_IPCMEM_GetStartOfMemBufferAddr(), offset);
}

/*  FUNCTION NAME : K_L_IPCMEM_GetMRefBufStartAddress
 *  PURPOSE:
 *      Get the starting address(logical address) of the mref buffer.
 *
 *  OUTPUT:
 *      mref_buf_start_address -- the starting address of the mref buffer
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      1.This API is for L_MM to prepare mref buffer.
 *      2.On Linux, mref buffer is located in the region of high memory.
 */
BOOL_T K_L_IPCMEM_GetMRefBufStartAddress(void **mref_buf_start_address)
{
    if (k_mem_buffer_base_addr == NULL)
        return FALSE;

    *mref_buf_start_address = K_L_IPCMEM_GetPtr(l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_MREF]);
    return TRUE;
}


/* system call implementation
 */

/* return value and args depends on the given cmd
 */
static UI32_T K_L_IPCMEM_Operation(void* arg0, void* arg1, void* arg2, void* arg3, void* arg4)
{
    UI32_T ret = FALSE;
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {
        case L_IPCMEM_CMD_GET_PHYSICAL_BASE_ADDRESS:
            /* arg1(OUT): physical base address of the high memory
             * ret: TRUE - Scuess. FALSE - Fail.
             */
            if(k_mem_buffer_physical_base_addr != 0)
            {
                SYSFUN_CopyToUser(arg1, &k_mem_buffer_physical_base_addr, sizeof(void*));
                ret = (UI32_T)TRUE;
            }
            break;
        case L_IPCMEM_CMD_GET_IPCMEM_INFO:
        {
            /* arg1(OUT): the base physical addr
             * arg2(OUT): the size of memory to mmap
             * arg3(OUT): the size for each type of memory
             * arg4(OUT): the offset for each type of memory
             * ret: TRUE/FALSE
             */
            void* k_ipcmem_buffer_physical_base_addr;
            UI32_T k_ipcmem_buffer_size;
#ifdef MARVELL_CPSS
            k_ipcmem_buffer_physical_base_addr=k_mem_buffer_physical_base_addr;
            k_ipcmem_buffer_size=k_mem_buffer_size;
#else
            k_ipcmem_buffer_physical_base_addr=(UI8_T*)k_mem_buffer_physical_base_addr + L_CVRT_GET_OFFSET(k_mem_buffer_base_addr, k_mem_buffer_real_start_addr);
            k_ipcmem_buffer_size=total_size_of_buf_pool;
#endif
            if (k_mem_buffer_physical_base_addr != NULL && k_mem_buffer_real_start_addr != NULL)
            {
                ret = (UI32_T)TRUE;

                if (L_ALIGN(k_ipcmem_buffer_physical_base_addr, PAGE_SIZE)!=L_CVRT_PTR_TO_UINT(k_ipcmem_buffer_physical_base_addr))
                {
                    printk(KERN_EMERG "%s:L_IPCMEM_CMD_GET_IPCMEM_INFO:k_ipcmem_buffer_physical_base_addr(0x%p) is not aligned to page boundary(page size=0x%lX)\n",
                        __FUNCTION__, k_ipcmem_buffer_physical_base_addr, PAGE_SIZE);
                    ret = (UI32_T)FALSE;
                }

                if (L_ALIGN(k_ipcmem_buffer_size, PAGE_SIZE)!=k_ipcmem_buffer_size)
                {
                    printk(KERN_EMERG "%s:L_IPCMEM_CMD_GET_IPCMEM_INFO:k_ipcmem_buffer_size(0x%08lX) is not aligned to page boundary(page size=0x%lX)\n",
                        __FUNCTION__, (unsigned long)k_ipcmem_buffer_size, (unsigned long)PAGE_SIZE);
                    ret = (UI32_T)FALSE;
                }

                if (ret ==(UI32_T)TRUE)
                {
                    SYSFUN_CopyToUser((void *)arg1, &k_ipcmem_buffer_physical_base_addr, sizeof(k_ipcmem_buffer_physical_base_addr));
                    SYSFUN_CopyToUser((void *)arg2, &k_ipcmem_buffer_size, sizeof(k_ipcmem_buffer_size));
                    SYSFUN_CopyToUser((void *)arg3, l_ipcmem_mem_size_ar, sizeof(l_ipcmem_mem_size_ar));
                    SYSFUN_CopyToUser((void *)arg4, l_ipcmem_mem_offset_ar, sizeof(l_ipcmem_mem_offset_ar));
                }

            }
            else
            {
                ret = (UI32_T)FALSE;
            }
        }
            break;

        case L_IPCMEM_CMD_GET_IPCMEM_TOTAL_SIZE:
            /* arg1(OUT): the total required size
             * ret: TRUE/FALSE
             */
            SYSFUN_CopyToUser((void *)arg1, &total_size_of_buf_pool, sizeof(total_size_of_buf_pool));
            if (total_size_of_buf_pool)
            {
                ret = (UI32_T)TRUE;
            }
            else
            {
                ret = (UI32_T)FALSE;
            }
            break;

        case L_IPCMEM_CMD_INITIALIZE_BUF_POOL:
        {
            /* arg1(IN/OUT): mem_buffer_real_start_offset
             */
            UI32_T mem_buffer_real_start_offset;

            K_L_IPCMEM_Uninitialize();

            SYSFUN_CopyFromUser(&mem_buffer_real_start_offset, (void*)arg1, sizeof(UI32_T));
            /* -1 means to initialize memory pool without specified memory buffer.
             */
            if (mem_buffer_real_start_offset != (UI32_T)-1)
            {
                k_mem_buffer_real_start_addr = L_CVRT_GET_PTR(k_mem_buffer_base_addr, mem_buffer_real_start_offset);
                if (L_CVRT_PTR_TO_UINT(k_mem_buffer_real_start_addr) != L_ALIGN(k_mem_buffer_real_start_addr, PAGE_SIZE))
                {
                    printk(KERN_INFO "%s:L_IPCMEM_CMD_INITIALIZE_BUF_POOL:org k_mem_buffer_real_start_addr=0x%p, real_start_offset=0x%08lX",
                        __FUNCTION__, k_mem_buffer_real_start_addr, (unsigned long)mem_buffer_real_start_offset);
                    k_mem_buffer_real_start_addr=(void*)L_ALIGN(k_mem_buffer_real_start_addr, PAGE_SIZE);
                    printk(KERN_INFO "%s:L_IPCMEM_CMD_INITIALIZE_BUF_POOL:revised k_mem_buffer_real_start_addr=0x%p, real_start_offset=0x%08lX",
                        __FUNCTION__, k_mem_buffer_real_start_addr, (unsigned long)mem_buffer_real_start_offset);
                    mem_buffer_real_start_offset=L_CVRT_GET_OFFSET(k_mem_buffer_base_addr, k_mem_buffer_real_start_addr);
                    SYSFUN_CopyToUser((void*)arg1, &mem_buffer_real_start_offset, sizeof(UI32_T));
                }
            }

            K_L_IPCMEM_Initialize();
            K_L_MM_Init();
        }
            break;

        case L_IPCMEM_CMD_GET_OFFSET:
            /* arg1(IN): void *
             * ret : UI32_T
             */
            ret = K_L_IPCMEM_GetOffset((void *)arg1);
            break;

        case L_IPCMEM_CMD_GET_PTR:
        {
            void* physical_addr;

            /* arg1(IN): UI32_T
             * arg2(OUT): void *
             * ret : Do not care
             */
            physical_addr = K_L_IPCMEM_GetPtr(L_CVRT_PTR_TO_UINT(arg1));
            SYSFUN_CopyToUser((void*)arg2, &physical_addr, sizeof(void*));
        }
            break;

        case L_IPCMEM_CMD_PREINITIALIZE_SETKERNELSPACEADDRINFO:
            /* arg1(IN): void *k_virtual_base_address
             * arg2(IN): void *k_physical_base_address
             * ret : Do not care
             */
            printk(KERN_INFO "%s:L_IPCMEM_CMD_PREINITIALIZE_SETKERNELSPACEADDRINFO(k_virtual_base_address=%p,k_physical_base_address=%p\n",
                __FUNCTION__, arg1, arg2);

            if (arg1==NULL)
            {
                arg1 = phys_to_virt((phys_addr_t)arg2);
                printk(KERN_INFO "%s:Coverted virtaul addr=%p\n", __FUNCTION__, arg1);
            }
            K_L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo(arg1, arg2);
            ret = (UI32_T)TRUE;
            break;

        case L_IPCMEM_CMD_BACKDOOR:
#ifdef  BACKDOOR_OPEN
            switch(L_CVRT_PTR_TO_UINT(arg1))
            {
                /* arg1(IN): L_IPCMEM_Backdoor command
                 * arg2(IN/OUT): L_Mem_BackdoorInfoEntry_S[L_IPCMEM_NUMBER_OF_BUF_TYPE]
                 *               backdoor information will be put to arg2.
                 * arg3(IN/OUT): UI32_T. allocate_fail_counter. Total allocation fail counts.
                 * ret : TRUE - Sucess. FALSE - Fail.
                 */
                case L_IPCMEM_BACKDOOR_CMD_GETBACKDOORINFO:
                {
                    L_Mem_BackdoorInfoEntry_T backdoor_info[L_IPCMEM_NUMBER_OF_BUF_TYPE];

                    if(arg3==0)
                    {
                        printk(KERN_WARNING "L_IPCMEM_BACKDOOR_CMD_GETBACKDOORINFO:Invalid arg3.\n");
                        return FALSE;
                    }
                    K_L_IPCMEM_GetBackdoorInfo(backdoor_info);
                    copy_to_user((L_Mem_BackdoorInfoEntry_T*)arg2, &backdoor_info, sizeof(backdoor_info));
                    put_user(allocate_fail_counter, (UI32_T*)arg3);
                    ret = TRUE;
                }
                break;
                /* arg1(IN): L_IPCMEM_Backdoor command
                 * arg2(IN): BOOL_T. is_get_front_tmz? TRUE - get front tmz. FALSE - get rear tmz.
                 * arg3(IN/OUT): char[MEM_TMZ_SIZE]. TMZ Info will be put in this buffer.
                 * ret : TRUE - Scuess. FALSE - Fail.
                 */
                case L_IPCMEM_BACKDOOR_CMD_GETTMZINFO:
                    if(arg3==0)
                    {
                        printk(KERN_WARNING "L_IPCMEM_BACKDOOR_CMD_GETTMZINFO:Invalid arg3.\n");
                        return FALSE;
                    }
                    if(L_CVRT_PTR_TO_UINT(arg2) == TRUE)
                    {
                        copy_to_user((char*)arg3, l_mem_tmz_front, MEM_TMZ_SIZE);
                    }
                    else
                    {
                        copy_to_user((char*)arg3, l_mem_tmz_rear, MEM_TMZ_SIZE);
                    }
                    ret = TRUE;
                break;
                default:
                    printk(KERN_WARNING "%s: Unknown backdoor cmd(%d)\n", __FUNCTION__, (int)L_CVRT_PTR_TO_UINT(arg1));
                    ret = FALSE;
            }
#endif
            break;
        default:
            printk(KERN_WARNING "<4>%s: Unknown cmd(%d)\n", __FUNCTION__, cmd);
    }

    return ret;
}

/*  FUNCTION NAME : K_L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo
 *  PURPOSE:
 *      Set the virtual base address and physical base address of the dma packet
 *      buffer in kernel space.
 *
 *  INPUT:
 *      k_virtual_base_address  -- the start virtual address of the packet dma
 *                                 buffer in kernel space.
 *      k_physical_base_address -- the start physical address of the packet dma
 *                                 buffer in kernel space.
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      None.
 *
 *  NOTES:
 *      1. This function should only be called in driver process
 *         and before calling L_IPCMEM_Initialize().
 *      2. After the dma packet buffer had been allocated by SDK allocation API,
 *         need to pass the start virtual base address and start physical base
 *         address in kernel mode by calling this function.
 */
static void K_L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo(void *k_virtual_base_address, void *k_physical_base_address)
{
    k_mem_buffer_base_addr=k_virtual_base_address;
    k_mem_buffer_physical_base_addr=k_physical_base_address;
    k_mem_buffer_real_start_addr=k_virtual_base_address;
}


#ifdef  BACKDOOR_OPEN

static void K_L_IPCMEM_GetBackdoorInfo(L_Mem_BackdoorInfoEntry_T backdoor_info[])
{
    UI32_T buf_type;

    for(buf_type=0; buf_type<L_IPCMEM_NUMBER_OF_BUF_TYPE; buf_type++)
    {
        backdoor_info[buf_type].partition_len = buf_pool_descs[buf_type].partition_len;
        backdoor_info[buf_type].num_of_block = num_of_block_for_each_buf_type[buf_type];
        backdoor_info[buf_type].alloc_counter = access_counter[buf_type].alloc_counter;
        backdoor_info[buf_type].free_counter = access_counter[buf_type].free_counter;
        backdoor_info[buf_type].used_count = num_of_block_for_each_buf_type[buf_type] - buf_pool_descs[buf_type].nbr_of_free_partition;
        backdoor_info[buf_type].peak_count = num_of_block_for_each_buf_type[buf_type] - access_counter[buf_type].peak_left_amount;
        backdoor_info[buf_type].fail_count = access_counter[buf_type].alloc_fail_counter;
    }
}

#endif  /*  BACKDOOR_OPEN   */

