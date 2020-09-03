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
 *  Description: the interface for the communication between the ICs and the POE software
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_IC_COMM_H_
	#define _MSCC_POE_IC_COMM_H_

	#ifdef __cplusplus
		extern "C" {
	#endif


	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
	#include "mscc_poe_global_types.h"


	/*=========================================================================
	/ PROTOTYPES
	/========================================================================*/
	S32 mscc_IC_COMM_I2C_Write(_IN U8 I2C_Address,_IN U16 RegisterAddress,  _IN const U8* pTxdata, _IN U16 number_of_bytes_to_write);
	S32 mscc_IC_COMM_I2C_Read (_IN U8 I2C_Address,_IN U16 RegisterAddress,_OUT  U8* pRxdata, _IN U16 number_of_bytes_to_read);
	S32 mscc_IC_COMM_ReadDataFromSpecificICAddressAccess(_IN U8 I2C_Address,_IN U16 RegisterAddress,_OUT U16 *pRegisterData);
	S32 mscc_IC_COMM_ReadDataFromSpecificIC(_IN U8 IC_Number,_IN U16 RegisterAddress,_OUT U16 *pRegisterData);
	S32 mscc_IC_COMM_WriteDataToSpecificIC(_IN U8 IC_Number,_IN U16 RegisterAddress,_IN U16 RegisterData);
	S32 mscc_IC_COMM_WriteDataToAllICs(_IN U16 RegisterAddress,_IN U16 RegisterData);
	S32 mscc_IC_COMM_ReadDataFromSpecificPort(_IN U8 ChNumber,_IN U16 RegisterAddr,_OUT U16 *pRegisterData);
	S32 mscc_IC_COMM_WriteDataToSpecificPort(_IN U8 ChNumber,_IN U16 RegisterAddr,_IN U16 RegisterData);


#endif /* _MSCC_POE_IC_COMM_H_ */
