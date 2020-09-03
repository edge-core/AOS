/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_backdoor.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file contains the debugging information of PoE driver.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */
 
 
#ifndef POEDRV_BACKDOOR_H
#define POEDRV_BACKDOOR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 


/* FUNCTION NAME: POEDRV_BACKDOOR_Main
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
void POEDRV_BACKDOOR_Main (void);


/* FUNCTION NAME: POEDRV_BACKDOOR_DisplayRecvPacket
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
void POEDRV_BACKDOOR_DisplayPacket(BOOL_T is_transmit, UI8_T *buf);


/* FUNCTION NAME: POEDRV_BACKDOOR_IsDisplayPacketFlagOn
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
BOOL_T POEDRV_BACKDOOR_IsDisplayPacketFlagOn();


/* FUNCTION NAME: POEDRV_BACKDOOR_IsDisplayNotifyFlagOn
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
BOOL_T POEDRV_BACKDOOR_IsDisplayNotifyFlagOn();


/* FUNCTION NAME: POEDRV_BACKDOOR_IsDisplayDebugFlagOn
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
BOOL_T POEDRV_BACKDOOR_IsDisplayDebugFlagOn();


#endif /* POEDRV_BACKDOOR_H */
