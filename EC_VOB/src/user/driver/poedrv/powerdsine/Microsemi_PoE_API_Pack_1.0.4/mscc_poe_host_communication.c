/*************************************************************************
 *
 *            Copyright (c) 2008 by Microsemi Corp. Inc.
 *
 *  This software is copyrighted by, and is the sole property of Microsemi
 *  Corp. All rights, title, ownership, or other interests in the
 *  software  remain the property of Microsemi Corp. This software
 *  may only be used in accordance with the corresponding license
 *  agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice may not be removed or modified without prior
 *  written consent of Microsemi Corp.
 *
 *  Microsemi Corp. reserves the right to modify this software without
 *  notice. 
 *
 *************************************************************************
 *
 *  File Revision: 1.0 
 *
 *************************************************************************  
 *
 *  Description: the interface for the communication between the host and the POE software 
 *
 *
 *************************************************************************/

/*=========================================================================
/ INCLUDES
/========================================================================*/
#include "mscc_poe_host_communication.h"
#include "mscc_poe_global_types.h"
/*=========================================================================
/ GLOBALS
/========================================================================*/

static U8 ProtocolDataBuffer[MSG_SIZE];
static U8 IsMsgReady = POE_FALSE;

/*=========================================================================
/ FUNCTIONS
/========================================================================*/

/*---------------------------------------------------------------------        												                                                                                                                                           
 *    description: This function write the result to Recive fifo When command process finished  - 
 * 				   this is the result of POE_Write operation.
 * 				   The host should read that message.      	 			       
 *		            							     				   
 *    input :   pDataArr                  	- byte data array which contain 15 result protocol bytes.	   
 * 
 *    output:   none             
 *                     
 *    return:   e_POE_STATUS_OK         	- operation succeed 
 * 			    != e_POE_STATUS_OK          - operation failed                                     
 *---------------------------------------------------------------------*/
void mscc_POE_HOST_WriteMsgToTxBuffer(_IN U8* pDataArr)
{	 	 		 
	U8 i;		 	
	for(i=0;i < PACKAGE_SIZE;i++)
		ProtocolDataBuffer[i] = pDataArr[i]; 	
	
	IsMsgReady = POE_TRUE;
}



/*---------------------------------------------------------------------        												                                                                                                                                           
 *    description:    This function read from Recive fifo 
 * 					  The Host Reads 15 result protocol bytes from the Buffer 
 * 				      this the result of POE_Write operation.       	 			       
 *		            							     				   
 *    input :   none	   
 * 
 *    output:   pDataArr                  	- byte data array which contain 15 result protocol bytes.             
 *                     
 *    return:   e_POE_STATUS_OK         	- operation succeed 
 * 			    != e_POE_STATUS_OK          - operation failed                                     
 *---------------------------------------------------------------------*/
S32 mscc_POE_HOST_HostReadMsgFromTxBuffer(_OUT U8* pDataArr)
{		
	U8 i;
	
	if( IsMsgReady == POE_FALSE)
		return e_POE_STATUS_ERR_MSG_NOT_READY;
	
	for(i=0;i < PACKAGE_SIZE;i++)
		pDataArr[i] = ProtocolDataBuffer[i]; 
	
	return e_POE_STATUS_OK;
}
