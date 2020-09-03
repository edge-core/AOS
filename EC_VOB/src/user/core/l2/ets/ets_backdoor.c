/* MODULE NAME: ets_backdoor.c
 * PURPOSE:
 *   Definitions of backdoor APIs for ETS
 *   (IEEE Std 802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ETS == TRUE)

#include "ets_backdoor.h"
#include "ets_type.h"
#include "ets_om.h"
#include "ets_mgr.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"


/* NAMING CONSTANT DECLARATIONS
 */
#if (ETS_TYPE_BUILD_LINUX == FALSE)
    #define BACKDOOR_MGR_Printf            printf
    #define BACKDOOR_MGR_RequestKeyIn      BACKDOOR_MGR_RequestKeyIn
#endif


/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void   ETS_BACKDOOR_Engine(void);

/* STATIC VARIABLE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE: Main routine for ETS backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void ETS_BACKDOOR_Main(void)
{
    ETS_BACKDOOR_Engine();
}


/* LOCAL SUBPROGRAM BODIES
 */

static void ETS_BACKDOOR_Engine(void)
{
    return;
}
#endif /* #if (SYS_CPNT_ETS == TRUE) */


