#ifndef PSEC_POM_H
#define PSEC_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_POM_InitiateProcessResource
 * ---------------------------------------------------------------------
 * PURPOSE  : Initiate resource for PSEC_POM in the calling process.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : TRUE, success; else return FALSE;
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_POM_InitiateProcessResource(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_POM_GetMaxMacCount
 * ---------------------------------------------------------------------
 * PURPOSE  : This api will call PSEC_OM_GetMaxMacCount through the IPC msgq.
 * INPUT    : lport           -- Logic port
 * OUTPUT   : mac_count       -- Max MAC count
 * RETURN   : TRUE, success; else return FALSE;
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_POM_GetMaxMacCount(UI32_T lport, UI32_T *mac_count);

#endif /* #ifndef PSEC_POM_H */
