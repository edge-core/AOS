/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_board.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file includes the APIs for POEDRV.
 *
 * NOTES:
 *
 * HISTORY
 *
 * Copyright(C)    Accton Corporation, 2013
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_hwcfg.h"
#include "phyaddr_access.h"
#include "i2cdrv.h"
#include "dev_swdrv.h"
#include "dev_swdrv_pmgr.h"
#include "stktplg_board.h"
#include "stktplg_om.h"
#include "sysfun.h"
#include "uc_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define POEDRV_BOARD_GPIO_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(poedrv_board_gpio_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POEDRV_BOARD_GPIO_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(poedrv_board_gpio_sem_id)


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T poedrv_board_gpio_sem_id = 0; /* Use the Semaphore ID of poedrv.c for GPIO. */


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : POEDRV_BOARD_HardwareReset
 * PURPOSE: This function is used to issue a hardware reset to PoE controller.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   An hardware reset, bit 6 in system reset register, will be issued
 *          to PoE controller, as shown in following.
 *
 *          /reset _____          ____
 *                      |________|
 *
 *                      |<-10ms->|
 */
BOOL_T POEDRV_BOARD_HardwareReset(UI32_T board_id)
{
    UI32_T ret = 0;
    UI32_T poe_reset_addr_in = 0;
    UI32_T poe_reset_addr_out = 0;
    UI32_T poe_reset_addr_act = 0;
    UI8_T poe_reset_data = 0;
    UI8_T poe_reset_data_mask = 0;
    UI8_T data_out = 0, data_act = 0;
    BOOL_T retval = FALSE;


    if ((ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &poedrv_board_gpio_sem_id)) != SYSFUN_OK)
    {
        printf("%s(%d): SYSFUN_GetSem return != SYSFUN_OK value = %lu.\r\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    switch (board_id)
    {
        case 1:
            poe_reset_addr_in = SYS_HWCFG_GPIO_IN_LOW + 2;
            poe_reset_addr_out = SYS_HWCFG_GPIO_OUT_LOW + 2;
            poe_reset_addr_act = SYS_HWCFG_GPIO_ACT_LOW + 2;
            poe_reset_data = SYS_HWCFG_SYSTEM_RESET_POE_SOFTWARE_RESET_BID_1;
            poe_reset_data_mask = ~poe_reset_data;
            break;
        case 3:
            poe_reset_addr_in = SYS_HWCFG_GPIO_IN + 1;
            poe_reset_addr_out = SYS_HWCFG_GPIO_OUT + 1;
            poe_reset_addr_act = SYS_HWCFG_GPIO_ACT + 1;
            poe_reset_data = SYS_HWCFG_SYSTEM_RESET_POE_SOFTWARE_RESET_BID_3;
            poe_reset_data_mask = ~poe_reset_data;
            break;
        default:
            return FALSE;
    }

    POEDRV_BOARD_GPIO_ENTER_CRITICAL_SECTION();
    retval = PHYSICAL_ADDR_ACCESS_Read(poe_reset_addr_in, 1, 1, &data_out);
    data_out &= poe_reset_data_mask;
    retval = PHYSICAL_ADDR_ACCESS_Write(poe_reset_addr_out, 1, 1, &data_out);
    retval = PHYSICAL_ADDR_ACCESS_Read(poe_reset_addr_act, 1, 1, &data_act);
    data_act &= poe_reset_data_mask;
    retval = PHYSICAL_ADDR_ACCESS_Write(poe_reset_addr_act, 1, 1, &data_act);
    POEDRV_BOARD_GPIO_LEAVE_CRITICAL_SECTION();
    SYSFUN_Sleep(1); /* at least 100uS in spec. */

    POEDRV_BOARD_GPIO_ENTER_CRITICAL_SECTION();
    retval = PHYSICAL_ADDR_ACCESS_Read(poe_reset_addr_in, 1, 1, &data_out);
    data_out |= poe_reset_data;
    retval = PHYSICAL_ADDR_ACCESS_Write(poe_reset_addr_out, 1, 1, &data_out);
    retval = PHYSICAL_ADDR_ACCESS_Read(poe_reset_addr_act, 1, 1, &data_act);
    data_act &= poe_reset_data_mask;
    retval = PHYSICAL_ADDR_ACCESS_Write(poe_reset_addr_act, 1, 1, &data_act);
    POEDRV_BOARD_GPIO_LEAVE_CRITICAL_SECTION();
    SYSFUN_Sleep(100); /* a 500mS delay request in spec. */

    return TRUE;
}

/* FUNCTION NAME : POEDRV_BOARD_ReleaseSoftwareReset
 * PURPOSE: This function is used to release/hold software reset for PoE controller.
 *          PoE controller will start/stop powering connected PDs.
 * INPUT:   is_enable -- TRUE : port powering enabled
 *                       FALSE: port powering disabled
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   An software reset, by setting bit-5 in system reset register, will be issued
 *          to PoE controller, as shown in following.
 *
 *          /reset       _________
 *                 _____|
 *
 *                      |<-10ms->|
 */
BOOL_T POEDRV_BOARD_ReleaseSoftwareReset(UI32_T board_id, BOOL_T is_enable)
{
    UI32_T ret = 0;
    UI32_T poe_disable_addr_in = 0;
    UI32_T poe_disable_addr_out = 0;
    UI32_T poe_disable_addr_act = 0;
    UI8_T poe_disable_data = 0;
    UI8_T poe_disable_data_mask = 0;
    UI8_T data_out = 0, data_act = 0;
    BOOL_T retval = FALSE;


    if ((ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &poedrv_board_gpio_sem_id)) != SYSFUN_OK)
    {
        printf("%s(%d): SYSFUN_GetSem return != SYSFUN_OK value = %lu.\r\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    switch (board_id)
    {
        case 1:
            poe_disable_addr_in = SYS_HWCFG_GPIO_IN_LOW + 3;
            poe_disable_addr_out = SYS_HWCFG_GPIO_OUT_LOW + 3;
            poe_disable_addr_act = SYS_HWCFG_GPIO_ACT_LOW + 3;
            poe_disable_data = SYS_HWCFG_SYSTEM_RESET_POE_ENABLE;
            poe_disable_data_mask = ~poe_disable_data;
            break;
        case 3:
            poe_disable_addr_in = SYS_HWCFG_GPIO_IN;
            poe_disable_addr_out = SYS_HWCFG_GPIO_OUT;
            poe_disable_addr_act = SYS_HWCFG_GPIO_ACT;
            poe_disable_data = SYS_HWCFG_SYSTEM_RESET_POE_ENABLE;
            poe_disable_data_mask = ~poe_disable_data;
            break;
        default:
            return FALSE;
    }

    POEDRV_BOARD_GPIO_ENTER_CRITICAL_SECTION();
    retval = PHYSICAL_ADDR_ACCESS_Read(poe_disable_addr_in, 1, 1, &data_out);
    if (is_enable == TRUE)
        data_out |= poe_disable_data;
    else
        data_out &= poe_disable_data_mask;
    retval = PHYSICAL_ADDR_ACCESS_Write(poe_disable_addr_out, 1, 1, &data_out);
    retval = PHYSICAL_ADDR_ACCESS_Read(poe_disable_addr_act, 1, 1, &data_act);
    data_act &= poe_disable_data_mask;
    retval = PHYSICAL_ADDR_ACCESS_Write(poe_disable_addr_act, 1, 1, &data_act);
    POEDRV_BOARD_GPIO_LEAVE_CRITICAL_SECTION();
    SYSFUN_Sleep(1);

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

