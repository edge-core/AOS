#ifndef INCLUDE_LOADER
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#endif

#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "l_cvrt.h"

#ifndef INCLUDE_LOADER
#include "sysfun.h"
#include "sysrsc_mgr.h"
#endif

#include "uc_mgr.h"

#ifndef INCLUDE_LOADER
#if (SYS_CPNT_SDRAM_AUTO_DETECTION == TRUE)
#include "phyaddr_access.h"
#endif
#include "l_ipcmem.h"
#endif

/* NAMING CONSTANT
 */

#define SIG_LEN            8
#define UC_MGR_MIN_POINTER_INDEX    0
#define UC_MGR_MAX_POINTER_INDEX    (UC_MGR_MIN_POINTER_INDEX + SYS_ADPT_MAX_UC_BUFFER_POINT_INDEX_NUM - 1)

/* DATA TYPE DECLARACTION
 */
typedef struct
{
   char   signature [ SIG_LEN ] ;
   size_t free_offset;
   size_t offset[ SYS_ADPT_MAX_UC_BUFFER_POINT_INDEX_NUM ];
   char data[0];
} UC_MEM_Header_T;


/* STATIC LOCAL VARIABLES
 */
#define UC_MGR_DEBUG       FALSE

#if (SYS_CPNT_SDRAM_AUTO_DETECTION == TRUE)
static size_t               uc_mgr_sdram_size=0;
#endif
static uintptr_t            uc_mgr_uc_mem_start_addr=0;
static uintptr_t            uc_mgr_uc_mem_end_addr=0;
static UC_MEM_Header_T      *header = NULL;
static const char           signature[] = "_UC_MGR_";
static const char           dirty_signature[] = "_RGM_CU_"; /* to indicate that UC structure might be different after reload */
#ifndef INCLUDE_LOADER
static UI32_T               uc_mgr_sem_id=0;   /* semaphore id */
static void                 *shmem_data_p=NULL;
#endif

/* MACRO FUNCTIONS DECLARACTION
 */
#ifndef INCLUDE_LOADER
#define UC_MGR_LOCK()       SYSFUN_ENTER_CRITICAL_SECTION(uc_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define UC_MGR_UNLOCK()     SYSFUN_LEAVE_CRITICAL_SECTION(uc_mgr_sem_id)
#else
#define UC_MGR_LOCK()
#define UC_MGR_UNLOCK()
#endif

#if (UC_MGR_DEBUG == TRUE)
    #ifndef INCLUDE_LOADER
        #define UC_MGR_DEBUG_LINE(...)   printf(__VA_ARGS__);fflush(stdout);
    #else
        #define UC_MGR_DEBUG_LINE(...)   printf(__VA_ARGS__);
    #endif
#else
#define UC_MGR_DEBUG_LINE(...)
#endif

/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T UC_MGR_Init(void);
static BOOL_T UC_MGR_IsValidDataPointer(void *ptr_addr, UI32_T size);

/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: UC_MGR_InitiateProcessResources
 * PURPOSE: This function is used to initialize the un-cleared memory management module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the ucmgmt module and create semphore.
 *          If the signature is correct, don't init the uc memory and use old one,
 *          this case may be warmboot, or watchdog reset. We need to keep the log information
 *          and store the log to logfile after reboot.
 *
 */
unsigned long g_dbg_buf;

void UC_MGR_AttachSystemResources(void)
{
    shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_UC_SHMEM_SEGID);
    UC_MGR_Init();
    return;
}

void UC_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_UC_SHMEM_SEGID;
    *seglen_p = SYS_HWCFG_UC_MEM_SIZE;
}

static BOOL_T UC_MGR_Init(void)
{
#ifndef INCLUDE_LOADER
    /* create semaphore
     */
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_UC_MGR, &uc_mgr_sem_id) != SYSFUN_OK)
    {
        perror("\r\nCreate uc_mgr_sem_id failed.");
        return(FALSE);
    }
#endif
    /* Loader can use physical address,
     * so get uc_mgr_uc_mem_start_addr directly.
     */
#if (SYS_CPNT_SDRAM_AUTO_DETECTION == TRUE)
    /* get uc_mgr_sdram_size */
    UC_MGR_GetSdramSize();

    /* SYS_HWCFG_UC_MEM_END_ADDR */
    uc_mgr_uc_mem_end_addr      = (SYS_HWCFG_DRAM_BASE_ADDR + uc_mgr_sdram_size - 1);

    /* SYS_HWCFG_UC_MEM_START_ADDR */
    uc_mgr_uc_mem_start_addr    = (uc_mgr_uc_mem_end_addr + 1 - SYS_HWCFG_UC_MEM_SIZE);
#else
    uc_mgr_uc_mem_start_addr = SYS_HWCFG_UC_MEM_START_ADDR;
    uc_mgr_uc_mem_end_addr = SYS_HWCFG_UC_MEM_END_ADDR;
    UC_MGR_DEBUG_LINE("%s: physical start_addr=0x%08x physical end_addr=0x%08x\r\n",
        __FUNCTION__, uc_mgr_uc_mem_start_addr, uc_mgr_uc_mem_end_addr);
#endif /* SYS_CPNT_SDRAM_AUTO_DETECTION */


#ifndef INCLUDE_LOADER
#if (SYS_CPNT_UCMGMT_SHMEM == TRUE)
    /* here is workaround to avoid invalid memory space on x64.
     * use shared memory instead of himem.
     */
    if (shmem_data_p==NULL)
    {
        printf("%s(%d):Initiate uc_mgr_uc_mem_start_addr failed.\r\n",
            __FUNCTION__, __LINE__);
        return(FALSE);
    }

    uc_mgr_uc_mem_start_addr = (uintptr_t)shmem_data_p;
    uc_mgr_uc_mem_end_addr = uc_mgr_uc_mem_start_addr + SYS_HWCFG_UC_MEM_SIZE - 1;
#else /* SYS_CPNT_UCMGMT_SHMEM */
    /* get uc uc_mgr_uc_mem_start_addr via l_ipcmem
     */
    if (!L_IPCMEM_AttachUCBuffer(uc_mgr_uc_mem_start_addr, &uc_mgr_uc_mem_start_addr))
    {
        perror("\r\nInitiate uc_mgr_uc_mem_start_addr failed.");
        return(FALSE);
    }
    uc_mgr_uc_mem_end_addr = uc_mgr_uc_mem_start_addr + SYS_HWCFG_UC_MEM_SIZE - 1;
#endif /* SYS_CPNT_UCMGMT_SHMEM */
#endif /* INCLUDE_LOADER */

    header = (UC_MEM_Header_T *)uc_mgr_uc_mem_start_addr;
    UC_MGR_DEBUG_LINE("\r\n%s: UC_MEM_Header_T: %08x\r\n", __FUNCTION__, (unsigned int)header);

    if ( memcmp ( header->signature, signature, SIG_LEN ) == 0 )
    {
        g_dbg_buf = (unsigned long)(header->offset[UC_MGR_DBG_BUF] + (unsigned char *)header);
        return TRUE;
    }
    /* First time be initiated from power on */
    /* 1. clear memory */
    memset ( header, 0, SYS_HWCFG_UC_MEM_SIZE );

    /* 2. Initiate header */
    memmove ( header->signature, signature, SIG_LEN );
    header->free_offset = L_CVRT_GET_OFFSET(header, header->data);

    g_dbg_buf = (unsigned long)(header->offset[UC_MGR_DBG_BUF] + (unsigned char *)header);

    return TRUE;
} /* End of UC_MGR_Init() */

BOOL_T UC_MGR_InitiateProcessResources (void)
{
#if (SYS_CPNT_UCMGMT_SHMEM == TRUE)
    /* When SYS_CPNT_UCMGMT_SHMEM is TRUE, the initialization operations
     * will be done by SYSRSC_MGR. Do nothing here.
     */
    return TRUE;
#else
    return UC_MGR_Init();
#endif
} 

BOOL_T UC_MGR_InitiateSystemResources(void)
{
#if (SYS_CPNT_UCMGMT_SHMEM == TRUE)
    shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_UC_SHMEM_SEGID);
    memset(shmem_data_p, 0, SYS_HWCFG_UC_MEM_SIZE);
#endif
    return UC_MGR_Init();
}

/* FUNCTION NAME: UC_MGR_Allocate
 * PURPOSE: This function is used to allocate memory from uc memory for need module.
 * INPUT:   alloc_index -- each data object must correspond a unique index value,
 *                         vaule range: 0 .. (SYS_ADPT_MAX_UC_BUFFER_POINT_INDEX_NUM-1).
 *          alloc_size  -- bytes to allocate.
 *          boundary    -- must order of 2
 * OUTPUT:  None.
 * RETUEN:  not 0   -- successful.
 *          0       -- unspecified failure, no free memory.
 * NOTES:   Must be called after UC_MGR_InitiateProcessResources.
 *
 */
void *UC_MGR_Allocate(UI32_T alloc_index, UI32_T alloc_size, UI32_T boundary)
{
    /* LOCAL VARIABLES */
    void *ptr;

    /* Check the alloc index */
    if (alloc_index > UC_MGR_MAX_POINTER_INDEX)
        return NULL;

    /* Check the pointer address and end of data is excess the uc memory end address */
    if (header->offset[alloc_index])
    {
        ptr = L_CVRT_GET_PTR(header, header->offset[alloc_index]);
        if (UC_MGR_IsValidDataPointer(ptr, alloc_size))
        {
            UC_MGR_DEBUG_LINE("%s: alloc: %08x\n", __FUNCTION__, (unsigned int)ptr);
            return ptr;
        }
    }

    /* Have not allocated */
    UC_MGR_LOCK();

    ptr = (void*)L_ALIGN(L_CVRT_GET_PTR(header, header->free_offset), boundary);
    if (!UC_MGR_IsValidDataPointer(ptr, alloc_size))
    {
        UC_MGR_UNLOCK();
        return NULL;
    }

    header->offset[alloc_index] = L_CVRT_GET_OFFSET(header, ptr);

    header->free_offset = header->offset[alloc_index] + alloc_size;

    UC_MGR_UNLOCK();

    UC_MGR_DEBUG_LINE("%s: alloc: %08x\n", __FUNCTION__, (unsigned int)ptr);
    return ptr;
} /* End of UC_MGR_Allocate() */


/* FUNCTION NAME: UC_MGR_GetSysInfo
 * PURPOSE: This function is used to get system information from uc memory.
 * INPUT:   *sys_info   -- output buffer of the system information.
 * OUTPUT:  *sys_info   -- the system information.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by stktplg.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_GetSysInfo(UC_MGR_Sys_Info_T *sys_info)
{
    UI32_T  tid;
    char    task_name_ar[SYSFUN_TASK_NAME_LENGTH]={0};

    if (header == NULL)
    {
        tid = SYSFUN_TaskIdSelf();
        SYSFUN_TaskIDToName(tid, task_name_ar, sizeof(task_name_ar));
        printf("%s(%d): %s need to call UC_MGR_InitiateProcessResources() first.\r\n", __FUNCTION__, __LINE__, task_name_ar);
        return FALSE;
    }

    if (header->offset[UC_MGR_SYS_INFO_INDEX] == 0)
        return FALSE; /* Memory not be allocated now */

    /* Return the system information from uc memory */
    UC_MGR_LOCK();
    *sys_info = *(UC_MGR_Sys_Info_T *)L_CVRT_GET_PTR(header, header->offset[UC_MGR_SYS_INFO_INDEX]);
    UC_MGR_UNLOCK();

    return TRUE;
} /* End of UC_MGR_GetSysInfo() */

/* FUNCTION NAME: UC_MGR_SetSysInfo
 * PURPOSE: This function is used to set system information to uc memory.
 * INPUT:   sys_info    -- setting value of the system information.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by prom code/load code.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_SetSysInfo(UC_MGR_Sys_Info_T sys_info)
{
    UI32_T  i, sys_info_db_size = sizeof (UC_MGR_Sys_Info_T);
    UI16_T  *ptr = (UI16_T *)&sys_info;

    if (header->offset[UC_MGR_SYS_INFO_INDEX] == 0)
        return FALSE; /* Memory not be allocated now */

    /* Re-calculate the checksum value here
     * What checksum algorithm does we use to calculate??
     */

    /* setting the system information to uc memory */
    UC_MGR_LOCK();

    /* Calculate the check_sum value,  2001/12/28 */
    sys_info.check_sum = 0;
    for (i = 0; i < (sys_info_db_size/2)-1; i++)
    {
        sys_info.check_sum += ptr[i];
    }

    *(UC_MGR_Sys_Info_T *)L_CVRT_GET_PTR(header, header->offset[UC_MGR_SYS_INFO_INDEX]) = sys_info;

    UC_MGR_UNLOCK();

    return TRUE;
} /* End of UC_MGR_SetSysInfo() */


/* FUNCTION NAME: UC_MGR_GetDownloadInfo
 * PURPOSE: This function is used to get download information from uc memory.
 * INPUT:   *download_info   -- output buffer of the download information.
 * OUTPUT:  *download_info   -- the download information.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by download module.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_GetDownloadInfo(UC_MGR_Download_Info_T *download_info)
{
    if (header->offset[UC_MGR_DOWNLOAD_INFO_INDEX] == 0)
        return FALSE; /* Memory not be allocated now */

    /* Return the download information from uc memory */
    UC_MGR_LOCK();
    *download_info = *(UC_MGR_Download_Info_T *)L_CVRT_GET_PTR(header, header->offset[UC_MGR_DOWNLOAD_INFO_INDEX]);
    UC_MGR_UNLOCK();

    return TRUE;
} /* End of UC_MGR_GetDownloadInfo() */


/* FUNCTION NAME: UC_MGR_SetDownloadInfo
 * PURPOSE: This function is used to set download information to uc memory.
 * INPUT:   download_info    -- setting value of the download information.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by prom code/load code.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_SetDownloadInfo(UC_MGR_Download_Info_T download_info)
{
    if (header->offset[UC_MGR_DOWNLOAD_INFO_INDEX] == 0)
        return FALSE; /* Memory not be allocated now */

    /* Re-calculate the checksum value here
     * What checksum algorithm does we use to calculate??
     */

    /* setting the system information to uc memory */
    UC_MGR_LOCK();
    *(UC_MGR_Download_Info_T *)L_CVRT_GET_PTR(header, header->offset[UC_MGR_DOWNLOAD_INFO_INDEX]) = download_info;
    UC_MGR_UNLOCK();

    return TRUE;
} /* End of UC_MGR_SetDownloadInfo() */

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/* FUNCTION NAME: UC_MGR_GetLoopbackTestResult
 * PURPOSE:       This function is used to get loopback test result, that was
 *                set by DIAG, from UC memory.
 * INPUT:         test_result --- The loopback test result in the format of port bit map.
 *                                If loopback test fail, the bit is "1", otherwise "0".
 *                                The MSB of byte 0 is port 1,
 *                                the LSB of byte 0 is port 8,
 *                                the MSB of byte 1 is port 9,
 *                                the LSB of byte 1 is port 16,
 *                                ...
 *                                and so on.
 * OUTPUT:        None.
 * RETUEN:        TRUE        --- successful
 *                FALSE       --- failure
 * NOTES:         1. This API shall only be called by SWCTRL.
 *                2. The function should be called after the memory be allocated by using
 *                   index UC_MGR_LOOPBACK_TEST_RESULT_INDEX.
 *                3. If the memory is not allocated, the function will return FALSE.
 */
BOOL_T UC_MGR_GetLoopbackTestResult(UI8_T test_result[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    if (header->offset[UC_MGR_LOOPBACK_TEST_RESULT_INDEX] == 0)
        return FALSE; /* Memory not be allocated now */

    /* Return the loopback test result from uc memory */
    UC_MGR_LOCK();
    memcpy (test_result,
            L_CVRT_GET_PTR(header, header->offset[UC_MGR_LOOPBACK_TEST_RESULT_INDEX]),
            SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    UC_MGR_UNLOCK();

    return TRUE;
}


/* FUNCTION NAME: UC_MGR_SetLoopbackTestResult
 * PURPOSE:       This function is used to set loopback test result, that wiil be
 *                got by SWCTRL, to UC memory.
 * INPUT:         None.
 * OUTPUT:        test_result --- The loopback test result in the format of port bit map.
 *                                If loopback test fail, the bit is "1", otherwise "0".
 *                                The MSB of byte 0 is port 1,
 *                                the LSB of byte 0 is port 8,
 *                                the MSB of byte 1 is port 9,
 *                                the LSB of byte 1 is port 16,
 *                                ...
 *                                and so on.
 * RETUEN:        TRUE        --- successful
 *                FALSE       --- failure
 * NOTES:         1. This API shall only be called by DIAG.
 *                2. The function should be called after the memory be allocated by using
 *                   index UC_MGR_LOOPBACK_TEST_RESULT_INDEX.
 *                3. If the memory is not allocated, the function will return FALSE.
 */
BOOL_T UC_MGR_SetLoopbackTestResult(UI8_T test_result[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    if (header->offset[UC_MGR_LOOPBACK_TEST_RESULT_INDEX] == 0)
        return FALSE; /* Memory not be allocated now */

    /* Re-calculate the checksum value here
     * What checksum algorithm does we use to calculate??
     */

    /* setting the loopback test result to uc memory */
    UC_MGR_LOCK();
    memcpy (L_CVRT_GET_PTR(header, header->offset[UC_MGR_LOOPBACK_TEST_RESULT_INDEX]),
            test_result,
            SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    UC_MGR_UNLOCK();

    return TRUE;
}
#endif

#if (SYS_CPNT_SDRAM_AUTO_DETECTION == TRUE)
/* FUNCTION NAME: UC_MGR_GetSdramSize
 * PURPOSE: This function is used to get SDRAM size.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  SDRAM size
 *
 * NOTES:   As this function is only used by loader,
 *          it can read the value in SYS_HWCFG_SDRAM_CONFIG_ADDR
 *          directly.
 *
 */
UI32_T UC_MGR_GetSdramSize(void)
{
    static  UI8_T    getOneTime = 0;
    UI8_T   sdram_val;

    if(getOneTime)
    {
        return uc_mgr_sdram_size;
    }
    getOneTime = 1;

#ifndef INCLUDE_LOADER
    {
        SYS_TYPE_VAddr_T  virtual_address;

        if (!PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_SDRAM_CONFIG_ADDR, &virtual_address) ||
            !PHYADDR_ACCESS_Read(virtual_address, 1, 1, &sdram_val))
        {
            printf("\r\n%s: Can't access SYS_HWCFG_SDRAM_CONFIG_ADDR: %08x\r\n", __FUNCTION__, SYS_HWCFG_SDRAM_CONFIG_ADDR);
            while(1);
        }
    }
#else
    sdram_val = *((volatile UI8_T*)SYS_HWCFG_SDRAM_CONFIG_ADDR);
#endif /* INCLUDE_LOADER */

    /* not support SDRAM Configuration */
    if (sdram_val == 0xff)
    {
        return uc_mgr_sdram_size = SYS_HWCFG_DRAM_SIZE;
    }

    sdram_val &= SYS_HWCFG_SDRAM_SIZE_MASK;

    switch (sdram_val)
    {
        case SYS_HWCFG_SDRAM_SIZE_4M  :
            uc_mgr_sdram_size = (4 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_8M  :
            uc_mgr_sdram_size = (8 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_16M :
            uc_mgr_sdram_size = (16 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_32M :
            uc_mgr_sdram_size = (32 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_64M :
            uc_mgr_sdram_size = (64 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_128M:
            uc_mgr_sdram_size = (128 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_256M:
            uc_mgr_sdram_size = (256 * SYS_TYPE_1M_BYTES);
            break;
        case SYS_HWCFG_SDRAM_SIZE_512M:
            uc_mgr_sdram_size = (512 * SYS_TYPE_1M_BYTES);
            break;
        default:
            uc_mgr_sdram_size = SYS_HWCFG_DRAM_SIZE;
    }

    return uc_mgr_sdram_size;

} /* End of UC_MGR_GetSdramSize */

#endif /* SYS_CPNT_SDRAM_AUTO_DETECTION */

/* FUNCTION NAME: UC_MGR_GetUcMemStartAddr
 * PURPOSE: This function is used to get UC memory start address.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  UC memory start address
 *
 * NOTES:   Before get uc memory start address
 *          UC_MGR_InitiateProcessResources need do first!!
 */
UI32_T UC_MGR_GetUcMemStartAddr(void)
{
    return uc_mgr_uc_mem_start_addr;
} /* End of UC_MGR_GetUcMemStartAddr */

/* FUNCTION NAME: UC_MGR_GetUcMemEndAddr
 * PURPOSE: This function is used to get UC memory end address.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  UC memory end address
 *
 * NOTES:   Before get uc memory end address
 *          UC_MGR_InitiateProcessResources need do first!!
 */
UI32_T UC_MGR_GetUcMemEndAddr(void)
{
    return uc_mgr_uc_mem_end_addr;
} /* End of UC_MGR_GetUcMemEndAddr */

/* FUNCTION NAME: UC_MGR_SetDirtySignature
 * PURPOSE: This function is used to change UC signature word,
 *          so that UC can be reinit after reload.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 */
void UC_MGR_SetDirtySignature(void)
{
    memcpy(header->signature, dirty_signature, sizeof(header->signature));
}

/******************/
/* Local function */
/******************/

/* FUNCTION NAME: UC_MGR_IsValidDataPointer
 * PURPOSE: This function is used to check the uc pointer valid or not.
 * INPUT:   ptr_addr    -- pointer address.
 *          size        -- buffer size.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:
 *
 */
static BOOL_T UC_MGR_IsValidDataPointer(void *ptr_addr, UI32_T size)
{
    if (((uintptr_t)ptr_addr < uc_mgr_uc_mem_start_addr) ||
        ((uintptr_t)ptr_addr + size - 1) > uc_mgr_uc_mem_end_addr)
        return FALSE;

    return TRUE;
} /* end of UC_MGR_IsValidDataPointer */
