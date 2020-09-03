/* MODULE NAME:  stkctrl_group.h
 * PURPOSE:
 *     Implementations of stkctrl group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/11/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKCTRL_GROUP_H
#define STKCTRL_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKCTRL_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in STKCTRL GROUP.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    All threads in the same CSC group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void STKCTRL_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKCTRL_GROUP_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be the function entry of main thread of the calling
 *    process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    1.All threads in the same CSC group will join the same thread group.
 *    2.Because the process STKCTRL_PROC conains no OM, STKCTRL_GROUP MGR thread
 *      will become main thread.
 *------------------------------------------------------------------------------
 */
void STKCTRL_GROUP_Main_Thread_Function_Entry(void);

#endif    /* End of STKCTRL_GROUP_H */

