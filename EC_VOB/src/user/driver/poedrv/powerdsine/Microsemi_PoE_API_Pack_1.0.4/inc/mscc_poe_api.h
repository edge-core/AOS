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
 *  File Revision: 1.1 
 *
 *************************************************************************  
 *
 *  Description: API for POE software
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_API_H_
	#define _MSCC_POE_API_H_
	
	#ifdef __cplusplus
		extern "C" {
	#endif
				
	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
		
	#include "mscc_poe_global_types.h"
			
	
	/*=========================================================================
	/ PROTOTYPES
	/=======================================================================*/
	mscc_POE_STATUS_e MSCC_POE_Init(_IN mscc_InitInfo_t *pInitInfo,_OUT S32 *pDevice_error);
	mscc_POE_STATUS_e MSCC_POE_Write(_IN U8* pTxdata,_IN U16 num_write_length,_OUT S32 *pDevice_error);			
	mscc_POE_STATUS_e MSCC_POE_Read (_OUT U8* pRxdata,_IN U16 num_read_length,_OUT S32 *pDevice_error);
	mscc_POE_STATUS_e MSCC_POE_Exit(_IN mscc_CloseInfo_t *pMscc_CloseInfo,_OUT S32 *pDevice_error);
	mscc_POE_STATUS_e MSCC_POE_Timer_Tick(_IN U8 IntervalTime_Sec,_OUT S32 *pDevice_error);
						
	#ifdef __cplusplus
		}
	#endif

#endif /* _MSCC_POE_API_H_ */
