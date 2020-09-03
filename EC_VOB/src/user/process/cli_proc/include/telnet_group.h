/* MODULE NAME:  telnet_group.h
 * PURPOSE:
 *     For TELENT group 
 *
 * NOTES:
 *
 * HISTORY
 *    7/07/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef TELNET__GROUP_H
#define TELNET__GROUP_H

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
 * ROUTINE NAME : TELNET_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in TELNET GROUP.
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
 *    
 *------------------------------------------------------------------------------
 */
void TELNET_GROUP_Create_All_Threads(void);


void  TELENT_GROUP_ProvisionComplete(void);

#endif    /* End of TELNET_GROUP_H */

