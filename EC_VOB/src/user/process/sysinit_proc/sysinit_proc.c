/* MODULE NAME:  sysinit_proc.c
 * PURPOSE:
 *    This file defines the main() routine for the executable binary file "sysint".
 *    sysinit is used to do all system-wised initialization before any CSC is initialized.
 *    sysinit shall be called in the boot-up script which is specified in "/etc/inittab"
 *    and executed by linux INIT process.
 *    Note that processes that contain CSCs can only be spawned after the execution
 *    of sysinit is done.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/28/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "sys_type.h"
#include "sysrsc_mgr.h"

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
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for sysint
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    0 -- Success
 *   <0 -- Error
 * NOTES:
 *    This function is the entry point for sysinit.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(FALSE==SYSRSC_MGR_CreateAndInitiateSystemResources())
    {
        printf("SYSRSC_MGR_CreateAndInitiateSystemResources fail.\r\n");
        /* fail to create and init system resouce
         * use a forever loop to hang here
         */
        while(1){}
     }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

