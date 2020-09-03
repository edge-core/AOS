/* Package Name: i2cdrv.h
 * Purpose: This package defines I2C interface driver APIs.
 * Notes: This file is used for run-time to read/write I2C interface using
 *        interrupt mode.
 * History:
 *    05/06/2003        -- Modified, Benson
 *
 * Copyright(C)      Accton Corporation  2002
 */
#ifndef I2CDRV_H
#define I2CDRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "i2c_export.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef BOOL_T (*I2CDRV_TwsiDataRead_Func_T)(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);
typedef BOOL_T (*I2CDRV_TwsiDataWrite_Func_T)(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);
typedef BOOL_T (*I2CDRV_TwsiDataReadWithBusIdx_Func_T)(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);
typedef BOOL_T (*I2CDRV_TwsiDataWriteWithBusIdx_Func_T)(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

/* EXPORTED SUBPROGRAM DECLARATIONS
 */

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - I2CDRV_Initiate_System_Resources                                        
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is used to initialize I2C interface, including
 *           defining I2C registers address, selecting SCL frequency and
 *           enabling I2C module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_InitiateSystemResources(void);

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
void I2CDRV_AttachSystemResources(void);

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
BOOL_T I2CDRV_SetAndLockMux(UI32_T index, UI32_T channel_bmp);

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
                 I2CDRV_TwsiDataWrite_Func_T write_fn_p);

/* FUNCTION NAME : I2CDRV_UnLockMux
 * PURPOSE : This function will unlock the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : This function must be called after I2C_SetAndLockMux() is called.
 */
BOOL_T I2CDRV_UnLockMux(UI32_T index);

/* FUNCTION NAME : I2CDRV_UnLockMuxWithFunPtr
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
                 I2CDRV_TwsiDataWrite_Func_T write_fn_p);

/* FUNCTION NAME : I2CDRV_LockMux
 * PURPOSE : This function will lock the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 * OUTPUT  :
 * RETUEN  : TRUE if success.
 * NOTES   : Do not call this function together with I2CDRV_SetAndLockMux()
 */
BOOL_T I2CDRV_LockMux(UI32_T index);

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
BOOL_T I2CDRV_SetChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num);

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - I2CDRV_GetI2CInfo                                        
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is used to read information from I2C module.
 * INPUT   : slave_addr  : slave address
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be read
 * OUTPUT  : info        : the data, read from I2C module
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T I2CDRV_GetI2CInfo(UI8_T  slave_addr, 
                         UI16_T offset, 
                         UI8_T  size, 
                         UI8_T  *info);

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
BOOL_T I2CDRV_SetI2CInfo(UI8_T  slave_addr, 
                         UI16_T offset, 
                         UI8_T  size, 
                         UI8_T  *info);

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
BOOL_T I2CDRV_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);

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
BOOL_T I2CDRV_TwsiDataReadWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);

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
BOOL_T I2CDRV_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

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
BOOL_T I2CDRV_TwsiDataWriteWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

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
                                   UI8_T  *info);

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
                                   UI8_T  *info);

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
void I2CDRV_InitPeripheral(I2CDRV_TwsiDataReadWithBusIdx_Func_T read_fn_p, I2CDRV_TwsiDataWriteWithBusIdx_Func_T write_fn_p);

#endif  /* I2CDRV_H */

