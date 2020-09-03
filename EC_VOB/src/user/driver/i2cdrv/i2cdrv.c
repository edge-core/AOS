/* Package Name: i2cdrv.c
 * Purpose: This package defines I2C interface driver APIs.
 * Notes: This file is used for run-time to read/write I2C interface using 
 *        interrupt mode.
 * History:
 *    05/06/2003        -- Modified, Benson
 *
 * Copyright(C)      Accton Corporation  2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_hwcfg.h"
#include "sysrsc_mgr.h"
#include "i2cdrv.h"
#include "i2c_export.h"
#include "i2c.h"
#include "sysdrv.h"

#if (SYS_HWCFG_I2C_INIT_PHERIPHERAL==TRUE) || \
    (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
#include "uc_mgr.h"
#endif

#if !defined(INCLUDE_DIAG) && defined(MARVELL_CPSS)
#include "dev_swdrv_pmgr.h"
#endif

#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
#include "uart_to_i2c.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define I2CDRV_ENTER_CRITICAL_SECTION        SYSFUN_ENTER_CRITICAL_SECTION(i2cdrv_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
#define I2CDRV_LEAVE_CRITICAL_SECTION        SYSFUN_LEAVE_CRITICAL_SECTION(i2cdrv_sem_id);
#if (SYS_CPNT_I2C==TRUE) && (SYS_HWCFG_NUM_OF_I2C_MUX > 0)
#if defined(SYS_CPNT_SELECT_CHANNEL_MODE__GPIO) && (SYS_CPNT_SELECT_CHANNEL_MODE == SYS_CPNT_SELECT_CHANNEL_MODE__GPIO)
#define I2CDRV_GPIO_LOCK()                   SYSFUN_TakeSem(i2cdrv_gpio_sem_id,SYSFUN_TIMEOUT_WAIT_FOREVER)
#define I2CDRV_GPIO_UNLOCK()                 SYSFUN_GiveSem(i2cdrv_gpio_sem_id)
#else
#define I2CDRV_GPIO_LOCK()                   ({UI32_T __rc=SYSFUN_OK; __rc;})
#define I2CDRV_GPIO_UNLOCK()                 ({UI32_T __rc=SYSFUN_OK; __rc;})
#endif

#define I2CDRV_MUX_LOCK()                    (I2CDRV_GPIO_LOCK(), \
                                              SYSFUN_TakeSem(i2cdrv_mux_sem_id,SYSFUN_TIMEOUT_WAIT_FOREVER))
#define I2CDRV_MUX_UNLOCK()                  (SYSFUN_GiveSem(i2cdrv_mux_sem_id), \
                                              I2CDRV_GPIO_UNLOCK())
#else
#define I2CDRV_MUX_LOCK() ({UI32_T __rc=SYSFUN_OK; __rc;})
#define I2CDRV_MUX_UNLOCK() ({UI32_T __rc=SYSFUN_OK; __rc;})
#endif


/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_I2C == TRUE)
static UI32_T i2cdrv_sem_id = 0;          /* I2C driver semaphore ID    */
    #if (SYS_HWCFG_NUM_OF_I2C_MUX > 0)
static UI32_T i2cdrv_mux_sem_id = 0;      /* I2C mux semaphore ID */
    #endif
static UI32_T i2cdrv_gpio_sem_id = 0;
#endif

/* STATIC FUNCTION DECLARATIONS
 */
static void I2CDRV_EnterCriticalSection(UI8_T i2c_bus_idx);
static void I2CDRV_LeaveCriticalSection(UI8_T i2c_bus_idx);

/* EXPORTED SUBPROGRAM DECLARATIONS
 */

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - I2CDRV_InitiateSystemResources                                        
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is used to initialize I2C interface, including
 *           defining I2C registers address, selecting SCL frequency and
 *           enabling I2C module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_InitiateSystemResources(void)
{
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
    UART_TO_I2C_ChipInit();
#endif
   return TRUE;
}  /* End of I2CDRV_Initiate_System_Resources() */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: I2CDRV_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for I2CDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void I2CDRV_AttachSystemResources(void)
{
#if (SYS_CPNT_I2C == TRUE)
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_I2CDRV, &i2cdrv_sem_id)!=SYSFUN_OK)
        printf("%s(%d): Failed to get semaphore.\r\n", __FUNCTION__, __LINE__);
    #if (SYS_HWCFG_NUM_OF_I2C_MUX > 0)
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_I2CDRV_MUX, &i2cdrv_mux_sem_id)!=SYSFUN_OK)
        printf("%s(%d): Failed to get semaphore.\r\n", __FUNCTION__, __LINE__);
    #endif
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &i2cdrv_gpio_sem_id)!=SYSFUN_OK)
        printf("%s(%d): Failed to get semaphore.\r\n", __FUNCTION__, __LINE__);

#if !defined(INCLUDE_DIAG) && defined(MARVELL_CPSS)
    /* On projects using MARVELL CPSS SDK, i2cdrv will call dev_swdrv_pmgr
     * api to do I2C operation in low level i2cdrv code(i.e. i2c_marvell.c),
     * call DEV_SWDRV_PMGR_InitiateProcessResource() here.
     * Diag will call dev_swdrv api in low level i2cdrv code, so it does not
     * need to call DEV_SWDRV_PMGR_InitiateProcessResource() in diag.
     */
    DEV_SWDRV_PMGR_InitiateProcessResource();

#endif

#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
    /* in UART_TO_I2C_ChipInit(), I2C_TwsiDataReadWithBusIdx()
     * and I2C_TwsiDataWriteWithBusIdx(),
     * it will call UC_MGR_GetSysInfo to get board id,
     * need to call UC_MGR_InitiateProcessResources() here.
     */
    UC_MGR_InitiateProcessResources();
    UART_TO_I2C_AttachSystemResources();
#endif

#endif /* end of #if (SYS_CPNT_I2C == TRUE) */
}

#if (SYS_CPNT_I2C == TRUE)
/* FUNCTION NAME : I2CDRV_SetAndLockMux
 * PURPOSE : This function will set the channel on the multiplexer
 *           and lock to prevent other threads to set the multiplexer
 *           at the same time.
 * INPUT   : index       - multiplexer index (0 based)
 *           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
 *                         channel 2 and so on.
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : 1.Caller must call I2C_UnLockMux after I2C transaction is done
 *           to unlock mux device.
 *           2.If return FALSE, caller do not need to call I2C_UnLockMux.
 */
BOOL_T I2CDRV_SetAndLockMux(UI32_T index, UI32_T channel_bmp)
{
    return I2CDRV_SetAndLockMuxWithFunPtr(index, channel_bmp, NULL);
}

/* FUNCTION NAME : I2CDRV_SetAndLockMuxWithFunPtr
 * PURPOSE : This function will set the channel on the multiplexer
 *           and lock to prevent other threads to set the multiplexer
 *           at the same time.
 * INPUT   : index       - multiplexer index (0 based)
 *           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
 *                         channel 2 and so on.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is
 *                         NULL.
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : 1.Caller must call I2C_UnLockMux after I2C transaction is done
 *           to unlock mux device.
 *           2.If return FALSE, caller do not need to call I2C_UnLockMux.
 */
BOOL_T I2CDRV_SetAndLockMuxWithFunPtr(UI32_T index, UI32_T channel_bmp,
                 I2CDRV_TwsiDataWrite_Func_T write_fn_p)
{
    UI32_T     rc;
    I2C_Status i2c_rc;

    rc=I2CDRV_MUX_LOCK();
    if(rc!=SYSFUN_OK)
    {
        printf("%s(%d): mux lock error rc=%lu\r\n", __FUNCTION__, __LINE__, rc);
        return FALSE;
    }

#ifdef SYS_HWCFG_PCA9548_CHANNELS_ALWAYS_SET
    channel_bmp = channel_bmp | SYS_HWCFG_PCA9548_CHANNELS_ALWAYS_SET;
#endif
    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    i2c_rc = I2C_SetMuxWithFunPtr(index, channel_bmp, write_fn_p);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);

    if(i2c_rc!=I2C_SUCCESS)
    {
        I2CDRV_MUX_UNLOCK();
    }

    return (i2c_rc==I2C_SUCCESS)?TRUE:FALSE;
}
/* FUNCTION NAME : I2CDRV_UnLockMux
 * PURPOSE : This function will unlock the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : This function must be called after I2C_SetAndLockMux() is called.
 */
BOOL_T I2CDRV_UnLockMux(UI32_T index)
{
    return I2CDRV_UnLockMuxWithFunPtr(index, NULL);
}
/* FUNCTION NAME : I2CDRV_UnLockMuxWithPtr
 * PURPOSE : This function will unlock the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is
 *                         NULL.
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : This function must be called after I2C_SetAndLockMux() is called.
 */
BOOL_T I2CDRV_UnLockMuxWithFunPtr(UI32_T index,
                 I2CDRV_TwsiDataWrite_Func_T write_fn_p)
{
    UI32_T rc, channel_bmp = 0;

#ifdef SYS_HWCFG_PCA9548_CHANNELS_ALWAYS_SET
    channel_bmp = channel_bmp | SYS_HWCFG_PCA9548_CHANNELS_ALWAYS_SET;
#endif
    /* close all channels on the mux,
     * sfp eeprom addr 0x50 might exists among different mux
     * if channels are not close on the mux, there might
     * be more than 1 sfp eeprom addr 0x50 exsits on I2C bus
     */
    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    I2C_SetMuxWithFunPtr(index, channel_bmp, write_fn_p);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);

    rc=I2CDRV_MUX_UNLOCK();
    if(rc!=SYSFUN_OK)
    {
        printf("%s(%d): mux unlock error rc=%lu\r\n", __FUNCTION__, __LINE__, rc);
        return FALSE;
    }

    return TRUE;
}
/* FUNCTION NAME : I2CDRV_LockMux
 * PURPOSE : This function will lock the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : Do not call this function together with I2CDRV_SetAndLockMux()
 */
BOOL_T I2CDRV_LockMux(UI32_T index)
{
    UI32_T rc;

    rc=I2CDRV_MUX_LOCK();
    if(rc!=SYSFUN_OK)
    {
        printf("%s(%d): mux lock error rc=%lu\r\n", __FUNCTION__, __LINE__, rc);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - I2CDRV_SetChannel                                        
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is used to set channel on I2C multiplexer.
 * INPUT   : channel_addr    : slave address of I2C multiplexer.
 *           channel_regaddr : offset on I2C address mapping
 *           channel_num     : the channel number to be set on I2C multiplexer
 *                             (1 based)
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : This function is going to be obsoleted. Do not call
 *           this function for new code.
 *           Please use I2CDRV_SetAndLockMux() and I2CDRV_UnLockMux().
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_SetChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num)
{

    I2C_Status status;

    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    status = I2C_SetChannel(channel_addr, channel_regaddr, channel_num);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);

    if (status == I2C_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

   return FALSE;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - I2CDRV_GetI2CInfo                                        
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is used to read information from I2C module.
 * INPUT   : slave_addr  : slave address
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be read
 *           info        : the data, read from I2C module
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_GetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info)
{

    I2C_Status status;

    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    status = I2C_Transaction(I2C_MASTER_RCV, slave_addr, (UI8_T)offset, size, (char*)info);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);

    if (status == I2C_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

   return FALSE;
} /* End of I2CDRV_GetI2CInfo() */

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - I2CDRV_SetI2CInfo                                        
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is used to write information to I2C module.
 * INPUT   : slave_addr  : slave address 
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be writen
 *           info        : the data, written to I2C module
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_SetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info)
{

    I2C_Status status;

    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    status = I2C_Transaction(I2C_MASTER_XMIT, slave_addr, (UI8_T)offset, size, (char*)info);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);

    if (status == I2C_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

   return FALSE;

} /* End of I2CDRV_SetI2CInfo() */

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_TwsiDataRead
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to read data from TWSI (I2C)
 * INPUT   : dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data_len       - length of data
 * OUTPUT  : data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2CDRV_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
    BOOL_T ret;

    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    ret=I2C_TwsiDataRead(dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);
    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_TwsiDataReadWithBusIdx
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to read data from TWSI (I2C)
 * INPUT   : i2c_bus_idx    - i2c bus index
 *           dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data_len       - length of data
 * OUTPUT  : data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2CDRV_TwsiDataReadWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
    BOOL_T ret;
    I2CDRV_EnterCriticalSection(i2c_bus_idx);
    ret=I2C_TwsiDataReadWithBusIdx(i2c_bus_idx, dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);
    I2CDRV_LeaveCriticalSection(i2c_bus_idx);
    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_TwsiDataWrite
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to write data to TWSI (I2C)
 * INPUT   : dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data           - data to be written
 *           data_len       - length of data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2CDRV_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
    BOOL_T ret;

    I2CDRV_EnterCriticalSection(I2C_DEFAULT_BUS_IDX);
    ret=I2C_TwsiDataWrite(dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
    I2CDRV_LeaveCriticalSection(I2C_DEFAULT_BUS_IDX);
    return ret;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_TwsiDataWriteWithBusIdx
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to write data to TWSI (I2C)
 * INPUT   : i2c_bus_idx    - i2c bus index
 *           dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data           - data to be written
 *           data_len       - length of data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2CDRV_TwsiDataWriteWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
    BOOL_T ret;
    I2CDRV_EnterCriticalSection(i2c_bus_idx);
    ret=I2C_TwsiDataWriteWithBusIdx(i2c_bus_idx, dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
    I2CDRV_LeaveCriticalSection(i2c_bus_idx);
    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_GetI2CInfoWithBusIdx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to read information from I2C module.
 * INPUT   : i2c_bus_idx : i2c bus index
 *           slave_addr  : slave address
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be read
 * OUTPUT  : info        : the data, read from I2C module
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_GetI2CInfoWithBusIdx(UI8_T  i2c_bus_idx,
                                   UI8_T  slave_addr,
                                   UI16_T offset,
                                   UI8_T  size,
                                   UI8_T  *info)
{
    I2C_Status status;

    I2CDRV_EnterCriticalSection(i2c_bus_idx);
    status = I2C_TransactionWithBusIdx(i2c_bus_idx, I2C_MASTER_RCV, slave_addr, (UI8_T)offset, size, (char*)info);
    I2CDRV_LeaveCriticalSection(i2c_bus_idx);

    if (status == I2C_SUCCESS)
    {
        return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_SetI2CInfoWithBusIdx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to write information to I2C module.
 * INPUT   : i2c_bus_idx : i2c bus index
 *           slave_addr  : slave address
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be writen
 *           info        : the data, written to I2C module
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_SetI2CInfoWithBusIdx(UI8_T  i2c_bus_idx,
                                   UI8_T  slave_addr, 
                                   UI16_T offset, 
                                   UI8_T  size, 
                                   UI8_T  *info)
{
    I2C_Status status;

    I2CDRV_EnterCriticalSection(i2c_bus_idx);
    status = I2C_TransactionWithBusIdx(i2c_bus_idx, I2C_MASTER_XMIT, slave_addr, (UI8_T)offset, size, (char*)info);
    I2CDRV_LeaveCriticalSection(i2c_bus_idx);

    if (status == I2C_SUCCESS)
    {
        return TRUE;
    }
    return FALSE;

}

#if (SYS_HWCFG_I2C_INIT_PHERIPHERAL==TRUE)
/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_InitPeripheral
 * -----------------------------------------------------------------------------
 * FUNCTION: This function will do initialization to hardware pherial through
 *           I2C bus.
 * INPUT   : read_fn_p  -  Function pointer for doing I2C read operation. Use
 *                         default I2C read function if this argument is NULL.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is NULL.
 * RETURN  : None
 * NOTE    : 1. This function is only called when SYS_HWCFG_I2C_INIT_PHERIPHERAL
 *              is TRUE.
 *           2. SYS_HWCFG_GetI2CPeripheralInitOpers() must be implemented when
 *              SYS_HWCFG_I2C_INIT_PHERIPHERAL is TRUE.
 * -----------------------------------------------------------------------------
 */
void I2CDRV_InitPeripheral(I2CDRV_TwsiDataReadWithBusIdx_Func_T read_fn_p, I2CDRV_TwsiDataWriteWithBusIdx_Func_T write_fn_p)
{
    const SYS_HWCFG_I2CTransactionEntry_T *i2c_trans_lst_p;
    UC_MGR_Sys_Info_T uc_sys_info;
    UI32_T idx, lst_size;
    UI8_T  data;
    BOOL_T rc;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        printf("%s(%d)Get UC System Information Fail.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    if (SYS_HWCFG_GetI2CPeripheralInitOpers(uc_sys_info.board_id, &i2c_trans_lst_p, &lst_size)==FALSE)
    {
        printf("%s(%d)Get I2C init opration list failed.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    if (read_fn_p==NULL)
    {
        read_fn_p = &I2CDRV_TwsiDataReadWithBusIdx;
    }

    if (write_fn_p==NULL)
    {
        write_fn_p = &I2CDRV_TwsiDataWriteWithBusIdx;
    }

    for(idx=0; idx<lst_size; idx++)
    {
        rc=read_fn_p(i2c_trans_lst_p[idx].i2c_bus_idx,
               i2c_trans_lst_p[idx].dev_addr, i2c_trans_lst_p[idx].trans_type,
               i2c_trans_lst_p[idx].validOffset, i2c_trans_lst_p[idx].offset,
               i2c_trans_lst_p[idx].offset_more_than_256, 1, &data);
        if(rc==TRUE)
        {
            data &= ~(i2c_trans_lst_p[idx].data_mask);
            data |= i2c_trans_lst_p[idx].data;
            rc=write_fn_p(i2c_trans_lst_p[idx].i2c_bus_idx,
                   i2c_trans_lst_p[idx].dev_addr, i2c_trans_lst_p[idx].trans_type,
                   i2c_trans_lst_p[idx].validOffset, i2c_trans_lst_p[idx].offset,
                   i2c_trans_lst_p[idx].offset_more_than_256, &data, 1);
            if(rc==FALSE)
            {
                printf("%s(%d): Write failed: i2c bus=0x%u i2c addr=0x%x data=0x%x data_mask=0x%x", __FUNCTION__, __LINE__,
                i2c_trans_lst_p[idx].i2c_bus_idx, i2c_trans_lst_p[idx].dev_addr, i2c_trans_lst_p[idx].data, i2c_trans_lst_p[idx].data_mask);
                if(i2c_trans_lst_p[idx].validOffset==TRUE)
                    printf(" offset=0x%lx\r\n", i2c_trans_lst_p[idx].offset);
                else
                    printf("\r\n");
            }
        }
        else
        {
            printf("%s(%d): Read failed: i2c bus=%u i2c addr=0x%x data=0x%x data_mask=0x%x", __FUNCTION__, __LINE__,
                i2c_trans_lst_p[idx].i2c_bus_idx, i2c_trans_lst_p[idx].dev_addr, i2c_trans_lst_p[idx].data, i2c_trans_lst_p[idx].data_mask);
            if(i2c_trans_lst_p[idx].validOffset==TRUE)
                printf(" offset=0x%lx\r\n", i2c_trans_lst_p[idx].offset);
            else
                printf("\r\n");

        }
    }
}
#endif /* end of #if (SYS_HWCFG_I2C_INIT_PHERIPHERAL==TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */
/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_EnterCriticalSection
 * -----------------------------------------------------------------------------
 * FUNCTION: This function will do entering critical section operation.
 * INPUT   : i2c_bus_idx - index of the I2C bus
 * RETURN  : None
 * NOTE    : On marvell project, i2c bus operation is done through API in
 *           marvell SDK, thus the i2c bus operation is performed in driver
 *           process context. SYS_CPNT_I2CDRV_BUS0_RUN_IN_DRIVER_PROC_CONTEXT
 *           need to be defined as TRUE in this case. The i2c transaction is
 *           protected by semaphore in dev_swdrv.c, no need to get a semaphore
 *           again in I2CDRV.c. Besides, it is possible to have dead lock
 *           condition if get a semaphore again in I2CDRV.c.
 *           Here is an example of dead lock case:
 *           1. SYSDRV thread call I2CDRV API to do I2C read (I2CDRV semaphore got by SYSDRV thread)
 *             1.a SYSDRV thread send IPC to DRIVER_GROUP thread and wait for response.
 *
 *           2. DRIVER_GROUP thread is handling a IPC request for DEV_SWDRV_SetPortSfpTxEnable()
 *              which will call I2CDRV API to do I2C read and write (Blocked by I2CDRV semaphore)
 *             2.a DRIVER_GROUP thread will never get I2CDRV semaphore because it
 *               is locked by SYSDRV thread.
 * -----------------------------------------------------------------------------
 */
static void I2CDRV_EnterCriticalSection(UI8_T i2c_bus_idx)
{
#if (SYS_CPNT_I2CDRV_BUS0_RUN_IN_DRIVER_PROC_CONTEXT==TRUE)
    if (i2c_bus_idx==0)
        return;
#endif
    I2CDRV_ENTER_CRITICAL_SECTION;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2CDRV_LeaveCriticalSection
 * -----------------------------------------------------------------------------
 * FUNCTION: This function will do leaving critical section operation.
 * INPUT   : i2c_bus_idx - index of the I2C bus
 * RETURN  : None
 * NOTE    : Please refer to note in I2CDRV_EnterCriticalSection().
 * -----------------------------------------------------------------------------
 */
static void I2CDRV_LeaveCriticalSection(UI8_T i2c_bus_idx)
{
#if (SYS_CPNT_I2CDRV_BUS0_RUN_IN_DRIVER_PROC_CONTEXT==TRUE)
    if (i2c_bus_idx==0)
        return;
#endif
    I2CDRV_LEAVE_CRITICAL_SECTION;
}

#else /* #if (SYS_CPNT_I2C == TRUE) */
BOOL_T I2CDRV_SetAndLockMux(UI32_T index, UI32_T channel_bmp){return FALSE;}
BOOL_T I2CDRV_UnLockMux(UI32_T index){return FALSE;}
BOOL_T I2CDRV_GetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info){return FALSE;}
BOOL_T I2CDRV_SetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info){return FALSE;}
BOOL_T I2CDRV_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data){return FALSE;}
BOOL_T I2CDRV_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len){return FALSE;}
BOOL_T I2CDRV_GetI2CInfoWithBusIdx(UI8_T  i2c_bus_idx, UI8_T  slave_addr, UI16_T offset, UI8_T  size, UI8_T  *info){return FALSE;}
BOOL_T I2CDRV_SetI2CInfoWithBusIdx(UI8_T  i2c_bus_idx, UI8_T  slave_addr, UI16_T offset, UI8_T  size, UI8_T  *info){return FALSE;}
static void I2CDRV_EnterCriticalSection(UI8_T i2c_bus_idx){;}
static void I2CDRV_LeaveCriticalSection(UI8_T i2c_bus_idx){;}
#endif /* #if (SYS_CPNT_I2C == TRUE) */

