/*-----------------------------------------------------------------------------
 * FILE NAME: XFER_BUF_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for XFER_BUF_MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/25     --- Andy, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef _XFER_BUF_PMGR_MGR_H_
#define _XFER_BUF_PMGR_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h> 
#include <string.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "xfer_buf_mgr.h"

#if (SYS_CPNT_BUFFERMGMT == TRUE)
#include "buffer_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : XFER_BUF_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for XFER_BUF_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void XFER_BUF_PMGR_InitiateProcessResource(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_BUF_PMGR_Allocate                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will malloc xfer_buf                 
 * INPUT   : None                                        
 * OUTPUT  : xbuf                                                         
 * RETURN  : TRUE(Success; return xbuf); FALSE  
 * NOTE    :   
 *------------------------------------------------------------------------
 */
void *XFER_BUF_PMGR_Allocate(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_BUF_MGR_Allocate                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will free bufffer                
 * INPUT   : void *buf_P                                                   
 * OUTPUT  : None                                                         
 * RETURN  : TRUE(Success); FALSE  
 * NOTE    :   
 *------------------------------------------------------------------------
 */
BOOL_T	XFER_BUF_MGR_Free(void *buf_P);

#endif /* #ifndef XFERBUF__PMGR_H */
