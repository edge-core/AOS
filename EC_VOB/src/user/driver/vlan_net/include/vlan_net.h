/* MODULE NAME:  vlan_net.h
 * PURPOSE:
 *     For manipulate craft port net device in kernel space.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    8/31/2010 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2010
 */
#ifndef VLAN_NET_H
#define VLAN_NET_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_mm.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_NET_SendPacketToCraftPortNetDevice
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Send a packet to craft port net device.
 *
 * 	INPUT:
 *   None.
 *
 * 	OUTPUT:
 *   pkt_buffer - packet to be sent to craft port net device.
 *   pkt_len    - packet length
 *
 * 	RETURN:
 *   None.
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
void VLAN_NET_SendPacketToCraftPortNetDevice(UI8_T * pkt_buffer, UI32_T pkt_len);

#endif    /* End of VLAN_NET_H */

