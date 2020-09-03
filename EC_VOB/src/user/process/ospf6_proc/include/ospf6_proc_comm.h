/******************************************************************************
 * Filename: ospf6_proc_comm.h
 * File description:
 *        
 * Copyright (C) 2005-2007 unknown, Inc. All Rights Reserved.
 *        
 * author: steven.gao
 * Create Date: Tuesday, July 14, 2009 
 *        
 * Modify History
 *   Modifyer: 
 *   Date:     
 *   Description:
 *        
 *   Modifyer: 
 *   Date:     
 *   Description:
 *        
 * Version: 
 ******************************************************************************/
#ifndef _OSPF6_PROC_COMM_H
#define _OSPF6_PROC_COMM_H
/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif

/* INCLUDE FILE DECLARATIONS
 */
#include "l_threadgrp.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : OSPF6_PROC_COMM_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in OSPF6 process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
BOOL_T OSPF6_PROC_COMM_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : OSPF6_PROC_COMM_GetOspfTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for OSPF6 CSC Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    The OSPF6 thread group handle.
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T OSPF6_PROC_COMM_GetOspfTGHandle(void);

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif /*_OSPF6_PROC_COMM_H*/

