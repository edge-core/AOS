/* Module Name: i2c_marvell.c
 * Purpose:
 *         The implementation to access i2c through marvell cpss SDK.
 *
 * Notes:
 *
 * History:
 *    06/13/2011 - Charlie Chen, Created
 *
 * Copyright(C)      Edge-Core Networks Corporation, 2011
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#ifdef INCLUDE_DIAG
#include "dev_swdrv.h"
#else
#include "dev_swdrv_pmgr.h"
#endif
#include "i2c_export.h"
#include "sys_hwcfg.h"
#include "phyaddr_access.h"
#include "i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uc_mgr.h"
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_TYPE_NONE)
#include "uart_to_i2c.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define INVALID_BOARD_ID     0xFFFFFFFFUL

#if (SYS_CPNT_SELECT_CHANNEL_MODE == SYS_CPNT_SELECT_CHANNEL_MODE__GPIO)
/* I2C_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL
 * max number of channel supported in i2c_marvell.c
 */
#define I2C_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL 4

/* emit error if SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL is larger than
 * I2C_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL
 */
#if (SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL > I2C_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL)
#error "SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL is largern than I2C_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL"
#endif

#endif /* end of #if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__GPIO) */


/* MACRO FUNCTION DECLARATIONS
 */
#ifdef INCLUDE_DIAG
#define DEV_SWDRV_PMGR_TwsiDataRead DEV_SWDRV_TwsiDataRead
#define DEV_SWDRV_PMGR_TwsiDataWrite DEV_SWDRV_TwsiDataWrite
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T I2C_GetBoardId(void);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T board_id=INVALID_BOARD_ID;

/* EXPORTED SUBPROGRAM BODIES
 */
/* quick dirty workaround to make diag build ok
 */
#if (SYS_CPNT_I2C == TRUE)
char I2C_ADAPTOR_NAME[64]="/dev/i2c-0";
#endif

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
I2C_Status I2C_Transaction(I2C_TRANSACTION_MODE act, unsigned char i2c_addr, unsigned char data_addr, int len, char * buffer)
{
    return I2C_TransactionWithBusIdx(I2C_DEFAULT_BUS_IDX, act, i2c_addr, data_addr, len, buffer);
}

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
                                     char * buffer)
{
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
    UI8_T  uart_to_i2c_i2c_bus_idx;
    BOOL_T have_uart_to_i2c, ret_val=FALSE, use_uart_to_i2c=FALSE;

    have_uart_to_i2c=SYS_HWCFG_GetI2CBusIdxOfUARTToI2C(I2C_GetBoardId(), &uart_to_i2c_i2c_bus_idx);

    if(have_uart_to_i2c==TRUE && uart_to_i2c_i2c_bus_idx==i2c_bus_idx)
    {
        use_uart_to_i2c=TRUE;
    }

    switch(act)
    {
        case I2C_MASTER_RCV:
            if(use_uart_to_i2c==TRUE)
            {
                ret_val = UART_TO_I2C_TwsiDataRead(i2c_bus_idx, i2c_addr,
                    I2C_7BIT_ACCESS_MODE, TRUE, data_addr, FALSE, len, (UI8_T*)buffer);
            }
            else
            {
                ret_val = DEV_SWDRV_PMGR_TwsiDataRead(i2c_addr,
                    I2C_7BIT_ACCESS_MODE, TRUE, data_addr, FALSE, len, (UI8_T*)buffer);
            }
            break;
        case I2C_MASTER_XMIT:
            if(use_uart_to_i2c==TRUE)
            {
                ret_val = UART_TO_I2C_TwsiDataWrite(i2c_bus_idx, i2c_addr,
                    I2C_7BIT_ACCESS_MODE, TRUE, data_addr, FALSE, (UI8_T*)buffer, len);
            }
            else
            {
                ret_val = DEV_SWDRV_PMGR_TwsiDataWrite(i2c_addr,
                    I2C_7BIT_ACCESS_MODE, TRUE, data_addr, FALSE, (UI8_T*)buffer, len);
            }
            break;
        default:
            break;
    }

    if(ret_val==FALSE)
        return I2C_ERROR;

    return I2C_SUCCESS;

#else /* #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */
    if(i2c_bus_idx!=I2C_DEFAULT_BUS_IDX)
    {
        printf("%s(%d): Not support i2c transaction on i2c bus idx %u\r\n",
            __FUNCTION__, __LINE__, i2c_bus_idx);
        return I2C_ERROR;
    }

    switch(act)
    {
        case I2C_MASTER_RCV:
            if(DEV_SWDRV_PMGR_TwsiDataRead(i2c_addr, I2C_7BIT_ACCESS_MODE, 1, data_addr, 0, len, (UI8_T*)buffer)==TRUE)
                return I2C_SUCCESS;
            else
                return I2C_ERROR;
            break;
        case I2C_MASTER_XMIT:
            if(DEV_SWDRV_PMGR_TwsiDataWrite(i2c_addr, I2C_7BIT_ACCESS_MODE, 1, data_addr, 0, (UI8_T*)buffer, len)==TRUE)
                return I2C_SUCCESS;
            else
                return I2C_ERROR;
            break;
        default:
            printf("%s():Invalid act=0x%d\r\n", __FUNCTION__, (int)act);
            break;
    }

    return I2C_ERROR;
#endif /* end of #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */
}

/* FUNCTION NAME : I2C_SetChannel
 * PURPOSE : This function set pca9548 device Channel
 * INPUT   : channel_addr ,channel_regaddr and channel_num
 * OUTPUT  :
 * RETURN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : This function is going to be obsoleted.
 *           Please use I2C_SetMux() for new code.
 */
I2C_Status I2C_SetChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num)
{
#if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE_NOCHANNEL)
    /* no channel device between I2C bus and I2C device
     */
    return I2C_SUCCESS;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__PCA9548)
    /* PCA9548 device between I2C bus and I2C device
     */
    UI8_T data;

    data = (UI8_T)(0x1<<(channel_num-1));
    return (DEV_SWDRV_PMGR_TwsiDataWrite(channel_addr, I2C_7BIT_ACCESS_MODE,
        0, 0, 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__EPLD)
    printf("%s(%d): Obsoleted API, do not use.\r\n", __FUNCTION__, __LINE__);
    return I2C_ERROR;
#else
    printf("%s(%d): Obsoleted API, do not use.\r\n", __FUNCTION__, __LINE__);
    return I2C_ERROR;
#endif
}

/* FUNCTION NAME : I2C_SetMux
 * PURPOSE : This function will set the channel on the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 *           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
 *                         channel 2 and so on.
 * OUTPUT  : None.
 * RETURN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : This function shall be called by I2CDRV only.
 */
I2C_Status I2C_SetMux(UI32_T index, UI32_T channel_bmp)
{
    return I2C_SetMuxWithFunPtr(index, channel_bmp, NULL);
}
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
I2C_Status I2C_SetMuxWithFunPtr(UI32_T index, UI32_T channel_bmp,
                 I2C_TwsiDataWrite_Func_T write_fn_p)
{
    if (write_fn_p==NULL)
    {
        write_fn_p = &DEV_SWDRV_PMGR_TwsiDataWrite;
    }

#if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE_NOCHANNEL)
    /* no channel device between I2C bus and I2C device
     */
    return I2C_SUCCESS;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__PCA9548)
    /* PCA9548 device between I2C bus and I2C device
     */
    UI8_T i2c_mux_addrs[SYS_HWCFG_NUM_OF_I2C_MUX] = SYS_HWCFG_I2C_MUX_ADDR_ARRAY;
    UI8_T data;

    if(index>=(sizeof(i2c_mux_addrs)/sizeof(i2c_mux_addrs[0])))
    {
        printf("%s(%d): Invalid index %lu\r\n", __FUNCTION__, __LINE__, index);
        return I2C_ERROR;
    }
    data = (UI8_T)channel_bmp;
    return (write_fn_p(i2c_mux_addrs[index], I2C_7BIT_ACCESS_MODE,
        0, 0, 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__PCA9544)
    /* PCA9544 device between I2C bus and I2C device
     */
    UI8_T i2c_mux_addrs[SYS_HWCFG_NUM_OF_I2C_MUX] = SYS_HWCFG_I2C_MUX_ADDR_ARRAY;
    UI8_T data;

    if(index>=(sizeof(i2c_mux_addrs)/sizeof(i2c_mux_addrs[0])))
    {
        printf("%s(%d): Invalid index %lu\r\n", __FUNCTION__, __LINE__, index);
        return I2C_ERROR;
    }

    switch(channel_bmp)
    {
        case 0:
            data=0;
            break;
        case BIT_0:
            data=4;
            break;
        case BIT_1:
            data=5;
            break;
        case BIT_2:
            data=6;
            break;
        case BIT_3:
            data=7;
            break;
        default:
            /* PCA9544 can not turn on multiple channels at the same time
             */
            printf("%s(%d): Invalid channel_bmp(0x%x)\r\n", __FUNCTION__, __LINE__,
                channel_bmp);
            return I2C_ERROR;
            break;
    }

    return (write_fn_p(i2c_mux_addrs[index], I2C_7BIT_ACCESS_MODE,
        0, 0, 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;

#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__EPLD)
    printf("%s(%d): Not support yet.\r\n", __FUNCTION__, __LINE__);
    return I2C_ERROR;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__EPLD_VIA_I2C)
    UI8_T i2c_mux_addrs[SYS_HWCFG_NUM_OF_I2C_MUX] = SYS_HWCFG_I2C_MUX_ADDR_ARRAY;
    UI8_T data;

    if(index>=(sizeof(i2c_mux_addrs)/sizeof(i2c_mux_addrs[0])))
    {
        printf("%s(%d): Invalid index %lu\r\n", __FUNCTION__, __LINE__, index);
        return I2C_ERROR;
    }

    data = (UI8_T)channel_bmp;
    #if defined(ES4627MB)
    if(index == 0)/*other 6 boards, use register 0xC on CPLD via i2c to change channel*/
    {
        return (write_fn_p(SYS_HWCFG_I2C_SLAVE_CPLD, I2C_7BIT_ACCESS_MODE,
            TRUE, i2c_mux_addrs[index], 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;
    }
    else/*only bid==2(U24, ES4627MB-SFP-FLF-EC, ECS4510-28F) use PCA9548*/
    {
        return (write_fn_p(i2c_mux_addrs[index], I2C_7BIT_ACCESS_MODE,
            0, 0, 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;
    }
    #else
    return (write_fn_p(SYS_HWCFG_I2C_SLAVE_CPLD, I2C_7BIT_ACCESS_MODE,
        TRUE, i2c_mux_addrs[index], 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;
    #endif

#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__GPIO)
    I2C_GpioChannelModeInfoEntry_T
        gpio_set_channel_info_array[SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL]= SYS_HWCFG_GPIO_SET_CHANNEL_INFO_ARRAY;

    BOOL_T ret_val = TRUE;
    UI8_T channel_num = 0;
    UI8_T reg_val = 0;

#if defined(ES3510MA_FLF_38)
    static I2C_GpioChannelModeInfoEntry_T
        local_gpio_set_channel_info_array[SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL]= SYS_HWCFG_GPIO_SET_CHANNEL_INFO_ARRAY_BID_4;
    if(I2C_GetBoardId() == 4)
    {
        memcpy(gpio_set_channel_info_array, local_gpio_set_channel_info_array,
                sizeof(local_gpio_set_channel_info_array));
    }
#endif
    switch (channel_bmp)
    {
        case 0:
            /* Channel can not be closed, so direct return  */
            return I2C_SUCCESS;
        case BIT_0:
            channel_num = 1;
            break;

        case BIT_1:
            channel_num = 2;
            break;

        case BIT_2:
            channel_num = 3;
            break;

        case BIT_3:
            channel_num = 4;
            break;

        default:
            /* return error if channel number larger than 4 is set or
             * turn on multiple channels at the same time
             */
            printf("%s(%d):Invalid channel_bmp(0x%x)\r\n",
                __FUNCTION__, __LINE__, channel_bmp);
            return I2C_ERROR;
    }

    if (channel_num > SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL)
    {
        printf("%s(%d):The given channel number(%u) is larger than sys_hwcfg max channel(%u)\r\n",
            __FUNCTION__, __LINE__, channel_num, SYS_HWCFG_GPIO_CHANNEL_MODE_MAX_NBR_OF_CHANNEL);
        return I2C_ERROR;
    }

    /* Select SFP channel by config GPIO
     */
    ret_val = PHYSICAL_ADDR_ACCESS_Read(
        (UI32_T)gpio_set_channel_info_array[channel_num - 1].gpio_reg_physical_addr, 1, 1, &reg_val);

    if (FALSE == ret_val)
    {
        printf("%s(%d):PHYSICAL_ADDR_ACCESS_Read Fail\r\n", __FUNCTION__, __LINE__);
        return I2C_ERROR;
    }

    reg_val &= ~(gpio_set_channel_info_array[channel_num - 1].mask);
    reg_val |= gpio_set_channel_info_array[channel_num - 1].enable_channel_val;
    ret_val = PHYSICAL_ADDR_ACCESS_Write(
        (UI32_T)gpio_set_channel_info_array[channel_num - 1].gpio_reg_physical_addr, 1, 1, &reg_val);

    if (FALSE == ret_val)
    {
        printf("%s(%d):PHYSICAL_ADDR_ACCESS_Write Fail\r\n", __FUNCTION__, __LINE__);
        return I2C_ERROR;
    }
    else
    {
        return I2C_SUCCESS;
    }
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__HWCFG_API)
    if(TRUE == SYS_HWCFG_I2cSetMuxWithFunPtr(I2C_GetBoardId(), index, channel_bmp, write_fn_p))
        return I2C_SUCCESS;
    else
    {
        printf("%s(%d):SYS_HWCFG_I2cSetMuxWithFunPtr Fail\r\n", __FUNCTION__, __LINE__);
        return I2C_ERROR;
    }
#else
    printf("%s(%d): Unknown select channel mode\r\n", __FUNCTION__, __LINE__);
    return I2C_ERROR;
#endif
}

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
BOOL_T I2C_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
    return DEV_SWDRV_PMGR_TwsiDataRead(dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);
}

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
BOOL_T I2C_TwsiDataReadWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
    UI8_T  uart_to_i2c_i2c_bus_idx;
    BOOL_T have_uart_to_i2c, ret_val;

    have_uart_to_i2c=SYS_HWCFG_GetI2CBusIdxOfUARTToI2C(I2C_GetBoardId(), &uart_to_i2c_i2c_bus_idx);

    if(have_uart_to_i2c==TRUE && uart_to_i2c_i2c_bus_idx==i2c_bus_idx)
    {
        ret_val = UART_TO_I2C_TwsiDataRead(i2c_bus_idx, dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);
    }
    else
    {
        ret_val = DEV_SWDRV_PMGR_TwsiDataRead(dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);
    }

    return ret_val;
#else /* #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */

    if(i2c_bus_idx!=I2C_DEFAULT_BUS_IDX)
    {
        printf("%s(%d): Not support i2c transaction on i2c bus idx %u\r\n",
            __FUNCTION__, __LINE__, i2c_bus_idx);
        return FALSE;
    }

    return DEV_SWDRV_PMGR_TwsiDataRead(dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);

#endif /* #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */
}

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
BOOL_T I2C_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
    return DEV_SWDRV_PMGR_TwsiDataWrite(dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
}

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
BOOL_T I2C_TwsiDataWriteWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
    UI8_T  uart_to_i2c_i2c_bus_idx;
    BOOL_T have_uart_to_i2c, ret_val;

    have_uart_to_i2c=SYS_HWCFG_GetI2CBusIdxOfUARTToI2C(I2C_GetBoardId(), &uart_to_i2c_i2c_bus_idx);
    if(have_uart_to_i2c==TRUE && uart_to_i2c_i2c_bus_idx==i2c_bus_idx)
    {
        ret_val = UART_TO_I2C_TwsiDataWrite(i2c_bus_idx, dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
    }
    else
    {
        ret_val = DEV_SWDRV_PMGR_TwsiDataWrite(dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
    }

    return ret_val;

#else /* #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */
    if(i2c_bus_idx!=I2C_DEFAULT_BUS_IDX)
    {
        printf("%s(%d): Not support i2c transaction on i2c bus idx %u\r\n",
            __FUNCTION__, __LINE__, i2c_bus_idx);
        return FALSE;
    }
    return DEV_SWDRV_PMGR_TwsiDataWrite(dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
#endif /* #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */
}

/* LOCAL SUBPROGRAM BODIES
 */
/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_GetBoardId
 * -----------------------------------------------------------------------------
 * FUNCTION: Get board id.
 * INPUT   : None.
 * RETURN  : Board id.
 * -----------------------------------------------------------------------------
 */
static UI32_T I2C_GetBoardId(void)
{
    if (board_id==INVALID_BOARD_ID)
    {
        UC_MGR_Sys_Info_T uc_sys_info;
        if (!UC_MGR_GetSysInfo(&uc_sys_info))
        {
            printf("\r\nGet UC System Information Fail. Halt.");

            /* severe problem, while loop here
             */
            while (TRUE);
        }
        board_id=uc_sys_info.board_id;
    }
    return board_id;
}

