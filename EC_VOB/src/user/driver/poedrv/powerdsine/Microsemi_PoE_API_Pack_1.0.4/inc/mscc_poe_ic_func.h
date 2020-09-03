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
 *  Description: Contains System oriented functions
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_IC_FUNC_H_
	#define _MSCC_POE_IC_FUNC_H_

	#ifdef __cplusplus
		extern "C" {
	#endif


	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
	#include "mscc_poe_global_types.h"
	#include "mscc_poe_db.h" /* include peripheral declarations */
	#include "mscc_poe_cmd.h"


	/*=========================================================================
	/ GLOBALS
	/========================================================================*/

extern	mscc_FPTR_Write mscc_fptr_write;
extern	mscc_FPTR_Read mscc_fptr_read;
extern	void *mscc_pUserData;


	/*=========================================================================
	/ PROTOTYPES
	/========================================================================*/

	/*    InitSoftware functions      */

	S32  mscc_POE_IC_FUNC_Log2Phy(_IN U8 phyChannelNumber,_OUT U8 *pPhyIcNumber ,_OUT U8 *pPhyPortNumber);
	S32  mscc_ICAutoDetection(void);
	U8   mscc_POE_IC_FUNC_IsICNumberValid(_IN U8 ICNum);
	S32  mscc_POE_IC_FUNC_GetActiveSlaveList(_IN U8 *pDeviceFail);
	S32  mscc_POE_IC_FUNC_Get_IC_STATUS(_IN U8 IC_Exp,_IN U8 IC_HW,_OUT U8 *pICStatus,_OUT U8 *pIC_Ports);
	S32  mscc_InitSoftware(_IN mscc_InitInfo_t *pMscc_InitInfo);



#endif /* _MSCC_POE_IC_FUNC_H_ */
