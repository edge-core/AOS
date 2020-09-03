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
 *  Description:
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_UTIL_H_
	#define _MSCC_POE_UTIL_H_

	#ifdef __cplusplus
		extern "C" {
	#endif


	/*=========================================================================
	/ INCLUDES
	/========================================================================*/

	#include "mscc_poe_global_types.h"
	#include "mscc_poe_db.h" /* include peripheral declarations */


	#define mscc_MIN(a, b)	((a) < (b) ? (a) : (b))
	#define mscc_MAX(a, b)	((a) > (b) ? (a) : (b))

	/*=========================================================================
	/ PROTOTYPES
	/========================================================================*/
	U16 mscc_GetExtractData(_IN U8 low, _IN U8 high);
	U8 mscc_GetFirstByte(_IN U16 data);
	U8 mscc_GetSecondByte(_IN U16 data);

	U16 mscc_POE_UTIL_CommCalculateCheckSum(_IN U8 dataArr[]);
	U16 mscc_POE_UTIL_CommCheckSum(_IN U8 dataArr[]) ;
	mscc_POE_STATUS_e mscc_POE_UTIL_GetPOE_STATUS_ERR_value(_IN S32 error_value);
	void mscc_POE_UTIL_CommMakeCheckSum(_INOUT U8 dataArr[]);
	void mscc_POE_UTIL_GetRegisterData(U8 bitStartPosition,U8 bitLength, U16 RegData, U8 ModifyData,U16 *pOutputRegData);
	void MSCC_POE_UTIL_PrintStatus(_IN mscc_POE_STATUS_e ePOE_STATUS);

	U16  mscc_POE_UTIL_CheckAppearanceTwiceMatrix			 ();
	U16  mscc_POE_UTIL_ConvertDeciVoltToICValue             (_IN U16 DeciVoltageValue);
	U16  mscc_POE_UTIL_ConvertICValueToDeciVolt             (_IN U16 ICDeciVoltageValue);
	U16  mscc_POE_UTIL_ConvertPortValueToDeciVolt             (_IN U16 ICDeciVoltageValue);
	U16  mscc_POE_UTIL_ConvertICValueTo_ma                  (_IN U16 ICCurrentValue);
	U16  mscc_POE_UTIL_ConvertFromDeciWattToWatts             (_IN U16 ICDeciWattsValue);
	U16  mscc_POE_UTIL_ConvertICValueTo_mw                  (_IN U16 ICPowerValue);
	U16  mscc_POE_UTIL_CelziusToICValue                     (_IN U8 CelziusValue);
	U8   mscc_POE_UTIL_ICValueToCelzius                     (_IN U16 ICTemperatureValue);
	U16  mscc_POE_UTIL_CalcAbsValue                           (_IN U16 val1,_IN U16 val2);


	#ifdef __cplusplus
		}
	#endif

#endif /* _MSCC_POE_UTIL_H_ */
