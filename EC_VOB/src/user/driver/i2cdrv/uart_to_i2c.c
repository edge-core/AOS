/* MODULE NAME:  uart_to_i2c.c
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

/* for debug only
#define UART_TO_I2C_DEBUG 1
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#include "uc_mgr.h"
#include "i2c_export.h"
#include "uart_to_i2c.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define INVALID_UART_HANDLE 0xFFFFFFFFUL

#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE == SYS_HWCFG_UART_TO_I2C_CHIP_SC18IM700)
#define UART_TO_I2C_DEFAULT_UART_BAUDRATE 9600
#define UART_TO_I2C_MAX_UART_BAUDRATE     460800
#define UART_TO_I2C_MIN_UART_BAUDRATE     113
#define UART_TO_I2C_DEFAULT_I2C_BUS_CLOCK_FREQ 194021
#define UART_TO_I2C_MAX_I2C_BUS_CLOCK_FREQ     368640
#define UART_TO_I2C_MIN_I2C_BUS_CLOCK_FREQ     7230
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#if defined(UART_TO_I2C_DEBUG)
#define DBG(fmtstr) printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__)
#define DBGARG(fmtstr, arg...) printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##arg)
#else
#define DBG(...)
#define DBGARG(...)
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T UART_TO_I2C_InitUARTHandle(void);
static BOOL_T UART_TO_I2C_GetI2CBusClockFreq(UI32_T* i2c_bus_clk_freq_p);
static BOOL_T UART_TO_I2C_SetI2CBusClockFreq(UI32_T  i2c_bus_clk_freq);
static BOOL_T UART_TO_I2C_WriteToUART(UI8_T len, const UI8_T *data_p);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T uart_handle=INVALID_UART_HANDLE;

/* EXPORTED SUBPROGRAM BODIES
 */
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
void UART_TO_I2C_AttachSystemResources(void)
{
    if(UART_TO_I2C_InitUARTHandle()==FALSE)
        printf("%s(%d): Failed to init uart handle.\r\n", __FUNCTION__, __LINE__);

    return;
}

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
BOOL_T UART_TO_I2C_ChipInit(void)
{
    UI32_T uart_baudrate, i2c_bus_clk_freq, uart_channel;
    UC_MGR_Sys_Info_T sysinfo;

    if(UC_MGR_GetSysInfo(&sysinfo)==FALSE)
    {
        printf("%s(%d):Failed to get uc sysinfo.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(SYS_HWCFG_GetUARTChannelOfUARTToI2C(sysinfo.board_id, &uart_channel)==FALSE)
    {
        DBGARG("Board %lu does not have UART to I2C.\r\n", sysinfo.board_id);
        return TRUE;
    }

    if(UART_TO_I2C_InitUARTHandle()==FALSE)
    {
        printf("%s(%d):Failed to init uart handle.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(SYSFUN_GetUartBaudRate(uart_handle, &uart_baudrate)!=SYSFUN_OK)
    {
        printf("%s(%d):Failed to get uart baudrate.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    DBGARG("Current UART baudrate=%lu\r\n", uart_baudrate);


    if(uart_baudrate!=UART_TO_I2C_DEFAULT_UART_BAUDRATE)
    {
        /* set the uart baurade as default baudrate of chip
         */
        DBGARG("Change UART baudrate to %d\r\n", UART_TO_I2C_DEFAULT_UART_BAUDRATE);
        if(SYSFUN_SetUartBaudRate(uart_handle, UART_TO_I2C_DEFAULT_UART_BAUDRATE)!=SYSFUN_OK)
        {
            printf("%s(%d):Failed to set uart baudrate as %d\r\n", __FUNCTION__, __LINE__, UART_TO_I2C_DEFAULT_UART_BAUDRATE);
            return FALSE;
        }
    }

    if(UART_TO_I2C_GetI2CBusClockFreq(&i2c_bus_clk_freq)==FALSE)
    {
        printf("%s(%d):Failed to get i2c bus clock frequency\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    DBGARG("Original UART TO I2C I2C bus clock frequency=%lu hz\r\n", i2c_bus_clk_freq);

    if(i2c_bus_clk_freq!=SYS_HWCFG_UART_TO_I2C_BUS_CLOCK_FREQ)
    {
        DBGARG("Change UART TO I2C I2C bus clock as frequency=%d hz\r\n", SYS_HWCFG_UART_TO_I2C_BUS_CLOCK_FREQ);
        if(UART_TO_I2C_SetI2CBusClockFreq(SYS_HWCFG_UART_TO_I2C_BUS_CLOCK_FREQ)==FALSE)
        {
            printf("%s(%d):Failed to set i2c bus clock frequency as %d hz\r\n", __FUNCTION__, __LINE__, SYS_HWCFG_UART_TO_I2C_BUS_CLOCK_FREQ);
            return FALSE;
        }
    }

    if(uart_baudrate!=SYS_HWCFG_UART_TO_I2C_UART_BAUDRATE)
    {
        DBGARG("Change UART TO I2C baudrate to %d\r\n", SYS_HWCFG_UART_TO_I2C_UART_BAUDRATE);
        if(UART_TO_I2C_SetBaudrate(SYS_HWCFG_UART_TO_I2C_UART_BAUDRATE)==FALSE)
        {
            printf("%s(%d):Failed to set uart to i2c baudrate as %d\r\n", __FUNCTION__, __LINE__, SYS_HWCFG_UART_TO_I2C_UART_BAUDRATE);
            return FALSE;
        }

        /* need to delay here, no delay will lead to UART_TO_I2C
         * cannot work
         */
        SYSFUN_Sleep(50);

        if(SYSFUN_SetUartBaudRate(uart_handle, SYS_HWCFG_UART_TO_I2C_UART_BAUDRATE)!=SYSFUN_OK)
        {
            printf("%s(%d):Failed to set uart baudrate as %d\r\n", __FUNCTION__, __LINE__, SYS_HWCFG_UART_TO_I2C_UART_BAUDRATE);
            return FALSE;
        }
    }

    return TRUE;
}

#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE == SYS_HWCFG_UART_TO_I2C_CHIP_SC18IM700)
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
BOOL_T UART_TO_I2C_TwsiDataRead(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
    UI8_T *cmd_all_p;
    UI8_T cmd_all_len, read_len;
    #if defined(UART_TO_I2C_DEBUG)
    UI8_T i;
    #endif

    if(i2c_bus_idx!=SYS_HWCFG_UART_TO_I2C_I2C_BUS_INDEX)
    {
        printf("%s(%d) Invalid i2c bus idx(%hu).\r\n", __FUNCTION__, __LINE__,
            i2c_bus_idx);
        return FALSE;
    }

    if(type==I2C_10BIT_ACCESS_MODE)
    {
        printf("%s(%d) I2C 10-bit access mode not support.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(validOffset!=0)
    {
        UI8_T cmd_data1[] = {(UI8_T)'S', dev_slv_id<<1 /* write */, 1/* write len */, (UI8_T)offset};
        UI8_T cmd_data2[] = {(UI8_T)'S', dev_slv_id<<1 | 0x01 /* read */, data_len /* read len */, (UI8_T)'P'};

        cmd_all_len = sizeof(cmd_data1)+sizeof(cmd_data2);
        cmd_all_p = malloc(cmd_all_len);
        if(cmd_all_p==NULL)
        {
            printf("%s(%d)malloc %hu bytes failed.\r\n", __FUNCTION__, __LINE__, cmd_all_len);
            return FALSE;
        }
        memcpy(cmd_all_p, cmd_data1, sizeof(cmd_data1));
        memcpy(cmd_all_p + sizeof(cmd_data1), cmd_data2, sizeof(cmd_data2));
        #if defined(UART_TO_I2C_DEBUG)
        DBG("Dump cmd_all_p:\r\n");
        for(i=0; i<cmd_all_len; i++)
        {
            printf("[0x%2x]", cmd_all_p[i]);
        }
        printf("\r\n");
        #endif /* end of #if defined(UART_TO_I2C_DEBUG) */

        if(UART_TO_I2C_WriteToUART(cmd_all_len, cmd_all_p)==FALSE)
        {
            printf("%s(%d)Failed to write cmd_all_p to UART.\r\n", __FUNCTION__, __LINE__);
            free(cmd_all_p);
            return FALSE;
        }
        free(cmd_all_p);
        cmd_all_p=NULL;

        read_len=data_len;
        if(UART_TO_I2C_ReadFromUART(&data_len, data)==FALSE)
        {
            printf("%s(%d)Failed to read data from UART.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }

        if(read_len!=data_len)
        {
            printf("%s(%d)Expected read len is %hu, actual read len is %hu\r\n",
                __FUNCTION__, __LINE__, read_len, data_len);
        }
    }
    else /* if(validOffset!=0) */
    {
        UI8_T cmd_data[] = {'S', dev_slv_id<<1|0x01 /* read */, data_len /* read len */, 'P'};

        #if defined(UART_TO_I2C_DEBUG)
        DBG("Dump cmd_data:\r\n");
        for(i=0; i<sizeof(cmd_data); i++)
        {
            printf("[0x%2x]", cmd_data[i]);
        }
        printf("\r\n");
        #endif

        if( UART_TO_I2C_WriteToUART(sizeof(cmd_data), cmd_data)==FALSE)
        {
            printf("%s(%d)Failed to write data to UART.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }

        read_len=data_len;
        if(UART_TO_I2C_ReadFromUART(&read_len, data)==FALSE)
        {
            printf("%s(%d)Failed to read data from UART.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }

        if(read_len!=data_len)
        {
            printf("%s(%d)Expected read len is %hu, actual read len is %hu\r\n",
                __FUNCTION__, __LINE__, read_len, data_len);
        }
    } /* end of if(validOffset!=0) */

    return TRUE;
}

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
BOOL_T UART_TO_I2C_TwsiDataWrite(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
    UI8_T *cmd_all_p;
    UI8_T cmd_data[] = {(UI8_T)'S', dev_slv_id<<1 /* write */, data_len+1 /* write len */, (UI8_T)offset};
    UI8_T write_len;
    #if defined(UART_TO_I2C_DEBUG)
    UI8_T i;
    #endif

    if(i2c_bus_idx!=SYS_HWCFG_UART_TO_I2C_I2C_BUS_INDEX)
    {
        printf("%s(%d) Invalid i2c bus idx(%hu).\r\n", __FUNCTION__, __LINE__,
            i2c_bus_idx);
        return FALSE;
    }

    if(validOffset!=0)
    {
        write_len = 3            + /*'S', device slave id, write len*/
                    (data_len+1) + /* write data */
                    1;             /*'P' */
    }
    else
    {
        write_len = 3            + /*'S', device slave id, write len*/
                    data_len     + /* write data */
                    1;             /*'P' */
    }

    cmd_all_p = malloc(write_len);
    if(cmd_all_p==NULL)
    {
        printf("%s(%d)malloc %hu bytes failed\r\n", __FUNCTION__, __LINE__, write_len);
        return FALSE;
    }
    memcpy(cmd_all_p, cmd_data, sizeof(cmd_data));
    if(validOffset!=0)
    {
        memcpy(cmd_all_p + sizeof(cmd_data), data, data_len);
    }
    else
    {
        memcpy(cmd_all_p + sizeof(cmd_data) - 1, data, data_len);
    }
    cmd_all_p[write_len-1] = (UI8_T)'P';

    #if defined(UART_TO_I2C_DEBUG)
    DBGARG("Dump cmd_data (valid_offset=%hu):\r\n", validOffset);
    for(i=0; i<write_len; i++)
    {
        printf("[0x%2x]", cmd_all_p[i]);
    }
    printf("\r\n");
    #endif /* end of #if defined(UART_TO_I2C_DEBUG) */

    if(UART_TO_I2C_WriteToUART(write_len, cmd_all_p)==FALSE)
    {
        printf("%s(%d)Failed to write data to UART.\r\n", __FUNCTION__, __LINE__);
        free(cmd_all_p);
        return FALSE;
    }
    free(cmd_all_p);
    cmd_all_p=NULL;

    return TRUE;
}
   
#endif /* end of #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE == SYS_HWCFG_UART_TO_I2C_CHIP_SC18IM700) */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: UART_TO_I2C_InitUARTHandle
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will initialize the UART handle which is used by
 *          UART_TO_I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: This function call be called more than once. It will just return
 *        TRUE if the UART handle had been initialized successfully.
 */
static BOOL_T UART_TO_I2C_InitUARTHandle(void)
{
    UI32_T rc, uart_channel;
    UC_MGR_Sys_Info_T sysinfo;

    if (uart_handle != INVALID_UART_HANDLE)
        return TRUE;

    if(UC_MGR_GetSysInfo(&sysinfo)==FALSE)
    {
        printf("%s(%d):Failed to get uc sysinfo.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(SYS_HWCFG_GetUARTChannelOfUARTToI2C(sysinfo.board_id, &uart_channel)==FALSE)
    {
        DBGARG("Board %lu does not have UART to I2C.\r\n", sysinfo.board_id);
        return TRUE;
    }

    rc=SYSFUN_OpenUARTExt(uart_channel, SYSFUN_UART_MODE_UART_TO_I2C_RAW_MODE);
    if(rc==SYSFUN_RESULT_ERROR)
    {
        return FALSE;
    }

    uart_handle=rc;
    return TRUE;
}


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
void UART_TO_I2C_SetUARTBaudRate(UI32_T baudrate)
{
    if(SYSFUN_SetUartBaudRate(uart_handle, baudrate)!=SYSFUN_OK)
        printf("%s(%d):Failed to set uart baud rate as %lu\r\n", __FUNCTION__, __LINE__,
            baudrate);
}


#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE == SYS_HWCFG_UART_TO_I2C_CHIP_SC18IM700)
/* FUNCTION NAME: UART_TO_I2C_GetI2CBusClockFreq
 *-----------------------------------------------------------------------------
 * PURPOSE: This function outputs the i2c bus clock frequency in hz.
 *-----------------------------------------------------------------------------
 * INPUT   : None
 * OUTPUT  : i2c_bus_clk_freq_p  -  the i2c bus clock frequency in hz.
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: The uart baudrate must be set correctly before calling this function.
 */
static BOOL_T UART_TO_I2C_GetI2CBusClockFreq(UI32_T* i2c_bus_clk_freq_p)
{
    UI8_T i2cclkh,i2cclkl;

    /* Get register I2CClkH(addr=0x08) and I2CClkL(addr=0x07)
     */
    if(UART_TO_I2C_ReadRegister(0x08, &i2cclkh)==FALSE)
    {
        printf("%s(%d)Read register 0x08 error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(UART_TO_I2C_ReadRegister(0x07, &i2cclkl)==FALSE)
    {
        printf("%s(%d)Read register 0x09 error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }    

    /* I2C bus clock frequency = 7372800 / ( 2 * (I2CClkH + I2CClkL)
     */
    *i2c_bus_clk_freq_p = 7372800UL / ( 2* (i2cclkh+i2cclkl) );
    return TRUE;
}

/* FUNCTION NAME: UART_TO_I2C_SetI2CBusClockFreq
 *-----------------------------------------------------------------------------
 * PURPOSE: This function sets the given I2C bus clock frequency to the UART TO
 *          I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : i2c_bus_clk_freq  - the i2c bus frequency in hz
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: None.
 */
static BOOL_T UART_TO_I2C_SetI2CBusClockFreq(UI32_T i2c_bus_clk_freq)
{
    UI32_T i2cclk_total;
    UI8_T  i2cclk;

    if( (i2c_bus_clk_freq>UART_TO_I2C_MAX_I2C_BUS_CLOCK_FREQ) ||
        (i2c_bus_clk_freq<UART_TO_I2C_MIN_I2C_BUS_CLOCK_FREQ) )
    {
        printf("%s(%d)Invalid i2c bus clock frequency=%lu hz. (Max=%d, Min=%d)\r\n",
            __FUNCTION__, __LINE__, i2c_bus_clk_freq, UART_TO_I2C_MAX_I2C_BUS_CLOCK_FREQ,
            UART_TO_I2C_MIN_I2C_BUS_CLOCK_FREQ);
        return FALSE;
    }

    /* I2CClkTotal = I2CClkH + I2CClkL
     * I2CClkTotal = 3686400 / I2C bus clock frequency
     * Let I2CClkH = I2CClkL = I2CClkTotal / 2
     */
    i2cclk_total = 3686400UL / i2c_bus_clk_freq;

    if(i2cclk_total > (0xFF * 2))
    {
        printf("%s(%d)i2cclk_total overflow(value=0x%lx)\r\n", __FUNCTION__, __LINE__,
            i2cclk_total);
        return FALSE;
    }

    i2cclk = (UI8_T)(i2cclk_total / 2);

    DBGARG("Write 0x%2X to reg 0x07\r\n", i2cclk);
    if(UART_TO_I2C_WriteRegister(0x7, i2cclk)==FALSE)
    {
        printf("%s(%d)Failed to write reg 0x7\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    DBGARG("Write 0x%2X to reg 0x08\r\n", i2cclk);
    if(UART_TO_I2C_WriteRegister(0x8, i2cclk)==FALSE)
    {
        printf("%s(%d)Failed to write reg 0x8\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T UART_TO_I2C_ReadRegister(UI8_T reg_addr, UI8_T *reg_val_p)
{
    UI8_T cmd_data[3] = {'R', reg_addr, 'P'};
    UI8_T read_len = 1;

    if(UART_TO_I2C_WriteToUART(sizeof(cmd_data), cmd_data)==FALSE)
    {
        printf("%s(%d):Write to uart error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(UART_TO_I2C_ReadFromUART(&read_len, reg_val_p)==FALSE)
    {
        printf("%s(%d):Read from uart error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(read_len!=1)
    {
        printf("%s(%d):Expected read len is 1, but real read len is %hu bytes",
            __FUNCTION__, __LINE__, read_len);
    }

    return TRUE;
}

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
BOOL_T UART_TO_I2C_WriteRegister(UI8_T reg_addr, UI8_T reg_val)
{
    UI8_T cmd_data[4] = {'W', reg_addr, reg_val, 'P'};

    if(UART_TO_I2C_WriteToUART(sizeof(cmd_data), cmd_data)==FALSE)
    {
        printf("%s(%d):Write to uart error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T UART_TO_I2C_SetBaudrate(UI32_T baudrate)
{
    UI32_T brg_reg;

    if((baudrate > UART_TO_I2C_MAX_UART_BAUDRATE) ||
       (baudrate < UART_TO_I2C_MIN_UART_BAUDRATE))
    {
        printf("%s(%d)Invalid baudrate %lu. (Max=%d, Min=%d)\r\n",
            __FUNCTION__, __LINE__, baudrate, UART_TO_I2C_MAX_UART_BAUDRATE,
            UART_TO_I2C_MIN_UART_BAUDRATE);
        return FALSE;
    }

    /* Baud rate = 7372800 / (16 + (BRG1, BRG0))
     * => (BRG1, BRG0) = ( 7372800 / Baud rate ) - 16
     */
    brg_reg = ( 7372800UL / baudrate ) - 16;

    /* For the new baud rate to take effect, both BRG0 and BRG1 must be written in
     * sequence (BRG0, BRG1) with new values. The new baud rate will be in effect once
     * BRG1 is written
     */
    DBGARG("Write 0x%2X to reg 0x0\r\n", (UI8_T)(brg_reg & 0xFF));
    if(UART_TO_I2C_WriteRegister(0x0, (UI8_T)(brg_reg & 0xFF))==FALSE)
    {
        printf("%s(%d)Failed to write register 0x0\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    DBGARG("Write 0x%2X to reg 0x1\r\n", (UI8_T)((brg_reg>>8) & 0xFF));
    if(UART_TO_I2C_WriteRegister(0x1, (UI8_T)((brg_reg>>8) & 0xFF))==FALSE)
    {
        printf("%s(%d)Failed to write register 0x1\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    return TRUE;
}
#endif /* end of #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE == SYS_HWCFG_UART_TO_I2C_CHIP_SC18IM700) */

/* FUNCTION NAME: UART_TO_I2C_WriteToUART
 *-----------------------------------------------------------------------------
 * PURPOSE: Write the given data to the UART which is connected to the UART TO
 *          I2C chip.
 *-----------------------------------------------------------------------------
 * INPUT   : len     - the length of data_p in byte
 *           data_p  - the data to be written to the UART
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES: None.
 */
static BOOL_T UART_TO_I2C_WriteToUART(UI8_T len, const UI8_T *data_p)
{
    UI32_T rc;

    if(uart_handle==INVALID_UART_HANDLE)
    {
        printf("%s(%d):Error.Invalid uart handle.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc=SYSFUN_UARTPutData(uart_handle, len, (void*)data_p);

    if(rc!=SYSFUN_OK)
    {
        printf("%s(%d):Failed to write data to uart.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    return TRUE;
}

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
BOOL_T UART_TO_I2C_ReadFromUART(UI8_T* len_p, UI8_T *data_p)
{
    UI32_T rc;
    UI8_T  i;

    if(uart_handle==INVALID_UART_HANDLE)
    {
        printf("%s(%d):Error.Invalid uart handle.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    for(i=0; i<100; i++)
    {
        rc=SYSFUN_UARTPollRxBuff(uart_handle, *len_p, data_p);
        if(rc!=0)
        {
            #if defined(UART_TO_I2C_DEBUG)
            if(i>0)
                printf("%s(%d): retry count=%d\r\n", __FUNCTION__, __LINE__, i);
            #endif
            break;
        }
        SYSFUN_Sleep(1);
    }

    if(rc==0)
    {
        *len_p=0;
        return FALSE;
    }

    *len_p=rc;
    return TRUE;
}


