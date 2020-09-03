/* MODULE NAME:  snmpgroup.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of SNMP group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef SNMP_GROUP_H
#define SNMP_GROUP_H

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
 * ROUTINE NAME : SNMP_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in SNMPGroup.
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
 *    All threads in the same SNMP group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void SNMP_GROUP_Create_All_Threads(void);

#endif    /* End of SNMP_GROUP_H */
