/* MODULE NAME:  uart_to_i2c.h
 * PURPOSE:
 *  This module provides the functions of doing I2C operations through UART to
 *  I2C chip.
 *  The chips that are supported so far in this module are shown below:
 *      SC18IM700
 * 
 * NOTES:
 * HISTORY
 *    6/5/2012 - Charlie Chen, Created
 *
 * Copyright(C)      Edge-Core Networks Corporation, 2011
 */
#ifndef UART_TO_I2C_H
#define UART_TO_I2C_H

/* INCLUDE FILE DECLARATIONS
 */
#include "i2c_export.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: UART_TO_I2C_ChipInit
 *-----------------------------------------------------------------------------
 * PURPOSE: Do chip initialization.
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T UART_TO_I2C_ChipInit(void);

/* FUNCTION NAME: UART_TO_I2C_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Attach system resource for I2CDRV in the context of the calling
 *          process.
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void UART_TO_I2C_AttachSystemResources(void);

/* FUNCTION NAME: UART_TO_I2C_TwsiDataRead
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to read data from TWSI (I2C)
 *-----------------------------------------------------------------------------
 * INPUT   : i2c_bus_idx    - i2c bus index
 *           dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data_len       - length of data
 * OUTPUT  : data
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T UART_TO_I2C_TwsiDataRead(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data);

/* FUNCTION NAME: UART_TO_I2C_TwsiDataWrite
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to write data to TWSI (I2C)
 *-----------------------------------------------------------------------------
 * INPUT   : i2c_bus_idx    - i2c bus index
 *           dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data           - data to be written
 *           data_len       - length of data
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T UART_TO_I2C_TwsiDataWrite(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len);

/* FUNCTION NAME: UART_TO_I2C_ReadRegister
 *-----------------------------------------------------------------------------
 * PURPOSE: Read the given register of the UART TO I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : reg_addr   -  The address of the register to be read
 * OUTPUT  : reg_val_p  -  The value of the given register read from the chip
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: None.
 */
BOOL_T UART_TO_I2C_ReadRegister(UI8_T reg_addr, UI8_T *reg_val_p);

/* FUNCTION NAME: UART_TO_I2C_WriteRegister
 *-----------------------------------------------------------------------------
 * PURPOSE: Write the given value to the specified register of the
 *          UART TO I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : reg_addr   -  The address of the register to be written
 *           reg_val    -  The value of the value to be written to the register
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: None.
 */
BOOL_T UART_TO_I2C_WriteRegister(UI8_T reg_addr, UI8_T reg_val);

/* FUNCTION NAME: UART_TO_I2C_SetBaudrate
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the baudrate of the UART TO I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : baudrate  -  baudrate of the UART TO I2C chip to be set.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: Caller must change the bardrate of the UART which is connected to the
 *        UART TO I2C chip after calling this function to ensure the baudrate
 *        between these two UART is the same.
 */
BOOL_T UART_TO_I2C_SetBaudrate(UI32_T baudrate);

/* FUNCTION NAME: UART_TO_I2C_ReadFromUART
 *-----------------------------------------------------------------------------
 * PURPOSE: Read the given data from the UART which is connected to the UART TO
 *          I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : len_p   - the length of data_p in byte
 * OUTPUT  : len_p   - the length of output data in data_p in byte
 *           data_p  - the buffer to keep the data from the UART
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: None.
 */
BOOL_T UART_TO_I2C_ReadFromUART(UI8_T* len_p, UI8_T *data_p);

/* FUNCTION NAME: UART_TO_I2C_SetUARTBaudRate
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set the given baudrate to the UART
 *          which is connected to a UART TO I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : baudrate - the baudrate to be set to the UART
 * OUTPUT  : None.
 * RETURN  : None.
 *-----------------------------------------------------------------------------
 * NOTES: None.
 */
void UART_TO_I2C_SetUARTBaudRate(UI32_T baudrate);

#endif    /* End of UART_TO_I2C_H */

