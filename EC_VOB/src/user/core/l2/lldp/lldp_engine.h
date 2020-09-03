/*-----------------------------------------------------------------------------
 * Module Name: lldp_engine.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP engine
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
#ifndef LLDP_ENGINE_H
#define LLDP_ENGINE_H

#include "sys_type.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusDisableTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Disable the transmit mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_AdminStatusDisableTx(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusDisableRx
 *-------------------------------------------------------------------------
 * PURPOSE  : Disable the receive mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_AdminStatusDisableRx(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusEnableTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Enable the LLDPDU transmit mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_AdminStatusEnableTx(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusEnableRx
 *-------------------------------------------------------------------------
 * PURPOSE  : Enable the LLDPDU receive mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 *
 */
UI32_T LLDP_ENGINE_AdminStatusEnableRx(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_SomethingChangedLocal
 *-------------------------------------------------------------------------
 * PURPOSE  : When something changed local, must fire lldpdu if possible
 * INPUT    : UI32_T lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 10.5.4.1, IEEE Std 802.1AB-2005.
 *            this funtion needn't to enter CSC and om critical section
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_SomethingChangedLocal(UI32_T  lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_SomethingChangedRemote
 *-------------------------------------------------------------------------
 * PURPOSE  : When something changed local, some action must be done
 * INPUT    : UI32_T lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 10.5.4.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_SomethingChangedRemote(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_ProcessReinitDelayPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Waking up the port which is in reinit delay and be configured
 *            to tx enabled
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref. to the description in 10.5.3.3, (c), IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_ProcessReinitDelayPort() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_ProcessTimerEventTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Check each port if the LLDPDU transmission is necessary
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref. to the description int 10.5.3.3, (d), IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_ProcessTimerEventTx() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_ConstructLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct LLDPDU packet
 * INPUT    : lport, LLDPDU type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_ConstructLLDPDU(UI32_T lport, UI32_T type) ;

#endif /* End of LLDP_ENGINE_H */
