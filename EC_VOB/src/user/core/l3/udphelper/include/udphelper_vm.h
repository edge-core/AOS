/* MODULE NAME:  udphelper_vm.h
 * PURPOSE:
 *     This module provides APIs for UDPHELPER CSC to use.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    09/11/2013 - Jimi Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2013
 */


#ifndef _UDPHELPER_VM_H_
#define _UDPHELPER_VM_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "dhcp_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
 
/* DATA TYPE DECLARATIONS
 */

/* EXPORTED FUNCTION PROTOTYPE
 */ 
/*------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_VM_BOOTPRelay
 *------------------------------------------------------------------------
 * FUNCTION: Handle BOOTP packets.
 *
 * INPUT   : 
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 *------------------------------------------------------------------------
 */  
void UDPHELPER_VM_BOOTPRelay(
  UI32_T ifindex,
  DHCP_TYPE_IpHeader_T* ip_pkt,
  DHCP_TYPE_UdpHeader_T* udp_header,
  struct dhcp_packet* bootp_packet
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_VM_UDPRelay
 *------------------------------------------------------------------------
 * FUNCTION: Handle common UDP packets.
 *
 * INPUT   : 
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 *------------------------------------------------------------------------
 */  
void UDPHELPER_VM_UDPRelay(
  UI32_T ifindex,
  DHCP_TYPE_IpHeader_T* ip_pkt,
  DHCP_TYPE_UdpHeader_T* udp_header
);

void UDPHELPER_VM_SetDebugStatus(UI32_T status);
#endif 
