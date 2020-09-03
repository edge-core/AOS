/* MODULE NAME:  dhcp_pom.h
 * PURPOSE: For accessing om through IPC
 * 
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    12/10/2009 - Jimi Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _DHCP_POM_H
#define _DHCP_POM_H

#include "dhcp_om.h"
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DHCP_POM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE/FALSE.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DHCP_POM_InitiateProcessResource(void);

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82Status
 * PURPOSE:
 *     Get dhcp relay option 82 status
 *
 * INPUT:
 *      status     
 *
 * OUTPUT:
 *      status  -- DHCP_OPTION82_ENABLE/DHCP_OPTION82_DISABLE.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82Status(UI32_T *status);

/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82Policy
 * PURPOSE:
 *     Get dhcp relay option 82 policy
 *
 * INPUT:
 *      policy     
 *
 * OUTPUT:
 *      policy  -- DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82Policy(UI32_T *policy);

/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82RidMode
 * PURPOSE:
 *     Get dhcp relay option 82 remote id mode
 *
 * INPUT:
 *      mode     
 *
 * OUTPUT:
 *      mode  -- DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82RidMode(UI32_T *mode);

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82RidValue
 * PURPOSE:
 *     Get dhcp relay option 82 remote id value
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      string     --  configured string
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82RidValue(UI8_T *string);

/* FUNCTION	NAME : DHCP_POM_GetRelayServerAddress
 
 * PURPOSE:
 *		Get global relay server addresses.
 *
 * INPUT:
 *		relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_OM_OK	    -- successfully get
 *		DHCP_OM_FAIL	-- Fail to get server ip to interface
 *
 * NOTES:
 *		None.
 */
UI32_T DHCP_POM_GetRelayServerAddress(UI32_T *relay_server);

/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82Format
 * PURPOSE:
 *     Get dhcp relay option 82 subtype format 
 *
 * INPUT:
 *      subtype_format_p
 *
 * OUTPUT:
 *      subtype_format_p
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82Format(BOOL_T *subtype_format_p);


#endif

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCP_POM_GetIfRelayServerAddress
 *------------------------------------------------------------------------------
 * PURPOSE: Get all interface relay server addresses associated with interface.
 * INPUT :  vid_ifindex  -- the interface to be searched for relay server address.
 * OUTPUT:  relay_server -- the pointer to relay server
 * RETURN:  DHCP_OM_OK/DHCP_OM_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCP_POM_GetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER]);
#endif
#endif /*#ifndef _CLUSTER_POM_H*/

