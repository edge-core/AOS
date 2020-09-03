/* MODULE NAME:  sys_hwcfg.c
 * PURPOSE:
 *     This module provides functions for getting hardware related information such
 *     as flash partition information.
 *
 * NOTES:
 *
 * HISTORY
 *    11/5/2007 - kh_shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sys_adpt.h"
#include "uc_mgr.h"
#include "i2c_export.h"
#include "stktplg_board.h"

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_HWCFG_SwitchChip_MiscInit
 *---------------------------------------------------------------------------------
 * PURPOSE: config EPLD important bit for module
 * INPUT:
 * OUTPUT:
 * RETUEN:  TRUE/FALSE
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T SYS_HWCFG_SwitchChip_MiscInit()
{
    return TRUE;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_HWCFG_GetI2CPeripheralInitOpers
 *---------------------------------------------------------------------------------
 * PURPOSE: Output I2C peripheral initialization operations.
 * INPUT:   board_id        --   board id.
 * OUTPUT:  i2c_trans_lst_pp--   i2c peripheral initialization transactions list
 *          lst_size_p      --   list size of i2c_trans_lst_p
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_HWCFG_I2C_INIT_PHERIPHERAL is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T SYS_HWCFG_GetI2CPeripheralInitOpers(UI32_T board_id, const SYS_HWCFG_I2CTransactionEntry_T **i2c_trans_lst_pp, UI32_T* lst_size_p)
{
    /* validate input args
     */
    if (board_id>2)
    {
        printf("%s(%d): Invalid board_id=%lu\r\n", __FUNCTION__, __LINE__, board_id);
        return FALSE;
    }

    if (i2c_trans_lst_pp==NULL)
    {
        printf("%s(%d): i2c_trans_lst_pp is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (lst_size_p==NULL)
    {
        printf("%s(%d): lst_size_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    *lst_size_p=0;

    return TRUE;
}


