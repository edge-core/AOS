/* Module Name: i2c.h
 * Purpose: 
 *         The device driver interface to access I2C in linux user mode.
 *
 *
 * Notes:
 *
 * History:
 *    09/04/2007 - Echo Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef	I2C_H
#define	I2C_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "i2c_export.h"
#include "sys_hwcfg.h"
#include "stdio.h"

/* i2c channel device, add by michael.wang 2008-7-4 */
#define I2C_CHANNEL_DEVICE_ADDR                   SYS_HWCFG_I2C_PHYSICAL_ADDR_PCA9548		       
#define I2C_CHANNEL_DEVICE_DATARGADDR	            SYS_HWCFG_PCA9548_DATAADDR
#define I2C_CHANNEL_DEVICE_CHANNEL_DEFAULT         SYS_HWCFG_PCA9548_DEFAULT_CHANNEL   
#if (defined(ASF4526B_FLF_P5) || defined(ES4626F_FLF_38) )
#define I2C_CHANNEL_DEVICE_CHANNEL_RTC            SYS_HWCFG_PCA9548_CHANNEL_0     
#define I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL     SYS_HWCFG_PCA9548_CHANNEL_1    
#else
#define I2C_CHANNEL_DEVICE_CHANNEL_RTC            SYS_HWCFG_I2C_CHANNEL_RTC
#define I2C_CHANNEL_DEVICE_CHANNEL_FANTHERMAL     SYS_HWCFG_I2C_CHANNEL_FAN    
#define I2C_CHANNEL_DEVICE_CHANNEL_THERMAL        SYS_HWCFG_I2C_CHANNEL_THERMAL    
#define I2C_CHANNEL_DEVICE_CHANNEL_FAN            SYS_HWCFG_I2C_CHANNEL_FAN    
#endif

#define I2C_DEBUG_ENABLE    0
#define I2C_DEBUG_OUT       if (I2C_DEBUG_ENABLE) printf
#define I2C_BACKDOOR_OUT    printf

/* NAMING CONSTANT DECLARATIONS
 */
#define I2C_DEFAULT_BUS_IDX 0

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* Define the required info to set I2C channel through GPIO.
 * This is used in I2C_SetMux when
 * SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__GPIO
 */
typedef struct
{
    UI32_T gpio_reg_physical_addr; /* physical address of the gpio register */
    UI8_T  mask;                   /* the bit to be set will be 1 in this variable */
    UI8_T  enable_channel_val;     /* the value to be set to enable the channel */
} I2C_GpioChannelModeInfoEntry_T;

typedef BOOL_T (*I2C_TwsiDataRead_Func_T)(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);
typedef BOOL_T (*I2C_TwsiDataWrite_Func_T)(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

typedef BOOL_T (*I2C_TwsiDataReadWithBusIdx_Func_T)(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);
typedef BOOL_T (*I2C_TwsiDataWriteWithBusIdx_Func_T)(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);
typedef BOOL_T (*I2C_SetAndLockMux_Func_T)(UI32_T index, UI32_T channel_bmp);
typedef BOOL_T (*I2C_UnLockMux_Func_T)(UI32_T index);
typedef struct I2C_OpersFuncPtrs_S
{
    I2C_TwsiDataReadWithBusIdx_Func_T  i2c_read_fn_p;
    I2C_TwsiDataWriteWithBusIdx_Func_T i2c_write_fn_p;
    I2C_SetAndLockMux_Func_T           i2c_set_and_lock_mux_fn_p;
    I2C_UnLockMux_Func_T               i2c_unlock_mux_fn_p;
}I2C_OpersFuncPtrs_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: I2C_Transaction
 * PURPOSE  :  Perform the given I2C transaction, only I2C_MASTER_RCV and I2C_MASTER_XMIT
 *             are implemented.
 * INPUT    :   act        -- the type of transaction(I2C_MASTER_RCV/I2C_MASTER_XMIT)
 *              i2c_addr   -- the I2C address of the slave device
 *              data_addr  -- is the address of the data on the slave device
 *              len        -- the length of data to send or receive
 *              buffer     -- the address of the data buffer            
 * OUTPUT   : None.
 * RETURN   : I2C_SUCCESS/I2C_ERROR
 * NOTES    : This function shall be called by I2CDRV only.
 */
I2C_Status I2C_Transaction(I2C_TRANSACTION_MODE act, unsigned char i2c_addr, unsigned char data_addr, int len, char * buffer);

/* FUNCTION NAME: I2C_TransactionWithBusIdx
 * PURPOSE  :  Perform the given I2C transaction, only MASTER_XMIT and MASTER_RCV
 *             are implemented.
 * INPUT    :  i2c_bus_idx-- i2c bus index
 *             act        -- the type of transaction
 *             i2c_addr   -- the I2C address of the slave device
 *             data_addr  -- is the address of the data on the slave device
 *             len        -- the length of data to send or receive
 *             buffer     -- the address of the data buffer            
 * OUTPUT   : 
 * RETURN   : I2C_SUCCESS/I2C_ERROR
 * NOTES     : 
 */
I2C_Status I2C_TransactionWithBusIdx(UI8_T  i2c_bus_idx,
                                     I2C_TRANSACTION_MODE act,
                                     unsigned char i2c_addr, 
                                     unsigned char data_addr, 
                                     int len,
                                     char * buffer);

/* FUNCTION NAME : I2C_SetChannel
 * PURPOSE : This function set pca9548 device Channel
 * INPUT   : channel_addr ,channel_regaddr and channel_num
 * OUTPUT  : 
 * RETURN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : This function is going to be obsoleted.
 *           Please use I2C_SetMux() for new code.
 */
I2C_Status I2C_SetChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num);

/* FUNCTION NAME : I2C_SetMux
 * PURPOSE : This function will set the channel on the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 *           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
 *                         channel 2 and so on.
 * OUTPUT  : None.
 * RETURN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : This function shall be called by I2CDRV only.
 */
I2C_Status I2C_SetMux(UI32_T index, UI32_T channel_bmp);

/* FUNCTION NAME : I2C_SetMuxWithFunPtr
* PURPOSE : This function will set the channel on the multiplexer
* INPUT   : index       - multiplexer index (0 based)
*           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
*                         channel 2 and so on.
*           write_fn_p -  Function pointer for doing I2C write operation. Use
*                         default I2C write function if this argument is
*                         NULL.
* OUTPUT  : None.
* RETURN  : I2C_SUCCESS/I2C_ERROR
* NOTES   : This function shall be called by I2CDRV only.
*/
I2C_Status I2C_SetMuxWithFunPtr(UI32_T index, UI32_T channel_bmp, I2C_TwsiDataWrite_Func_T write_fn_p);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataRead
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
BOOL_T I2C_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataReadWithBusIdx
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
BOOL_T I2C_TwsiDataReadWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataWrite
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
BOOL_T I2C_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataWriteWithBusIdx
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
BOOL_T I2C_TwsiDataWriteWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

#endif  /* I2C_H */
