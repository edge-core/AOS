/* FUNCTION NAME: add_utest.h
 * PURPOSE:
 *	1. ADD unit test
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#ifndef ADD_UTEST_H
#define ADD_UTEST_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ADD_DO_UNIT_TEST          FALSE

#if (ADD_DO_UNIT_TEST == TRUE)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------
 * Routine Name : ADD_UTEST_Main
 *---------------------------------------------------------------------------
 * Function : excute unit test main function
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void ADD_UTEST_Main();

/*---------------------------------------------------------------------------
 * Routine Name : ADD_UTEST_RunVoiceVlanUTest
 *---------------------------------------------------------------------------
 * Function : excute unit test main function
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
BOOL_T ADD_UTEST_RunVoiceVlanUTest();

#endif /* #if (ADD_DO_UNIT_TEST == TRUE) */
#endif /* #ifndef ADD_UTEST_H */

