/* MODULE NAME:  vrrp_pom.h
 * PURPOSE: For accessing om through IPC
 * 
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    2014/ 3/ 10    -- Jimi Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2014
 */

#ifndef _VRRP_POM_H
#define _VRRP_POM_H

/* INCLUDE FILE DECLARATIONS 
 */
#include "vrrp_om.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* EXPORTED SUBPROGRAM DELARATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VRRP_POM_InitiateProcessResource(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP operation entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_POM_GetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetNextVrrpAssoIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the next availabe associated IP address of the
 *            vrrp on the specific interface.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address
 * OUTPUT   : returns the next associated ip address info
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_POM_GetNextVrrpAssoIpAddress(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get specified vrrp associated IP entry by ifindex and ip address
 * INPUT    : associated_info.ip_addr -- specify which ip address to search
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK /
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : None
 *--------------------------------------------------------------------------*/
UI32_T VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info);

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_POM_GetPingStatus(UI32_T *ping_status);
#endif
#endif
