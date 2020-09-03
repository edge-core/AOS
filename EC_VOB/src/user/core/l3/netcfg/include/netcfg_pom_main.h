/* MODULE NAME:  netcfg_pom_main.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    NETCFG_OM_MAIN service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    02/21/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
 
#ifndef NETCFG_POM_MAIN_H
#define NETCFG_POM_MAIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_POM_MAIN_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_POM_IP, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_MAIN_InitiateProcessResource(void);

#endif /* NETCFG_POM_MAIN_H */

