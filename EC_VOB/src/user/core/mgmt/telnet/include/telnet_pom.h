/* MODULE NAME:  telnet_pom.h
 * PURPOSE:
 * POM for telnet.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    06/03/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef TELNET_POM_H
#define TELNET_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "telnet_mgr.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_POM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource for TELNET_POM in the calling process.
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
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_POM_InitiateProcessResource(void);


/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_POM_GetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet port from OM.
 * INPUT: 
 *    None.
 *
 * OUTPUT:
 *    *port_p --   port 
 *
 * RETURN:
 *    TELNET_TYPE_RESULT_OK   -- Success
 *    TELNET_TYPE_RESULT_FAIL -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_POM_GetTnpdPort(UI32_T *port_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_POM_GetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet status from OM.
 * INPUT: 
 *    None.
 *
 * OUTPUT:
 *    *port_p --   port 
 *
 * RETURN:
 *    TELNET_TYPE_RESULT_OK   -- Success
 *    TELNET_TYPE_RESULT_FAIL -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_POM_GetTnpdStatus(TELNET_State_T *status_p);

#endif    /* End of CSCA_POM_H */

