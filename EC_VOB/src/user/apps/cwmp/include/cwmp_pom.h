/*-----------------------------------------------------------------------------
 * FILE NAME: CWMP_POM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/12/26     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef CWMP_POM_H
#define CWMP_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "cwmp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : CWMP_POM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for CWMP_POM in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T CWMP_POM_InitiateProcessResources(void);

BOOL_T CWMP_POM_GetUrl(char *url);

UI32_T CWMP_POM_GetRunningUrl(char *url);

BOOL_T CWMP_POM_GetUsername(char *username);

UI32_T CWMP_POM_GetRunningUsername(char *username);

BOOL_T CWMP_POM_GetPassword(char *password);

UI32_T CWMP_POM_GetRunningPassword(char *password);

BOOL_T CWMP_POM_GetPeriodicInformEnable(BOOL_T *status);

UI32_T CWMP_POM_GetRunningPeriodicInformEnable(BOOL_T *status);

BOOL_T CWMP_POM_GetPeriodicInformInterval(UI32_T *interval);

UI32_T CWMP_POM_GetRunningPeriodicInformInterval(UI32_T *interval);

BOOL_T CWMP_POM_GetConnectionRequestUsername(char* cr_username);

UI32_T CWMP_POM_GetRunningConnectionRequestUsername(char* cr_username);

BOOL_T CWMP_POM_GetConnectionRequestPassword(char* cr_password);

UI32_T CWMP_POM_GetRunningConnectionRequestPassword(char* cr_password);

BOOL_T CWMP_POM_GetCwmpConfigEntry(CWMP_TYPE_ConfigEntry_T *config_entry_p);


#endif /* #ifndef CWMP_POM_H */
