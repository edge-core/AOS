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

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

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
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

