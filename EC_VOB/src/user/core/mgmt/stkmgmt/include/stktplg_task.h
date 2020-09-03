/* Module Name: STKTPLG_TASK.H
 * Purpose: 
 *        The topology object manager provides the following APIs for other functionalities
 *        to access total number of units in the stack and ID, type, existence,
 *        serial number, hardware version and software version of specified unit.
 *
 * Notes: 
 *    The operations done by STKTPLG_TASK will be handled by STKTPLG_PROC mgr
 *    thread on linux platform.
 *
 * History:                                                               
 *       Date          -- Modifier,  Reason
 *    10/04/2002       -- David Lin, Create
 *    07/31/2007       -- Charlie Chen, Port to linux platform
 *
 * Copyright(C)      Accton Corporation, 2002 - 2007
 */

#ifndef 	STKTPLG_TASK_H
#define 	STKTPLG_TASK_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_type.h"
#include "isc.h"

/* NAME CONSTANT DECLARATIONS
 */
#define STKTPLG_INVALID_SRC_UNIT            0xFF

#define STKTPLG_TASK_EVENT_PERIODIC_TIMER   BIT_0

/*
 * Define topology event and event message
 */
#define STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE   0x01
#define STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE       0x02
#define STKTPLG_MASTER_LOSE_MSG                        0x03
#define STKTPLG_DOWNLOAD_REQUEST_MSG                   0x04
#define STKTPLG_MASTER_REMOVE_MSG                      0x05

/* periodic timer interval 
 */
#define STKTPLG_TASK_TIMER_EVENT_INTERVAL   50  

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 


/* FUNCTION NAME : STKTPLG_TASK_InitiateProcessResources
 * PURPOSE: This function initializes all releated variables and restarts
 *          the state machine.
 * INPUT:   None.
 * OUTPUT:  num_of_unit  -- the number of units existing in the stack.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_TASK_InitiateProcessResources(void);

/* FUNCTION NAME : STKTPLG_TASK_ReceivePackets
 * PURPOSE: 
 * INPUT:   key           -- isc key passed from ISC
 *          mref_handle_p -- pointer to mref handle that cotains the packet coming for stack management
 *          rx_port       -- from which port the packet is received(UP or DOWN)
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 */
BOOL_T STKTPLG_TASK_ReceivePackets(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T rx_port);

/* FUNCTION NAME : STKTPLG_TASK_HandleEvents
 * PURPOSE: This procedure will handle the events belong to STKTPLG_TASK
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKTPLG_TASK_HandleEvents(UI32_T events);

#endif   /* STKTPLG_TASK_H */
