/* MODULE NAME: cli_backdoor.h
 * PURPOSE:
 *	Definitions for the CLI backdoor
 *
 * NOTES:
 *	None.
 *
 * HISTORY:
 *    mm/dd/yy (A.D.)
 *    05/14/15    -- Emma, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef CLI_BACKDOOR_H
#define CLI_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_cpnt.h"

#if (SYS_CPNT_CLI == TRUE)
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: CLI_BACKDOOR_Main
 * PURPOSE : The main routine of the backdoor
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void CLI_BACKDOOR_Main(void);
#endif  /* #if (SYS_CPNT_CLI == TRUE) */

#endif   /* End of CLI_BACKDOOR_H */
