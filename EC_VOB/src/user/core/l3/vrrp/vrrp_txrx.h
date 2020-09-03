/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_TXRX.h                                                                 
 *                                                                                      
 * PURPOSE: This package provides the data types used in VRRP algorithm used (RFC 2338)
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009                              
 * -------------------------------------------------------------------------------------*/
 
 

#ifndef __VRRP_TXRX_H
#define __VRRP_TXRX_H

#include <sys_type.h> 
//#include <sock_port.h>
#include "vrrp_type.h"


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_SendAdvertisement
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will send ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
BOOL_T VRRP_TXRX_SendAdvertisement(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_SendGratuitousArp
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will send gratuitous ARP packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
BOOL_T VRRP_TXRX_SendGratuitousArp(VRRP_OPER_ENTRY_T *vrrp_oper_entry);


/* FUNCTION : VRRP_TXRX_SendGratuitousArpWithVlanMac
 * PURPOSE  : Send a gratuitous ARP packet with the mac address of the VLAN 
 *            and the IP address of the device.
 * USES     : When the VRRP MASTER is the IP owner, if it shutdown,
 *            we need to tell the local network that the VRRP MAC address
 *            is not valid anymore and the physical MAC address of the device
 *            shall be used instead.
 * PARAM.   : VRRP_OPER_ENTRY_T *vrrp_oper_entry
 *            UI8_T *vlan_mac_p
 *            UI8_T *ipAddress_p  
 * OUTPUT   : 
 * RETURN   : TRUE if the oparation succees, FALSE if the parameters are not valid.
 * NOTES    : Added to solve EPR:ES3628C-PoE-20-00181
 * AUTHOR   : Aris Michael MORGENSTERN
 */
BOOL_T VRRP_TXRX_SendGratuitousArpWithVlanMac(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *vlan_mac_p, UI8_T *ipAddress_p);

#endif


