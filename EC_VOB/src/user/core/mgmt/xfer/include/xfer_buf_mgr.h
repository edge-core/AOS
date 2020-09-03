/* Project Name: Mercury 
 * Module Name : XFER_BUF_MGR.h
 * Abstract    : 
 * Purpose     : 
 *
 * History :                                                               
 *          Date        		Modifier        Reason
 *          2001/10/11    BECK CHEN     Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 ,2001
 *
 * Note    : 
 */

#ifndef _XFER_BUF_MGR_H_
#define _XFER_BUF_MGR_H_



/*
 *   INCLUDE STRUCTURES                             
 */
#include "sys_type.h"



/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_BUF_MGR_Allocate                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will malloc xfer_buf                 
 * INPUT   : None                                               
 * OUTPUT  : xbuf                                                         
 * RETURN  : TRUE(Success;return xbuf); FALSE           
 * NOTE    :   
 *------------------------------------------------------------------------
 */
void *XFER_BUF_MGR_Allocate(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_BUF_MGR_Allocate                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will free bufffer                
 * INPUT   : void *buf_P                                                
 * OUTPUT  : None                                                         
 * RETURN  :TRUE(Success) ;FALSE                               
 * NOTE    :   
 *------------------------------------------------------------------------
 */
BOOL_T	XFER_BUF_MGR_Free(void *buf_P);


#endif /* end _XFER_BUF_MGR_H_ */



