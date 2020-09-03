/* MODULE NAME:  get_accu_sys_up_time.c
 * PURPOSE:
 *   This module is a utility program to show the accumulated system up time
 *   in console. This utility is for debug purpose only.
 *
 * NOTES:
 *   None.
 *
 * HISTORY
 *    9/12/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>

#include "sys_type.h"
#include "sysrsc_mgr.h"
#include "sys_time.h"

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

/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T init_system(void)
{
    BOOL_T rc;

    rc=SYSRSC_MGR_AttachSystemResourcesWithoutCscRsc();
    if (rc==FALSE)
    {
        printf("SYSRSC_MGR_AttachSystemResourcesWithoutCscRsc failed.\r\n");
        return FALSE;
    }

    SYS_TIME_AttachSystemResources();

    return TRUE;
}

int main(int argc, char* argv[])
{
    SYS_TIME_AccumulatedSysUpTimeInfoBlockHeader_T *header_p=0;
    UI64_T accumulated_sys_up_time_in_minute;
    BOOL_T rc;

    if ((UI32_T)(&(header_p->checksum)) != (0 + sizeof(*header_p) - sizeof(header_p->checksum)))
    {
        printf("Warning. Padding bytes exist in the header struct.\r\n");
    }

    if (init_system()==FALSE)
    {
        printf("init_system() error.\r\n");
        return -1;
    }

    rc=SYS_TIME_AccumulatedUpTime_Get(&accumulated_sys_up_time_in_minute);
    if (rc==TRUE)
    {
        printf("Accumulated system up time is %llu minutes.\r\n", accumulated_sys_up_time_in_minute);
        return 0;
    }
    printf("Error.\r\n");
    return -1;
}

