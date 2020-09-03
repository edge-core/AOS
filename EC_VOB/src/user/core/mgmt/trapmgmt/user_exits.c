/* ------------------------------------------------------------------------
 *  FILE NAME  -  USER_EXITS.C
 * ------------------------------------------------------------------------
 * ABSTRACT : User-configurable callout routines declared in the library,
 *            by the Epilogue document, these subprograms named user-exits
 *            routines.
 * Note     : None
 * ------------------------------------------------------------------------
 *  Copyright(C) Accton Technology Corporation, 2000
 * ------------------------------------------------------------------------*/

#include "sys_cpnt.h"

#if (SYS_CPNT_SNMP_VERSION == 3)
/* INCLUDE FILE DECLARATIONS 
 */
#include <envoy/h/snmpdefs.h>
#include "sys_type.h"

/* EXPORTED SUBPROGRAM SPECIFICATIONS 
 */
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_validate_community
 * ------------------------------------------------------------------------
 *  FUNCTION : Check an operation against the community name. Get class 
 *             operations can use either the set or get community. Set 
 *             operations may use only the set community.
 *  INPUT    : SNMP_PKT_T *rp (The received packet -- decode format) 
 *  OUTPUT   : SNMPADDR_T *pktsrc (Source of the packet)
 *             SNMPADDR_T *pktdst (Destination of the packet)
 *  OUTPUT   : None.
 *  RETURN   : 0 for ok, 1 for not.
 *  NOTE     : None
 * ------------------------------------------------------------------------*/
int SNMP_validate_community(SNMP_PKT_T * rp, SNMPADDR_T * pktsrc, SNMPADDR_T * pktdst)
{
    static UINT_16_T lcl_ident_source = 0;
   
    /*UI32_T           pktcom_length;*/

    rp->mib_view     = 0x10;
    rp->lcl_ident    = lcl_ident_source++;
    rp->user_private = (char *) 0;
    (void) memcpy((char *) &(rp->pkt_src), (char *) pktsrc, sizeof(SNMPADDR_T));
    (void) memcpy((char *) &(rp->pkt_dst), (char *) pktdst, sizeof(SNMPADDR_T));
    
    
  
    return 0;   /* suppose community is correct */
}
#endif