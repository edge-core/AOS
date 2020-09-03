/*-----------------------------------------------------------------------------
 * Module Name: lldp_uty.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP utilities
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    03/06/2006 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#ifndef LLDP_UTY_H
#define LLDP_UTY_H
#include "sys_type.h"
#include "lldp_type.h"
#include "lldp_om.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_InsertRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : determine if received lldpdu should be insert into om
 * INPUT    : LLDP_TYPE_MSG_T   *msg        -- incoming message
 *            UI32_T            *tlv_count  -- number of each tlv type
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_ERROR /
 *            LLDP_TYPE_RETURN_TOO_MANY_NEIGHBOR /
 *            LLDP_TYPE_RETURN_OK 
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_UTY_InsertRemData(LLDP_TYPE_MSG_T *msg, UI32_T *tlv_type_count) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ValidateLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Validate LLDPDU
 * INPUT    : msg
 * OUTPUT   : UI32_T    *tlv_type_count -- number of each tlv type
 * RETURN   : None
 * NOTE     : Ref. to the description in 10.3.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_UTY_ValidateLLDPDU(LLDP_TYPE_MSG_T *msg, UI32_T *tlv_type_count) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_RecogLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the type of the packet
 * INPUT    : LLDP_TYPE_MSG_T *msg
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref. to the description in 10.3.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_UTY_RecogLLDPDU(LLDP_TYPE_MSG_T *msg) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructNormalLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct normal LLDPDU
 * INPUT    : sys_config
 *            port_config
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_UTY_ConstructNormalLLDPDU(LLDP_OM_SysConfigEntry_T *sys_config, LLDP_OM_PortConfigEntry_T *port_config) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructShutdownLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct shutdown LLDPDU
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_UTY_ConstructShutdownLLDPDU(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_GetNextLogicalPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next logical port
 * INPUT    : lport
 * OUTPUT   : lport
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_UTY_GetNextLogicalPort(UI32_T *lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_LogicalPortExisting
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next logical port
 * INPUT    : lport
 * OUTPUT   : lport
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_UTY_LogicalPortExisting(UI32_T lport) ;


#endif  /* End of LLDP_UTY_H */
