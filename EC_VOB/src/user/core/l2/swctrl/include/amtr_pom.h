/* MODULE NAME:  amtr_pom.h
 * PURPOSE:
 *     For implementations of AMTR POM APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    5/3/2010 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2010
 */
#ifndef AMTR_POM_H
#define AMTR_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "amtr_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AMTR_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AMTR_POM in the calling process.
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
BOOL_T AMTR_POM_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * Function : AMTR_POM_GetPortInfo
 *-----------------------------------------------------------------------------
 * Purpose  : This function get the port Infomation
 * INPUT    : ifindex
 * OUTPUT   : port_info(learning mode, life time, count, protocol)
 * RETURN   : BOOL_T - True : successs, False : failed
 * NOTE     : 
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_POM_GetPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info);

#endif    /* End of AMTR_POM_H */
