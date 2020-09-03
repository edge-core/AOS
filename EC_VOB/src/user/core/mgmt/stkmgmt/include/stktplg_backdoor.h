/* Module Name: STKTPLG_BACKDOOR.H
 * Purpose: This file contains the information of stack topology debugging:
 *
 * Notes:   None.
 * History:
 *    04/11/02       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */
 
 
#ifndef STKTPLG_BACKDOOR_H
#define STKTPLG_BACKDOOR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "stktplg_om.h"

/* NAME CONSTANT DECLARATIONS
 */
#define STKTPLG_BACKDOOR_RXHBT   1
#define STKTPLG_BACKDOOR_TXHBT   2



/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME : STKTPLG_BACKDOOR_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_BACKDOOR_Create_InterCSC_Relation(void);

/* FUNCTION NAME : STKTPLG_BACKDOOR_ShowHbtPacket
 * PURPOSE: Show HBT packet
 * INPUT:   rx_tx         -- 
 *          current_state -- 
 *          pHbtPacket    --
 * OUTPUT:  None
 * RETUEN:  None          
 * NOTES:
 *          
 */
void STKTPLG_BACKDOOR_ShowHbtPacket(UI32_T rx_tx, STKTPLG_OM_State_T current_state, UI8_T* pHbtPacket);

/* MACRO FUNCTION DECLARATIONS
 */
BOOL_T STKTPLG_BACKDOOR_IncTxCounter(UI8_T type);

BOOL_T STKTPLG_BACKDOOR_IncRxCounter(UI8_T type);

void STKTPLG_BACKDOOR_ShowTxCounterType(UI8_T type);

BOOL_T STKTPLG_BACKDOOR_IncSendCounter(UI8_T type,UI8_T place);

void STKTPLG_BACKDOOR_SaveTopoState(UI8_T state);

BOOL_T STKTPLG_BACKDOOR_GetButtonState();

void STKTPLG_BACKDOOR_SetMaxProcessTime(UI32_T time,UI8_T state,BOOL_T event,UI8_T type);

BOOL_T STKTPLG_BACKDOOR_IncTimeOutCounter(UI8_T type);

#endif /* STKTPLG_BACKDOOR_H */
