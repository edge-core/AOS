/* ------------------------------------------------------------------------
 *  FILE NAME  -  SNMPVARS.C
 * ------------------------------------------------------------------------
 * ABSTRACT : Define the various SNMP management variables used in this 
 *            system. This file should closely match snmpvars.h
 * Note: None
 * ------------------------------------------------------------------------
 *  Copyright(C) Accton Technology Corporation, 2000
 * ------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
#include <envoy/h/snmpstat.h>   /* define snmp_stats global variable */

/* GLOBAL VARIABLES DECLARATION
 */
SNMP_STATS_T snmp_stats;    /* defined in snmpstat.h */

/* EXPORTED SUBPROGRAM SPECIFICATION 
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SNMPVARS_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will reset global struct snmp_stats
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : snmp_stats is a global structure defined in <envoy/h/snmpstat.h>
 *------------------------------------------------------------------------*/
void SNMPVARS_Init(void)
{
    memset((char *)&snmp_stats, 0, sizeof(snmp_stats));
}
