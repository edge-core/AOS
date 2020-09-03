/* MODULE NAME : netaccess_pom.h
 * PURPOSE  :
 *    This file declares the APIs for NETACCESS OM IPC.
 * NOTES    :
 *
 * HISTORY  :
 *   mm/dd/yy (A.D.)
 *    6/23/2008 - Squid Ro, Created
 *
 *
 * Copyright(C)     Accton Corporation, 2008
 */

#ifndef NETACCESS_POM_H
#define NETACCESS_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NETACCESS_POM_InitiateProcessResource
 * ------------------------------------------------------------------------
 *  PURPOSE: Initiate resource for NETACCESS POM in the calling process.
 *  INPUT:   None.
 *  OUTPUT:  None.
 *  RETURN:  None.
 *  NOTE:    None.
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_POM_InitiateProcessResource(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_POM_IsSecurityPort
 * ---------------------------------------------------------------------
 * PURPOSE: Check if security related function is enabled on lport
 * INPUT:   lport.
 * OUTPUT:  is_enabled.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_POM_IsSecurityPort(UI32_T lport, BOOL_T *is_enabled);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /* #ifndef NETACCESS_POM_H */

