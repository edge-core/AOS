/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_conntrol.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    PoE mirop/driver Communication interface
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    ??/??/???? - ???, Created
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
 

/* INCLUDE FILE DECLARATIONS
 */
/* Std lib */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* System default */
#include "sys_time.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"

/* Mib */
//#include "leaf_es3626a.h"
#include "leaf_3621.h"


/* System */
#include "sysfun.h"
#include "stktplg_pom.h"
#include "backdoor_mgr.h"
#include "fs.h"
#include "fs_type.h"
//#include "uartdrv.h"

/* PoE */
#include "poedrv.h"
#include "poedrv_backdoor.h"
#include "poedrv_control.h"

extern POEDRV_CONTROL_T poedrv_59101;

/* -------------------------------------------------------------------------
 * Function : POEDRV_Control_Hook
 * -------------------------------------------------------------------------
 * Purpose  :
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTE     :
 * -------------------------------------------------------------------------
 */
BOOL_T POEDRV_Control_Hook(POEDRV_CONTROL_T **poedrv_pointer)
{
    *poedrv_pointer = &poedrv_59101;
    return TRUE;
}
