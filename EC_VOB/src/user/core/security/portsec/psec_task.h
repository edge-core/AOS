/* Module Name: PSEC_TASK.H
 * Purpose: This file contains the information of port security module:
 *          1. Initialize resource.
 *          2. Create task.
 *
 * Notes:   None.
 * History:
 *    09/02/02       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2002
 *
 */


#ifndef PSEC_TASK_H
#define PSEC_TASK_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_callback_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


/* FUNCTION NAME: PSEC_TASK_InitiateProcessResources
 * PURPOSE: This function is used to initialize the port security module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the port security module.
 *
 */
BOOL_T PSEC_TASK_InitiateProcessResources(void);

/* FUNCTION NAME: PSEC_TASK_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void PSEC_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME: PSEC_TASK_Create_Tasks
 * PURPOSE: Create and start PSEC task.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T PSEC_TASK_Create_Tasks(void);



/* FUNCTION NAME: PSEC_TASK_SetTransitionMode
 * PURPOSE: set the transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the
 *             PSEC_TASK_EnterMasterMode function.
 */
void  PSEC_TASK_SetTransitionMode (void);



/* FUNCTION NAME: PSEC_TASK_EnterTransitionMode
 * PURPOSE: The function enter transition mode and free all PSEC resources
 *          and reset database to factory default.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the
 *             PSEC_TASK_EnterMasterMode function.
 */
void  PSEC_TASK_EnterTransitionMode (void);


/* FUNCTION NAME: PSEC_TASK_EnterMasterMode
 * PURPOSE: The function enter master mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
void  PSEC_TASK_EnterMasterMode(void);


/* FUNCTION NAME: PSEC_TASK_EnterSlaveMode
 * PURPOSE: The function enter slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
void PSEC_TASK_EnterSlaveMode(void);

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_TASK_IntrusionMac_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Notification intrusion mac address
 * INPUT    : src_lport, vid, ...
 * OUTPUT   : None.
 * RETURN   : TRUE -- intrusion packet, drop packet / FALSE -- not intrusion, go ahead
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_TASK_IntrusionMac_CallBack(
                UI32_T src_lport,
                UI16_T vid,
                UI8_T *src_mac,
                UI8_T *dst_mac,
                UI16_T ether_type,
                void *cookie);


/* FUNCTION NAME: PSEC_TASK_NA_IntrusionMac_CallBack
 * PURPOSE: Notification intrusion mac address
 * INPUT:   l_port, mac, reason
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   intrusion come from amtr
 */
void PSEC_TASK_NA_IntrusionMac_CallBack (UI32_T l_port, UI8_T mac[6], UI32_T reason);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_TASK_PortMove_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Notification port move
 * INPUT    : src_lport, vid, ...
 * OUTPUT   : None.
 * RETURN   : TRUE / FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_TASK_PortMove_CallBack(
                            UI32_T src_lport,
                            UI16_T vid,
                            UI8_T *mac,
                            UI32_T original_ifindex);
#endif

/* MACRO FUNCTION DECLARATIONS
 */


#endif /* PSEC_TASK_H */
