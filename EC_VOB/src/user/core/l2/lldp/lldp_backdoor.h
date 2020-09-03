/*-----------------------------------------------------------------------------
 * Module Name: lldp_backdoor.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP backdoor
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/17/2005 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 */
#ifndef _LLDP_BACKDOOR_H
#define _LLDP_BACKDOOR_H

#define LLDP_BACKDOOR_DEBUG_MED    0x00000001

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LLDP_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_BACKDOOR_Main() ;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LLDP_BACKDOOR_GetDebugFlag
 * ------------------------------------------------------------------------
 * FUNCTION : This function is to get debug flag
 * INPUT    : id -- LLDP_BACKDOOR_DEBUG_XXX
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_BACKDOOR_GetDebugFlag(UI32_T id);

#endif /* End of LLDP_BACKDOOR_H */
