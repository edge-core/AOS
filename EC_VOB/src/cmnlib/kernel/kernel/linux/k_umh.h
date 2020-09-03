/* Module Name: k_umh.c
 * Purpose:
 *      This module is for user mode helper.
 *
 * Notes:
 *      None.
 *
 * HISTORY
 *       Date       --  Modifier    Reason
 *       2014.11.26 --  Squid       Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 */

#ifndef _UMH_H
#define _UMH_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* PURPOSE: Clean up procedure for user mode helper.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None.
 */
void K_UMH_Cleanup( void );

/* PURPOSE: Init procedure for user mode helper.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None.
 */
void K_UMH_Init( void );

#endif /* #ifndef _UMH_H */

