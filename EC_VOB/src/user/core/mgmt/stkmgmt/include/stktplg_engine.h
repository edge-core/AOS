/* Module Name: STKTPLG_ENGINE.H
 * Purpose:
 *
 * Notes:   
 *
 * History:                                                               
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005 				
 *
 */

#ifndef  STKTPLG_ENGINE_H
#define  STKTPLG_ENGINE_H

/* INCLUDE FILE DECLARATIONS
 */
 
#include "sys_type.h" 
#include "l_mm.h" 

/* EXPORTED SUBPROGRAM DECLARATIONS
 */

/* FUNCTION NAME : STKTPLG_ENGINE_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG ENGINE which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_ENGINE_InitiateProcessResources(void);

/* FUNCTION NAME : STKTPLG_ENGINE_StackManagement
 * PURPOSE: This function is to deal tiemr and packet_received event
 * INPUT:
 *          is_timer_event -- TRUE if the function call is triggered by a timer event
 *                            FALSE if the function call is triggered by a incoming packet
 *
 *          rx_port        -- from which port that we receive packet
 *                            Valid values are UP_PORT_RX or DOWN_PORT_RX
 *                            Undefined value when is_timer_event==TRUE
 *
 *          mref_handle_p -- packet that we receive from IUC.
 *                     if NULL, means this function is called by periodical time event.
 * OUTPUT:  notify_msg -- do we need to inform stack control to change system state.
 * RETUEN:  None          
 * NOTES:
 *          
 */
void STKTPLG_ENGINE_StackManagement(BOOL_T is_timer_event, UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);

/* FUNCTION NAME: STKTPLG_ENGINE_TCN
 * PURPOSE: This function is to result STKTPLG TCN and reassigned unit ID to all units
 *          based on the stacking spec if renumber==TRUE
 * INPUT:   renumber -- Renumbering 
 * OUTPUT:  None
 * RETUEN:  TRUE   -- Renumbering work (not standalone or unit ID not equal to 1)
 *          FALSE  -- otherwise
 * NOTES:   
 *
 */
BOOL_T STKTPLG_ENGINE_TCN(BOOL_T renumber);


/* FUNCTION NAME: STKTPLG_ENGINE_GetDebugMode
 * PURPOSE: This function can get the debug mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_GetDebugMode(BOOL_T *debug_mode);


/* FUNCTION NAME: STKTPLG_ENGINE_ToggleDebugMode
 * PURPOSE: This function can toggle the debug mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_ToggleDebugMode(void);


/* FUNCTION NAME: STKTPLG_ENGINE_GetTCNMode
 * PURPOSE: This function can get the debug mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_GetTCNMode(BOOL_T *TCN_mode);


/* FUNCTION NAME: STKTPLG_ENGINE_ToggleTCNMode
 * PURPOSE: This function can force the stack not to topology change.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void STKTPLG_ENGINE_ToggleTCNMode(void);

void STKTPLG_ENGINE_ShowStktplgInfo(void);

void STKTPLG_ENGINE_BD_SetTick(UI8_T type1,UI8_T type2,UI32_T time);
void STKTPLG_ENGINE_BD_ClearTick();

void STKTPLG_ENGINE_BD_GetTick();
#endif   /* STKTPLG_ENGINE_H */
