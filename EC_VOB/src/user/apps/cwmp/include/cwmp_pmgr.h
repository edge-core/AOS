/*-----------------------------------------------------------------------------
 * FILE NAME: CWMP_PMGR.H
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

#ifndef CWMP_PMGR_H
#define CWMP_PMGR_H


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

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : CWMP_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for CWMP_PMGR in the calling process.
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
BOOL_T CWMP_PMGR_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetUrl
 *-----------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.URL" to
 *           the configuration file.
 *
 * INPUT   : url -- URL for the device to contact the ACS
 *                  (must have been allocated 257 bytes)
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : It may be necessary to change username or password for
 *           connecting to the ACS URL which is set by the function.
 *-----------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetUrl(char* url);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetUsername
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.Username"
 *           to the configuration file.
 * INPUT   : username -- username for the device to contact the ACS
 *                       (must have been allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetUsername(char* username);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetPassword
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.Password"
 *           to the configuration file.
 * INPUT   : password -- password for the device to contact the ACS
 *                       (must have been allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetPassword(char* password);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetPeriodicInformEnable
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           PeriodicInformEnable" to the configuration file.
 * INPUT   : status -- whether Inform must be sent periodically
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetPeriodicInformEnable(BOOL_T status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetPeriodicInformInterval
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           PeriodicInformInterval" to the configuration file.
 * INPUT   : interval - the duration in seconds between periodic Informs
 *                      (must be greater than 0)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetPeriodicInformInterval(UI32_T interval);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetConnectionRequestUsername
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           ConnectionRequestUsername" to the configuration file.
 * INPUT   : cr_username -- username for the ACS to contact the device
 *                          (must be allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetConnectionRequestUsername(char* cr_username);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_SetConnectionRequestPassword
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           ConnectionRequestPassword" to the configuration file.
 * INPUT   : cr_password -- password for the ACS to contact the device
 *                          (must be allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_PMGR_SetConnectionRequestPassword(char* cr_password);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_PMGR_NotifyConnectionRequest
 * ------------------------------------------------------------------------
 * PURPOSE : Notify CWMP that connetion request has been received and
 *           authenticated successfully.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_PMGR_NotifyConnectionRequest(void);


#endif /* #ifndef CWMP_PMGR_H */
