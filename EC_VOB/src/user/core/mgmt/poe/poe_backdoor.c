/*-----------------------------------------------------------------------------
 * FILE NAME: poe_backdoor.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    Implementations for the POE backdoor
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    1/2/2008 - Daniel Chen, Created
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "poe_om.h"
#include "poe_mgr.h"
#include "poe_engine.h"
#include "poe_task.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "dev_swdrv_pmgr.h"
#include "i2cdrv.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define MAXLINE                                            255

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE) && (SYS_CPNT_POE_MCU == SYS_CPNT_POE_ASIC_NONE)
#define SUPPORT_POWERDSINE_POE_RW_FUNCTION                 TRUE
#else
#define SUPPORT_POWERDSINE_POE_RW_FUNCTION                 FALSE
#endif

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void POE_BACKDOOR_ShowMenu();
static void POE_BACKDOOR_ShowDot3atVariable();
static void POE_BACKDOOR_ResetPort();

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
static void POE_BACKDOOR_SetManualForceHighPower();
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
static void POE_BACKDOOR_SendAnLldpFrame();
#endif

#if (SUPPORT_POWERDSINE_POE_RW_FUNCTION == TRUE)
static BOOL_T POE_BACKDOOR_GetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T data_len, UI8_T *data);
static BOOL_T POE_BACKDOOR_SetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T *data, UI8_T data_len);
#endif


/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T poe_backdoor_debug_flag = FALSE;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: POE_BACKDOOR_IsDebugMsgOn
 * PURPOSE: Get the status of debug flag
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
BOOL_T POE_BACKDOOR_IsDebugMsgOn()
{
    return poe_backdoor_debug_flag;
}


/* FUNCTION NAME: POE_BACKDOOR_Main
 * PURPOSE: Main function of POE backdoor
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
void POE_BACKDOOR_Main (void)
{
    UI8_T ch;
    char cmd_buf[MAXLINE + 1] = {0};

#if (SUPPORT_POWERDSINE_POE_RW_FUNCTION == TRUE)
    UI32_T i2c_bus_index = 0, i2c_mux_channel = 0;
    UI32_T i2c_addr = 0, reg = 0, reg_data = 0;
    UI8_T data[2] = {0}, data_len = 0;
    UI16_T *reg_data_16_ptr = (UI16_T *) data;
    BOOL_T ret = FALSE;
#endif


    POE_BACKDOOR_ShowMenu();

    while (1)
    {
        ch = 0;
		
        BACKDOOR_MGR_Printf("\r\n\r\nEnter Selection (0 for menu page and 99 to exit poe page): "); /* pgr0695 */
        BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
        ch = atoi(cmd_buf);
		
        switch (ch)
        {
            case 0:
            {
                POE_BACKDOOR_ShowMenu();
                break;
            }
            case 1:
			{
                POE_BACKDOOR_ResetPort();
                break;
            }
            case 2:
            {
                POE_BACKDOOR_ShowDot3atVariable();
                break;
            }
            case 3:
            {
                poe_backdoor_debug_flag = !poe_backdoor_debug_flag;
                break;
            }
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
            case 4:
            {
                POE_BACKDOOR_SetManualForceHighPower();
                break;
            }
#endif
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
            case 5:
            {
                POE_BACKDOOR_SendAnLldpFrame();
                break;
            }
#endif

#if (SUPPORT_POWERDSINE_POE_RW_FUNCTION == TRUE)
            case 97:
                memset(&data, 0, sizeof(data));
                BACKDOOR_MGR_Printf("\r\nEnter I2C BUS Index (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                i2c_bus_index = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter I2C MUX Channel (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                i2c_mux_channel = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter PoE Device I2C Address (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                i2c_addr = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter PoE Device Register Offset (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                reg = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter PoE Device Register Data Length (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                data_len = strtoul(cmd_buf, NULL, 16);
                if (reg > 0xFFFF || data_len > 2)
                    break;
                ret = POE_BACKDOOR_GetPoERegister(i2c_bus_index, i2c_mux_channel, (UI8_T) i2c_addr, reg, data_len, data);
                BACKDOOR_MGR_Printf("\r\nREG_DATA = 0x%02X%02X, the result is %sED!\r\n", data[0], data[1], (ret == TRUE) ? "PASS" : "FAIL"); /* pgr0695 */
                break;
            case 98:
                memset(&data, 0, sizeof(data));
                BACKDOOR_MGR_Printf("\r\nEnter I2C BUS Index (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                i2c_bus_index = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter I2C MUX Channel (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                i2c_mux_channel = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter PoE Device I2C Address (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                i2c_addr = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter PoE Device Register Offset (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                reg = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter new Register Data (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                reg_data = strtoul(cmd_buf, NULL, 16);
                BACKDOOR_MGR_Printf("\r\nEnter PoE Device Register Data Length (HEX): "); /* pgr0695 */
                memset(&cmd_buf, 0, sizeof(cmd_buf));
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE); /* pgr0695 */
                data_len = strtoul(cmd_buf, NULL, 16);
                if (reg > 0xFFFF || reg_data > 0xFFFF || data_len > 2)
                    break;
                *reg_data_16_ptr = (UI16_T) reg_data;
                ret = POE_BACKDOOR_SetPoERegister(i2c_bus_index, i2c_mux_channel, (UI8_T) i2c_addr, reg, data, data_len);
                BACKDOOR_MGR_Printf("\r\nWrite DATA (0x%02X%02X) into REG (0x%lX), the result is %sED!\r\n", data[0], data[1], reg, (ret == TRUE) ? "PASS" : "FAIL"); /* pgr0695 */
                break;
#endif

            case 99:
                return;
            default:
                break;
        }
    }
}


static void POE_BACKDOOR_ShowMenu()
{
    BACKDOOR_MGR_Printf("\n===========POE Backdoor=============\n"); /* pgr0695 */
    BACKDOOR_MGR_Printf(" 0: show menu\n"); /* pgr0695 */
    BACKDOOR_MGR_Printf(" 1: Reset port.\n"); /* pgr0695 */
    BACKDOOR_MGR_Printf(" 2: show dot3at port state and related variable\n"); /* pgr0695 */
    BACKDOOR_MGR_Printf(" 3: change debug msg state(current: %s)\n", poe_backdoor_debug_flag?"Enable":"Disable"); /* pgr0695 */
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
    BACKDOOR_MGR_Printf(" 4: set force high power\n"); /* pgr0695 */
#endif
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    BACKDOOR_MGR_Printf(" 5: send an lldp frame to the specified port\n"); /* pgr0695 */
#endif

#if (SUPPORT_POWERDSINE_POE_RW_FUNCTION == TRUE)
    BACKDOOR_MGR_Printf("97: Read data form PoE device register.\r\n"); /* pgr0695 */
    BACKDOOR_MGR_Printf("98: Write data into PoE device register.\r\n"); /* pgr0695 */
#endif

    BACKDOOR_MGR_Printf("99: quit\n"); /* pgr0695 */
}


static void POE_BACKDOOR_ResetPort()
{
    char      line_buffer[MAXLINE];
    UI32_T    port;

    BACKDOOR_MGR_Printf("\n\r\n\r--------------- Reset Port ------------------"); /* pgr0695 */
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", 0, SYS_ADPT_POE_PSE_MAX_PORT_NUMBER); /* pgr0695 */


    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE)) /* pgr0695 */
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n\r"); /* pgr0695 */
        return;
    }

    if (port>SYS_ADPT_POE_PSE_MAX_PORT_NUMBER)
    {
        BACKDOOR_MGR_Printf("set port id error\n"); /* pgr0695 */
        return;
    }

    if (port==0)
    {
        for (port=1;port<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;port++)
        {
            POE_MGR_ResetPort(1,port);
        }
    }
    else
    {
        POE_MGR_ResetPort(1,port);
    }
    BACKDOOR_MGR_Printf("done.\n"); /* pgr0695 */
}

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
static void POE_BACKDOOR_SetManualForceHighPower()
{
    char      line_buffer[MAXLINE];
    UI32_T    port, mode;

    BACKDOOR_MGR_Printf("\n\r\n\r--------------- POE_BACKDOOR_SetManualForceHighPower ------------------"); /* pgr0695 */
    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", 0, 24); /* pgr0695 */


    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE)) /* pgr0695 */
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n\r"); /* pgr0695 */
        return;
    }
    if (port<0 || port>24)
    {
        BACKDOOR_MGR_Printf("set port id error\n"); /* pgr0695 */
        return;
    }

    BACKDOOR_MGR_Printf("\n\r Set mode (0-normal, 1-force high):"); /* pgr0695 */
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE)) /* pgr0695 */
    {
        mode = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set error !!"); /* pgr0695 */
        return;
    }

    if (mode!=1 && mode != 0)
    {
        BACKDOOR_MGR_Printf("\n\r Failed to set port %ld , error mode : %d !!", port, mode); /* pgr0695 */
        return;
    }



    if (port==0)
    {
        for (port=1;port<=24;port++)
        {
            POE_MGR_SetPortManualHighPowerMode(1, port, mode);
        }
    }
    else
    {
        POE_MGR_SetPortManualHighPowerMode(1, port, mode);
    }
    BACKDOOR_MGR_Printf("done.\n"); /* pgr0695 */

}
#endif

static void POE_BACKDOOR_ShowDot3atVariable()
{
    char      line_buffer[MAXLINE];
    UI32_T    unit, port;

    BACKDOOR_MGR_Printf("\n\r\n\r--------------- Show Port Info ------------------"); /* pgr0695 */
    BACKDOOR_MGR_Printf("\n\r set Unit ID (%d-%d, 0 for all units) : ", 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK); /* pgr0695 */
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE)) /* pgr0695 */
    {
        unit = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Unit ID Error !!\n\r"); /* pgr0695 */
        return;
    }
    if (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        BACKDOOR_MGR_Printf("set unit id error\n"); /* pgr0695 */
        return;
    }

    BACKDOOR_MGR_Printf("\n\r set Port ID (%d-%d, 0 for all ports) : ", 0, SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT); /* pgr0695 */


    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE)) /* pgr0695 */
    {
        port = atoi(line_buffer);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n\r Set Port ID Error !!\n\r"); /* pgr0695 */
        return;
    }
    if (port > SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("set port id error\n"); /* pgr0695 */
        return;
    }

    POE_ENGINE_DumpDot3atInfo(unit, port);
}

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
static void POE_BACKDOOR_SendAnLldpFrame()
{
    POE_TYPE_Dot3atPowerInfo_T info;
    POE_OM_PsePort_T entry;
    char      line_buffer[MAXLINE + 1] = {0};
    UI32_T    unit = 0, port = 0;


    BACKDOOR_MGR_Printf("\r\n\r\n--------------- POE_BACKDOOR_SendAnLldpFrame ------------------");
    BACKDOOR_MGR_Printf("\r\n Set Unit ID (%d-%d): ", 1, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        unit = strtoul(line_buffer, NULL, 10);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n Set Unit ID Error !!\r\n");
        return;
    }
    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        BACKDOOR_MGR_Printf("\r\n Set Unit ID Error !!\r\n");
        return;
    }

    BACKDOOR_MGR_Printf("\r\n Set Port ID (%d-%d): ", 1, SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT);

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        port = strtoul(line_buffer, NULL, 10);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n Set Port ID Error !!\r\n");
        return;
    }
    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("\r\n Set Port ID Error !!\r\n");
        return;
    }

    memset(&entry, 0, sizeof(entry));
    POE_OM_GetPsePortEntry(unit, port, &entry);
    if (entry.pse_port_detection_status != VAL_pethPsePortDetectionStatus_deliveringPower && entry.pse_port_detection_status != VAL_pethPsePortDetectionStatus_test)
    {
        BACKDOOR_MGR_Printf("\r\n PD disconnected !!\r\n");
        return;
    }
    memset(&info, 0, sizeof(info));
    POE_ENGINE_GetPortDot3atPowerInfo(unit, port, &info);

    BACKDOOR_MGR_Printf("\r\n\r\n [Set frame data]");
    BACKDOOR_MGR_Printf("\r\n Power type(0-3, 0x%02X): ", info.power_type);
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        info.power_type = strtoul(line_buffer, NULL, 10);
        if (info.power_type > 3)
        {
            BACKDOOR_MGR_Printf("\r\n Invalid value : %u !!", info.power_type);
            return;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n Set error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\r\n Power source(type %1u %s, 0-3, 0x%02X): ", (info.power_type & 0x2) ? 1 : 2, (info.power_type & 0x1) ? "PD" : "PSE", info.power_source);
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        info.power_source = strtoul(line_buffer, NULL, 10);
        if (info.power_source > 3)
        {
            BACKDOOR_MGR_Printf("\r\n Invalid value : %u !!", info.power_source);
            return;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n Set error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\r\n Power priority(0-3, 0x%02X): ", info.power_priority);
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        info.power_priority = strtoul(line_buffer, NULL, 10);
        if (info.power_priority > 3)
        {
            BACKDOOR_MGR_Printf("\r\n Invalid value : %u !!", info.power_priority);
            return;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n Set error !!");
        return;
    }

    BACKDOOR_MGR_Printf("\r\n PSE allocated power(0-%u, %u x 100 mW): %u", SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION / 100, info.pse_allocated_power, info.pse_allocated_power);

    BACKDOOR_MGR_Printf("\r\n PD requested power(%u-%u, %u x 100 mW): ", SYS_HWCFG_MIN_POWER_INLINE_ALLOCATION / 100, SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION / 100, info.pd_requested_power);
    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) > 0)
    {
        info.pd_requested_power = strtoul(line_buffer, NULL, 10);
        if ((info.power_type & 0x1) == 0) /* PSE: 0 and 2, PD: 1 and 3. */
        {
            info.pd_requested_power = 0;
        }
        else if (info.pd_requested_power * 100 < SYS_HWCFG_MIN_POWER_INLINE_ALLOCATION / 100 || info.pd_requested_power * 100 > SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION)
        {
            BACKDOOR_MGR_Printf("\r\n Invalid value : %u !!", info.pd_requested_power);
            return;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n Set error !!");
        return;
    }
    BACKDOOR_MGR_Printf("\r\n");

    if (POE_MGR_NotifyLldpFameReceived_Callback(unit, port, &info) == TRUE)
        BACKDOOR_MGR_Printf(" Pass!");
    else
        BACKDOOR_MGR_Printf(" Fail!");
}
#endif

#if (SUPPORT_POWERDSINE_POE_RW_FUNCTION == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_BACKDOOR_GetPoERegister
 * -------------------------------------------------------------------------
 * FUNCTION: This function performs preprocessing to read data from register of PSE controller
 * INPUT   : i2c_bus_index: I2C bus index
 *           i2c_mux_channel: I2C mux channel
 *           dev_slv_id: Device slave address
 *           offset: The register offset of PSE controller
 *           data_len: length of data.     
 * OUTPUT  : data: The value to be readed from specified register of PSE controller
 * RETURN  : TRUE: If success
 *           FALSE: If failed
 * NOTE    : Need to refer HW design SPEC firstly!
 * -------------------------------------------------------------------------*/
static BOOL_T POE_BACKDOOR_GetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T data_len, UI8_T *data)
{
    BOOL_T ret = TRUE;


    if (data == NULL)
        return FALSE;
    memset(data, 0, data_len);

    if (I2CDRV_SetAndLockMux(i2c_bus_index, i2c_mux_channel) == FALSE)
    {
        printf("%s(%d): Failed to set and lock mux index %lu, channel_bmp = 0x%02lX.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index, i2c_mux_channel);
        return FALSE;
    }
    if (I2CDRV_TwsiDataRead(dev_slv_id, 0, 1, offset, (offset < 256) ? 0 : 1, data_len, data) == FALSE)
    {
        printf("%s(%d): Failed to read data from register 0x%04lX.\r\n",  __FUNCTION__, __LINE__, offset);
        ret = FALSE;
    }
    if (I2CDRV_UnLockMux(i2c_bus_index) == FALSE)
    {
        printf("%s(%d): Failed to unlock mux index %lu.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index);
        return FALSE;
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_BACKDOOR_SetPoERegister
 * -------------------------------------------------------------------------
 * FUNCTION: This function performs preprocessing to write data into register of PSE controller
 * INPUT   : i2c_bus_index: I2C bus index
 *           i2c_mux_channel: I2C mux channel
 *           dev_slv_id: Device slave address
 *           offset: The register offset of PSE controller
 *           data: The value to be written to specified register of PSE controller
 *           data_len: length of data.
 * OUTPUT  : None
 * RETURN  : TRUE: If success
 *           FALSE: If failed
 * NOTE    : Need to refer HW design SPEC firstly!
 * -------------------------------------------------------------------------*/
static BOOL_T POE_BACKDOOR_SetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T *data, UI8_T data_len)
{
    BOOL_T ret = TRUE;


    if (data == NULL)
        return FALSE;

    if (I2CDRV_SetAndLockMux(i2c_bus_index, i2c_mux_channel) == FALSE)
    {
        printf("%s(%d): Failed to set and lock mux index %lu, channel_bmp = 0x%02lX.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index, i2c_mux_channel);
        return FALSE;
    }
    if (I2CDRV_TwsiDataWrite(dev_slv_id, 0, 1, offset, (offset < 256) ? 0 : 1, data, data_len) == FALSE)
    {
        printf("%s(%d): Failed to write data into register 0x%04lX.\r\n",  __FUNCTION__, __LINE__, offset);
        ret = FALSE;
    }
    if (I2CDRV_UnLockMux(i2c_bus_index) == FALSE)
    {
        printf("%s(%d): Failed to unlock mux index %lu.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index);
        return FALSE;
    }

    return ret;
}
#endif

