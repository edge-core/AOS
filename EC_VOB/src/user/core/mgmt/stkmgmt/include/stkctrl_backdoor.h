/* MODULE NAME:  stkctrl_backdoor.h
 * PURPOSE:
 *    stack control backdoor
 *
 * NOTES:
 *
 * HISTORY
 *    8/10/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKCTRL_BACKDOOR_H
#define STKCTRL_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define STKCTRL_DEBUG_PERFORMACE TRUE

/* DATA TYPE DECLARATIONS
 */
typedef struct STKCTRL_BACKDOOR_CPNT_TIMER_S
{
#define MAX_CPNT_NBR	50
#define CPNT_NAME_LEN	20
    /* current system state
     */
    BOOL_T is_master;
    
    /* final state notified from stack topology
     */
    UI32_T cpnt_time[MAX_CPNT_NBR];

    /* name of cpnt
     */
    UI8_T cpnt_name[MAX_CPNT_NBR][CPNT_NAME_LEN];

    /* number of cpnt
     */
    UI8_T cpnt_nbr;
	
} STKCTRL_BACKDOOR_CPNT_TIMER_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : STKCTRL_BACKDOOR_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKCTRL_BACKDOOR_Create_InterCSC_Relation(void);

BOOL_T STKCTRL_BACKDOOR_GetTimeInfo(STKCTRL_BACKDOOR_CPNT_TIMER_T **pCpnt_ready_time);

/* FUNCTION NAME : STKCTRL_BACKDOOR_IsStkCtrlDbgMsgOn
 * PURPOSE: This function is used to know the if the debug message of STKCTRL should be 
 *          shown pr not.
 * INPUT:   None.
 * OUTPUT:  none.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKCTRL_BACKDOOR_IsStkCtrlDbgMsgOn(void);

/* FUNCTION NAME : STKCTRL_BACKDOOR_SetStkCtrlDbgMsgOn
 * PURPOSE: This function set the flag of stkctrl_dbgmsg_shown_flag
 * INPUT:   value -- TRUE/FALSE flag
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKCTRL_BACKDOOR_SetStkCtrlDbgMsgOn();

#endif    /* End of STKCTRL_BACKDOOR_H */

