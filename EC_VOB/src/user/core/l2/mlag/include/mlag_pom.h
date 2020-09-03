/* =============================================================================
 * MODULE NAME : MLAG_POM.H
 * PURPOSE     : Provide declarations for MLAG inter-CSC data management.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_POM_H
#define MLAG_POM_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "mlag_type.h"

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

/* FUNCTION NAME - MLAG_POM_InitiateProcessResources
 * PURPOSE : Initiate process resources for POM in context of the calling
 *           process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 */
BOOL_T MLAG_POM_InitiateProcessResources();

/* FUNCTION NAME - MLAG_POM_GetGlobalStatus
 * PURPOSE : Get global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_POM_GetGlobalStatus(UI32_T *status_p);

/* FUNCTION NAME - MLAG_POM_GetRunningGlobalStatus
 * PURPOSE : Get running global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : None
 */
SYS_TYPE_Get_Running_Cfg_T MLAG_POM_GetRunningGlobalStatus(UI32_T *status_p);

/* FUNCTION NAME - MLAG_POM_GetDomainEntry
 * PURPOSE : Get a MLAG domain entry.
 * INPUT   : entry_p->domain_id - MLAG domain ID
 * OUTPUT  : entry_p - buffer containing information for a domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_POM_GetDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_POM_GetNextDomainEntry
 * PURPOSE : Get next MLAG domain entry.
 * INPUT   : entry_p->domain_id - MLAG domain ID
 * OUTPUT  : entry_p - buffer containing information for next domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Empty string for domain ID to get the first entry
 */
UI32_T MLAG_POM_GetNextDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_POM_GetMlagEntry
 * PURPOSE : Get a MLAG entry.
 * INPUT   : entry_p->mlag_id - MLAG ID
 * OUTPUT  : entry_p - buffer containing information for a MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_POM_GetMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_POM_GetNextMlagEntry
 * PURPOSE : Get next MLAG entry.
 * INPUT   : entry_p->mlag_id - MLAG ID
 * OUTPUT  : entry_p - buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_POM_GetNextMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_POM_GetNextMlagEntryByDomain
 * PURPOSE : Get next MLAG entry in the given domain.
 * INPUT   : entry_p->mlag_id   -- MLAG ID
 *           entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_POM_GetNextMlagEntryByDomain(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_POM_IsMlagPort
 * PURPOSE : Check whether a logical port is peer link or MLAG member.
 * INPUT   : lport -- logical port to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- peer link or MLAG member
 *           FALSE -- otherwise
 * NOTE    : None
 */
BOOL_T MLAG_POM_IsMlagPort(UI32_T lport);

#endif /* End of MLAG_POM_H */
