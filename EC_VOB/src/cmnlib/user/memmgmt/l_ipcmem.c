/* MODULE NAME:  L_IPCMEM.c
 * PURPOSE:
 * This module is for allocating and freeing buffer for lan packet transmission.
 *
 * NOTES:
 * This module need to cooperate with lanbuf device.
 * Before using this library, ensure that lanbuf module have been loaded into
 * kernel and "/dev/lanbuf" must have been created.
 *
 * HISTORY
 *    2006.9.21 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2006
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <unistd.h>    /* for getpagesize() */
#include <sys/mman.h>  /* for mmap() */
#include <sys/types.h> /* for open() */
#include <sys/stat.h>  /* for open() */
#include <fcntl.h>     /* for open() */
#include <errno.h>
#include <stdint.h> /* for uintptr_t */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#include "l_cvrt.h"
#include "l_ipcmem.h"
#include "l_ipcmem_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T L_IPCMEM_GetIPCMemInfo(void *physical_base_addr_p,
                                     UI32_T *physical_mem_size_p,
                                     UI32_T ipcmem_size[L_IPCMEM_MEM_ID_MAX],
                                     UI32_T ipcmem_offset[L_IPCMEM_MEM_ID_MAX]);

/* STATIC VARIABLE DECLARATIONS
 */
static void   *mem_buffer_base_addr = NULL;
static int    fd = -1;

#ifdef MARVELL_CPSS
static void   *mvpp_buffer_base_addr = NULL;
static int    mvppfd = -1;
#endif


static UI32_T l_ipcmem_mem_size_ar[L_IPCMEM_MEM_ID_MAX];
static UI32_T l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_MAX];

static void   *uc_mem_buffer_base_addr = NULL;

static BOOL_T l_ipcmem_has_initialize = FALSE;

/* EXPORTED SUBPROGRAM BODIES
 */
/*  FUNCTION NAME : L_IPCMEM_Preinitialize_SetVirtualBaseAddress
 *  PURPOSE:
 *      Set the start address of memory that IPCMEM will use.
 *
 *  INPUT:
 *      virtual_base_address    -- the start address
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      None.
 *
 *  NOTES:
 *      This function should only be called in driver process
 *      and before calling L_IPCMEM_Initialize().
 */
void L_IPCMEM_Preinitialize_SetVirtualBaseAddress(void *virtual_base_address)
{
#if defined(BROADCOM)
    #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE)
        mem_buffer_base_addr = (void *)virtual_base_address;
    #else
        /* do nothing */
    #endif
#endif
}

/*  FUNCTION NAME : L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo
 *  PURPOSE:
 *      Set the virtual base address and physical base address of the dma packet
 *      buffer in kernel space.
 *
 *  INPUT:
 *      k_virtual_base_address  -- the start virtual address of the packet dma
 *                                 buffer in kernel space. If the value is NULL,
 *                                 it will use linux kernel function phys_to_virt
 *                                 to get virtual addr from k_physical_base_address.
 *      k_physical_base_address -- the start physical address of the packet dma
 *                                 buffer in kernel space.
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      None.
 *
 *  NOTES:
 *      This function should only be called in driver process
 *      and before calling L_IPCMEM_Initialize().
 */
void L_IPCMEM_Preinitialize_SetKernelSpaceAddrInfo(void *k_virtual_base_address, void *k_physical_base_address)
{
    if (k_physical_base_address==NULL)
    {
        printf("%s:Error. k_physical_base_address is NULL.\r\n", __FUNCTION__);
        return;
    }

    SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, (void*)L_IPCMEM_CMD_PREINITIALIZE_SETKERNELSPACEADDRINFO, k_virtual_base_address, k_physical_base_address, (void*)0, (void*)0);
    return;
}

void L_IPCMEM_Preinitialize_InitMemoryPool(void *(*alloc_func)(unsigned int size, char *name))
{
    void *mem_buffer_real_start_addr = NULL;
    UI32_T total_size_of_buf_pool=0;
    UI32_T mem_buffer_real_start_offset=0;

    if (!SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, (void*)L_IPCMEM_CMD_GET_IPCMEM_TOTAL_SIZE, &total_size_of_buf_pool, (void*)0, (void*)0, (void*)0))
    {
        printf("%s: can't get total require size.\r\n", __FUNCTION__);
        return;
    }

#if defined(BROADCOM)
    #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE && L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL == FALSE)
    if (mem_buffer_base_addr == NULL || alloc_func == NULL)
    {
        printf("%s: alloc_func is invalid.\r\n", __FUNCTION__);
        return;
    }

    mem_buffer_real_start_addr = alloc_func(total_size_of_buf_pool, "L_IPCMEM");
    if (mem_buffer_real_start_addr==NULL)
    {
        printf("%s:Fatal error! allocate %lu bytes failed!\r\n", __FUNCTION__, (unsigned long)total_size_of_buf_pool);
        return;
    }
    mem_buffer_real_start_offset = (UI32_T)((uintptr_t)mem_buffer_real_start_addr - (uintptr_t)mem_buffer_base_addr);
    #else /* #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE && L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL == FALSE) */
    /* -1 means to initialize memory pool without specified memory buffer.
     */
    mem_buffer_real_start_offset = (UI32_T)-1;
    #endif /* #if (L_IPCMEM_ALLOC_MEM_FROM_BCM == TRUE && L_IPCMEM_ALLOC_MEM_FROM_BCM_KERNEL == FALSE) */
#elif defined(MEDIATEK)
    mem_buffer_base_addr = mem_buffer_real_start_addr = alloc_func(total_size_of_buf_pool, "L_IPCMEM");
    mem_buffer_real_start_offset = (UI32_T)-1;
#endif /* end of #if defined(BROADCOM) */
    /* note that the mem_buffer_real_start_offset might not be at page boundary
     * it is possible that the offset will be revised to a page-aligned offset
     * in k_l_ipcmem.c
     */
    SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, (void*)L_IPCMEM_CMD_INITIALIZE_BUF_POOL, &mem_buffer_real_start_offset, (void*)0, (void*)0, (void*)0);

    if (mem_buffer_real_start_offset!=(UI32_T)-1)
    {
        /* update mem_buffer_base_addr based on the output of mem_buffer_real_start_offset
         */
        mem_buffer_real_start_addr = L_CVRT_GET_PTR(mem_buffer_base_addr, mem_buffer_real_start_offset);
        mem_buffer_base_addr=mem_buffer_real_start_addr;
    }
}

/*  FUNCTION NAME : L_IPCMEM_Initialize
 *  PURPOSE:
 *      Initializes L_IPCMEM.
 *
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      TRUE  --  Sucess
 *      FALSE --  Fail
 *
 *  NOTES:
 *      None.
 */
BOOL_T L_IPCMEM_Initialize(void)
{
    void* physical_base_addr;
    UI32_T physical_mem_size, i;

    if (l_ipcmem_has_initialize)
    {
        return TRUE;
    }

    if (fd == -1)
        fd = open("/dev/mem", O_RDWR);

    if (!L_IPCMEM_GetIPCMemInfo(&physical_base_addr, &physical_mem_size, l_ipcmem_mem_size_ar, l_ipcmem_mem_offset_ar))
    {
        printf("%s:L_IPCMEM_GetIPCMemInfo fail.\r\n", __FUNCTION__);
        return FALSE;
    }

    L_IPCMEM_Debug_Print("%s:physical_base_addr = %08x physical_mem_size = %08x\r\n",
            __FUNCTION__, (unsigned int)physical_base_addr, (unsigned int)physical_mem_size);

    for (i = 0; i < L_IPCMEM_MEM_ID_MAX; i++)
    {
        L_IPCMEM_Debug_Print("\r\n%s:mem%d size:%08x offset:%08x", __FUNCTION__, (int)i, (unsigned int)l_ipcmem_mem_size_ar[i], (unsigned int)l_ipcmem_mem_offset_ar[i]);
    }

    L_IPCMEM_Debug_Print("\r\n%s:fd is %d", __FUNCTION__, fd);

#if defined(BROADCOM) || defined(MEDIATEK)
    if (mem_buffer_base_addr == NULL)
    {
        /* In BCM project, physical_mem_size is the size of dma buffer allocated from BCM SDK.
         * The total dma buffer size of BCM SDK is determined in BCM kernel module
         * (passed by the argument "dmasize=XXM" when executing insmod linux-kernel-bde.ko in rc)
         *
         * In K_L_IPCMEM, it uses the starting address allocated through the BCM
         * function as the base address of the memory managed by IPCMEM.
         * Instead mapping all dma memory region into the address space of the
         * process, only the region managed by IPCMEM need to be mapped into.
         * The region to be mapped into is
         *     [physical_base_addr -- physical_base_addr+physical_mem_size-1]
         */
        mem_buffer_base_addr = mmap(0, (size_t)physical_mem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (uintptr_t)physical_base_addr);
        L_IPCMEM_Debug_Print("%s(%d): Map physical addr 0x%p--0x%p into virtual addr 0x%p--0x%p of the pid %lu\r\n",
            __FUNCTION__, __LINE__, physical_base_addr, (uintptr_t)physical_base_addr+physical_mem_size-1,
            mem_buffer_base_addr, (UI8_T*)mem_buffer_base_addr+physical_mem_size-1, SYSFUN_TaskIdSelf());

        if(mem_buffer_base_addr == MAP_FAILED)
        {
            printf("%s:mmap fail.\r\n", __FUNCTION__);
            return FALSE;
        }
    }

    L_IPCMEM_Debug_Print("\r\n%s:mem_buffer_base_addr = %08x\r\n", __FUNCTION__, (int)mem_buffer_base_addr);
#elif defined(MARVELL_CPSS) /* #if defined(BROADCOM) */
    if (mem_buffer_base_addr == NULL)/* map tx and mref. */
    {
        /* the starting virtual address of the buffer used by Marvell CPSS must
         * be at SYS_ADPT_DMA_VIRTUAL_ADDR.
         * For the end virtual address of the buffer managed by IPCMEM is
         * (SYS_ADPT_DMA_VIRTUAL_ADDR-1), and the size of buffer managed by
         * IPCMEM is physical_mem_size. So the starting virtual address of the
         * buffer managed by IPCMEM will be
         * SYS_ADPT_DMA_VIRTUAL_ADDR-physical_mem_size
         */
        mem_buffer_base_addr = mmap(SYS_ADPT_DMA_VIRTUAL_ADDR-physical_mem_size, (size_t)physical_mem_size, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, fd, physical_base_addr);
        if(mem_buffer_base_addr == MAP_FAILED)
        {
            printf("%s:mmap fail.\r\n", __FUNCTION__);
            return FALSE;
        }
    }

    L_IPCMEM_Debug_Print("\r\n%s:mem_buffer_base_addr = %08x\r\n", __FUNCTION__, (int)mem_buffer_base_addr);

    if (mvppfd == -1)
        mvppfd = open("/dev/mvPP", O_RDWR);
    
    L_IPCMEM_Debug_Print("\r\n%s:fd is %d", __FUNCTION__, mvppfd);
    if (mvpp_buffer_base_addr == NULL)/* map dma buffer. */
    {
        size_t dma_len = 2 * SYS_TYPE_1M_BYTES;

    #ifdef SYS_ADPT_DMA_BUFFER_SIZE
        dma_len = SYS_ADPT_DMA_BUFFER_SIZE;
    #endif

        mvpp_buffer_base_addr = mmap(SYS_ADPT_DMA_VIRTUAL_ADDR, dma_len, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, mvppfd, physical_base_addr+2*SYS_TYPE_1M_BYTES);
        if(mvpp_buffer_base_addr == MAP_FAILED)
        {
            printf("%s:mmap fail.\r\n", __FUNCTION__);
            return FALSE;
        }
    }
    L_IPCMEM_Debug_Print("\r\n%s:mvpp_buffer_base_addr = %08x\r\n", __FUNCTION__, (int)mvpp_buffer_base_addr);
#endif /* end of #if defined(BROADCOM) */


#if 0
    {
        void *mref_buf_p = L_CVRT_GET_PTR(mem_buffer_base_addr, l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_MREF]);
        printf("Dump: %08x", (unsigned int)mref_buf_p);
        DBG_DumpHex("", 128, mref_buf_p);
    }
#endif

    l_ipcmem_has_initialize = TRUE;
    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - L_IPCMEM_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void L_IPCMEM_Create_InterCSC_Relation(void)
{
    return;
}

/*  FUNCTION NAME : L_IPCMEM_GetPtr
 *  PURPOSE:
 *      This function will convert the offset to the logical address.
 *  INPUT:
 *      offset  --  The offset to the beginning of the ipcmem.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      The pointer to logical address of the specified offset.
 *      Return NULL if offset is invalid.
 *  NOTES:
 *      None.
 */
void* L_IPCMEM_GetPtr(UI32_T offset)
{
    return L_CVRT_GET_PTR(mem_buffer_base_addr, offset);
}

/*  FUNCTION NAME : L_IPCMEM_GetOffset
 *  PURPOSE:
 *      Get the offset to the beginning of L_IPCMEM memory address.
 *
 *  INPUT:
 *      buf_p -- the buffer allocated on the region of IPCMEM.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- the offset to the beginning of IPCMEM memory address of the given buffer
 *      Fail   -- a negative value will be returned if buf_p is invalid.
 *
 *  NOTES:
 *      None.
 */
I32_T L_IPCMEM_GetOffset(void* buf_p)
{
    I32_T offset = L_CVRT_GET_OFFSET(mem_buffer_base_addr, buf_p);

    if(buf_p==NULL)
        return -1;

    if(mem_buffer_base_addr == NULL)
    {
        SYSFUN_Debug_Printf("%s:L_IPCMEM has not been initialized\r\n", __FUNCTION__);
    }

    if(offset < 0)
    {
        SYSFUN_Debug_Printf("%s:Invalid buf_p\r\n", __FUNCTION__);
    }
    return offset;
}

/*  FUNCTION NAME : L_IPCMEM_GetPhysicalPtr
 *  PURPOSE:
 *      This function will convert the offset to the physical address.
 *  INPUT:
 *      offset  --  The offset to the beginning of the ipcmem.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      The pointer to physical address of the specified offset.
 *      Return NULL if offset is invalid.
 *  NOTES:
 *      None.
 */
void *L_IPCMEM_GetPhysicalPtr(UI32_T offset)
{
    void* physical_addr=NULL;

    SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, L_CVRT_UINT_TO_PTR(L_IPCMEM_CMD_GET_PTR), L_CVRT_UINT_TO_PTR(offset), &physical_addr, NULL, NULL);
    return physical_addr;
}

/*  FUNCTION NAME : L_IPCMEM_GetOffsetOfPhysicalPtr
 *  PURPOSE:
 *      Get the offset to the beginning of L_IPCMEM memory physical address.
 *
 *  INPUT:
 *      buf_p -- the physical address of buffer allocated on the region of IPCMEM.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- the offset to the beginning of IPCMEM memory address of the given buffer
 *      Fail   -- a negative value will be returned if buf_p is invalid.
 *
 *  NOTES:
 *      None.
 */
I32_T L_IPCMEM_GetOffsetOfPhysicalPtr(void *buf_p)
{
    return (I32_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, (void*)L_IPCMEM_CMD_GET_OFFSET, buf_p, (void*)0, (void*)0, (void*)0);
}

/*  FUNCTION NAME : L_IPCMEM_GetPhysicalBaseAddress
 *  PURPOSE:
 *      Get the physical address of the beginning of IPCMEM memory address.
 *
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      physical_base_address - Physical address of the beginning of IPCMEM.
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      TBD.
 */
BOOL_T L_IPCMEM_GetPhysicalBaseAddress(void **physical_base_address)
{
    if (physical_base_address==NULL)
        return FALSE;

    return SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, (void*)L_IPCMEM_CMD_GET_PHYSICAL_BASE_ADDRESS, physical_base_address, (void*)0, (void*)0, (void*)0);
}

/*  FUNCTION NAME : L_IPCMEM_GetTxBufStartAddress
 *  PURPOSE:
 *      Get the starting address(logical address) of the Tx buffer.
 *
 *  INPUT:
 *      tx_buf_start_address -- the starting address of the Tx buffer
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      None.
 */
BOOL_T L_IPCMEM_GetTxBufStartAddress(void **tx_buf_start_address)
{
    if ((!l_ipcmem_has_initialize) && (!L_IPCMEM_Initialize()))
        return FALSE;

    *tx_buf_start_address = L_CVRT_GET_PTR(mem_buffer_base_addr, l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_TX]);
    return TRUE;
}

/*  FUNCTION NAME : L_IPCMEM_GetRxBufStartAddress
 *  PURPOSE:
 *      Get the starting address(logical address) of the Rx buffer.
 *
 *  INPUT:
 *      rx_buf_start_address -- the starting address of the Rx buffer
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      1.This API is for NIC to prepare its Rx buffer. Rx buffer
 *        is also located at the place just behind the buffer used by L_IPCMEM.
 *      2.On Linux, IPCMEM and Rx buffer for NIC are all located in the region
 *        of high memory.
 */
BOOL_T L_IPCMEM_GetRxBufStartAddress(void **rx_buf_start_address)
{
    if ((!l_ipcmem_has_initialize) && (!L_IPCMEM_Initialize()))
        return FALSE;

    *rx_buf_start_address = L_CVRT_GET_PTR(mem_buffer_base_addr, l_ipcmem_mem_offset_ar[L_IPCMEM_MEM_ID_RX]);
    return TRUE;
}

/*  FUNCTION NAME : L_IPCMEM_GetUCBufStartAddress
 *  PURPOSE:
 *      Get the starting address(logical address) of the UC buffer.
 *
 *  INPUT:
 *      uc_buf_start_address -- the starting address of the UC buffer
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      1.This API is for UC MGR to prepare its buffer.
 *      2.On Linux, UC buffer is located in the region of high memory.
 */
BOOL_T L_IPCMEM_GetUCBufStartAddress(void **uc_buf_start_address)
{
/* This function is obsolete now,
 * ucmgmt should use L_IPCMEM_AttachUCBuffer instead.
 */
    if (uc_mem_buffer_base_addr == NULL)
        return FALSE;

    *uc_buf_start_address = uc_mem_buffer_base_addr;
    return TRUE;
}

/*  FUNCTION NAME : L_IPCMEM_AttachUCBuffer
 *  PURPOSE:
 *      Attach the UC buffer.
 *
 *  INPUT:
 *      uc_physical_start_addr -- the physical starting address of the UC buffer
 *      uc_buf_start_address   -- the virtual starting address of the UC buffer
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      1.This API is for UC MGR to prepare its buffer.
 *      2.On Linux, UC buffer is located in the region of high memory.
 */
BOOL_T L_IPCMEM_AttachUCBuffer(UI32_T uc_physical_start_addr, UI32_T *uc_buf_start_address)
{
#if (SYS_CPNT_UCMGMT_SHMEM != TRUE)
    UI32_T uc_real_physical_addr;
    void *uc_read_virtual_addr;

    if (fd == -1)
        fd = open("/dev/mem", O_RDWR);

    if (uc_mem_buffer_base_addr == NULL)
    {
        int pagesize = getpagesize();

        uc_real_physical_addr = L_ALIGN(uc_physical_start_addr + 1 - pagesize, pagesize);
        uc_read_virtual_addr = mmap(0, L_ALIGN(SYS_HWCFG_UC_MEM_SIZE, pagesize), PROT_READ|PROT_WRITE, MAP_SHARED, fd, uc_real_physical_addr);
        if (uc_read_virtual_addr == MAP_FAILED)
        {
            printf("%s:mmap fail: %d\r\n", __FUNCTION__, errno);
            return FALSE;
        }
        uc_mem_buffer_base_addr = (void *)((UI32_T)uc_read_virtual_addr + uc_physical_start_addr - uc_real_physical_addr);
    }
    *uc_buf_start_address = (UI32_T)uc_mem_buffer_base_addr;
#endif
    return TRUE;
}

/*  FUNCTION NAME : L_IPCMEM_GetTotalSize
 *  PURPOSE:
 *      Get the total size owned by L_IPCMEM.
 *
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      total_size_p  -  total size owned by L_IPCMEM
 *
 *  RETURN:
 *      Sucess -- return TRUE
 *      Fail   -- return FALSE
 *
 *  NOTES:
 *      None.
 */
BOOL_T L_IPCMEM_GetTotalSize(UI32_T *total_size_p)
{

    if (total_size_p==NULL)
    {
        printf("%s: Invalid input arg total_size_p\r\n", __FUNCTION__);
        return FALSE;
    }

    if (!SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM, (void*)L_IPCMEM_CMD_GET_IPCMEM_TOTAL_SIZE, total_size_p, (void*)0, (void*)0, (void*)0))
    {
        printf("%s: can't get total size.\r\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T L_IPCMEM_GetIPCMemInfo(void *physical_base_addr_p,
                                     UI32_T *physical_mem_size_p,
                                     UI32_T ipcmem_size[L_IPCMEM_MEM_ID_MAX],
                                     UI32_T ipcmem_offset[L_IPCMEM_MEM_ID_MAX])
{
    return SYSFUN_Syscall(SYSFUN_SYSCALL_L_IPCMEM,
                          (void*)L_IPCMEM_CMD_GET_IPCMEM_INFO,
                          physical_base_addr_p,
                          physical_mem_size_p,
                          ipcmem_size,
                          ipcmem_offset);
}
