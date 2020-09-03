/* =============================================================================
 * MODULE NAME : MLAG_PMGR.H
 * PURPOSE     : Provide declarations for MLAG IPC operational functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_PMGR_H
#define MLAG_PMGR_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM SPECIFICATIONS
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_PMGR_InitiateProcessResources
 * PURPOSE : Initiate process resources for PMGR in context of the calling
 *           process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 */
BOOL_T MLAG_PMGR_InitiateProcessResources();

/* FUNCTION NAME - MLAG_PMGR_SetGlobalStatus
 * PURPOSE : Set global status for the feature.
 * INPUT   : status -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                     MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_PMGR_SetGlobalStatus(UI32_T status);

/* FUNCTION NAME - MLAG_PMGR_SetDomain
 * PURPOSE : Set a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 *           lport       -- peer link
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Replace if the domain ID has existed
 */
UI32_T MLAG_PMGR_SetDomain(char *domain_id_p, UI32_T lport);

/* FUNCTION NAME - MLAG_PMGR_RemoveDomain
 * PURPOSE : Remove a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Success if the domain ID does not exist
 */
UI32_T MLAG_PMGR_RemoveDomain(char *domain_id_p);

/* FUNCTION NAME - MLAG_PMGR_SetMlag
 * PURPOSE : Set a MLAG.
 * INPUT   : mlag_id     -- MLAG ID
 *           lport       -- MLAG member
 *           domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Replace if the MLAG ID has existed
 */
UI32_T MLAG_PMGR_SetMlag(UI32_T mlag_id, UI32_T lport, char *domain_id_p);

/* FUNCTION NAME - MLAG_PMGR_RemoveMlag
 * PURPOSE : Remove a MLAG.
 * INPUT   : mlag_id -- MLAG ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_PMGR_RemoveMlag(UI32_T mlag_id);

#endif /* End of MLAG_PMGR_H */
