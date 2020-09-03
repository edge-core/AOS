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

#ifndef _MSCC_POE_HOST_COMM_H_
	#define _MSCC_POE_HOST_COMM_H_
	
	#ifdef __cplusplus
		extern "C" {
	#endif
			
			
	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
	#include "mscc_poe_global_types.h"
	

	/*=========================================================================
	/ CONSTANTS
	/========================================================================*/			
	
	#define  MSG_SIZE  15  
   	  

	/*=========================================================================
	/ PROTOTYPES
	/========================================================================*/	
	void mscc_POE_HOST_WriteMsgToTxBuffer(_IN U8* pDataArr);  /* This function Write to Transmit fifo */
	S32 mscc_POE_HOST_HostReadMsgFromTxBuffer(_OUT U8* pDataArr);				
	
#endif /* _MSCC_POE_HOST_COMM_H_ */
