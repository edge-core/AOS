/******************************************************************************
 * Filename: ospf6_proc_comm.c
 * File Description:
 *        
 * Copyright (C) 2005-2007 unknown, Inc. All Rights Reserved.
 *        
 * Author: steven.gao
 *        
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

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "ospf6_proc_comm.h"

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
static L_THREADGRP_Handle_T ospf6_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  OSPF6_PROC_COMM_InitiateProcessResources
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
 *
 */
BOOL_T OSPF6_PROC_COMM_InitiateProcessResources(void)
{
    BOOL_T ret = TRUE;

    ospf6_tg_handle = L_THREADGRP_Create();
    if(ospf6_tg_handle == NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/* FUNCTION NAME:  OSPF6_PROC_COMM_GetOspfTGHandle
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
 *
 */
L_THREADGRP_Handle_T OSPF6_PROC_COMM_GetOspfTGHandle(void)
{
    return ospf6_tg_handle;
}



