/* MODULE NAME:  syslog_pom.h
 * PURPOSE:
 *    POM implement for SYSLOG.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    07/30/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYSLOG_POM_H
#define SYSLOG_POM_H

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
 * ROUTINE NAME : SYSLOG_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for SYSLOG_POM in the calling process.
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
BOOL_T SYSLOG_POM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetFacilityType
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetFacilityType through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_POM_GetFacilityType(UI32_T *facility);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetRemotelogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetRemotelogLevel through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_POM_GetRemotelogLevel(UI32_T *level);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetRemotelogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetRemotelogStatus through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_POM_GetRemotelogStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetServerIPAddr
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetServerIPAddr through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_POM_GetServerIPAddr(UI8_T index, L_INET_AddrIp_T *ip_address);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetSyslogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetSyslogStatus through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_POM_GetSyslogStatus(UI32_T *syslog_status);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetUcLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetUcLogLevel through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_POM_GetUcLogLevel(UI32_T *uc_log_level);

#endif
