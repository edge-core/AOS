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

/*=========================================================================
/ INCLUDES
/========================================================================*/
#include "mscc_poe_util.h"
#include <stdio.h>


/*=========================================================================
/ FUNCTIONS
/========================================================================*/


/*---------------------------------------------------------------------
 *    description: Get the extracted data word that received from the IC
 *
 *    input :  low				- low byte that received from the IC
 *             high             - high byte that received from the IC
 *    output:  none
 *    return:  first byte to transmit to the IC
 *---------------------------------------------------------------------*/
U16 mscc_GetExtractData(_IN U8 low, _IN U8 high)
{
    return (low << 8) + high;
}


/*---------------------------------------------------------------------
 *    description: Get the first byte of a word to transmit to the IC
 *
 *    input :  data				- register data
 *    output:  none
 *    return:  first byte to transmit to the IC
 *---------------------------------------------------------------------*/
U8 mscc_GetFirstByte(_IN U16 data)
{
     return (U8)(data >> 8);
}


/*---------------------------------------------------------------------
 *    description: Get the second byte of a word to transmit to the IC
 *
 *    input :  data				- register data
 *    output:  none
 *    return:  second byte to transmit to the IC
 *---------------------------------------------------------------------*/
U8 mscc_GetSecondByte(_IN U16 data)
{
     return (U8)(data);
}


/*---------------------------------------------------------------------
 *    description: CALCULATE COMM CHECKSUM
 *
 *    input :  dataArr    - data bytes array
 *    output:  none
 *    return:  calculated check sum
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_CommCalculateCheckSum(_IN U8 dataArr[])
{
	U8 i=0;
	U16 sum=0;

	for (i=0; i < PACKAGE_SIZE-2; i++)
		sum += dataArr[i];

	return sum;
}


/*---------------------------------------------------------------------
 *    description: calculate the checkSum from data and compare it to the CheckSum fields.
 *
 *    input :   dataArr    - data bytes array
 *    output:   none
 *    return:   POE_TRUE - if Check sum OK
 * 				POE_FALSE - if check sum error
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_CommCheckSum(_IN U8 dataArr[])
{
	U16 CheckSum;

	CheckSum=mscc_POE_UTIL_CommCalculateCheckSum(dataArr);
	if (CheckSum != mscc_GetExtractData(dataArr[PACKAGE_SIZE-2], dataArr[PACKAGE_SIZE-1]))
		return (POE_FALSE);

	return POE_TRUE;
}






/*---------------------------------------------------------------------
 *    description: check if error value is in range
 *
 *    input :   error_value    - error value
 *    output:   none
 *    return:   mscc_POE_STATUS_e
 *---------------------------------------------------------------------*/
mscc_POE_STATUS_e mscc_POE_UTIL_GetPOE_STATUS_ERR_value(_IN S32 error_value)
{
	if(error_value >= e_POE_STATUS_OK)
		return e_POE_STATUS_OK;
	else if(error_value <= e_POE_STATUS_ERR_MAX_ENUM_VALUE)
		return e_POE_STATUS_UNKNOWN_ERR;
	else
		return error_value;
}




/*---------------------------------------------------------------------
 *    description: calculate the checkSum and update the CheckSum fields.
 *
 *    input :   dataArr    - data bytes array
 *    output:   dataArr    - add checksum at the end of the 15 bytes data
 *    return:   operation status
 *
 *---------------------------------------------------------------------*/
void mscc_POE_UTIL_CommMakeCheckSum(_INOUT U8 dataArr[])
{
	U16 CheckSum=0;

	CheckSum=mscc_POE_UTIL_CommCalculateCheckSum(dataArr);

	dataArr[PACKAGE_SIZE-2] = mscc_GetFirstByte(CheckSum);
	dataArr[PACKAGE_SIZE-1] = mscc_GetSecondByte(CheckSum);
}


/*---------------------------------------------------------------------
 *    description: Get specific data field from Data Register
 *
 *    input :  bitStartPosition - the bit position of which the field start in the data register
 * 			   bitLength        - the data length of the field
 * 			   RegData			- the actual register data
 * 			   ModifyData       - the data which we want to write to the field in the register
 *    output:  OutputRegData    - the result of the modification
 *    return:  none
 *---------------------------------------------------------------------*/
void mscc_POE_UTIL_GetRegisterData(_IN U8 bitStartPosition,_IN U8 bitLength, _IN U16 RegData, _IN U8 ModifyData,_OUT U16 *pOutputRegData)
{
	 U8 i;
     U8 shiftBit = 1;

     shiftBit <<= bitStartPosition;

     for (i = 0; i < bitLength; i++)
     {
        RegData &= ~shiftBit;
        shiftBit <<= 1;
     }

     ModifyData <<= bitStartPosition;
     RegData |= ModifyData;
     *pOutputRegData = RegData;
}



/*---------------------------------------------------------------------
 *    description: print description according to the input mscc_POE_STATUS_e number
 *
 *    input :  mscc_POE_STATUS_e - poe status
 *    output:  none
 *    return:  none
 *---------------------------------------------------------------------*/
void MSCC_POE_UTIL_PrintStatus(_IN mscc_POE_STATUS_e ePOE_STATUS)
{
	switch(ePOE_STATUS)
	{
		case e_POE_STATUS_OK:
		{
			printf("POE OK, status number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_POE_API_SW_INTERNAL:
		{
			printf("POE API SW INTERNAL error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_COMMUNICATION_DRIVER_ERROR:
		{
			printf("COMMUNICATION DRIVER error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_COMMUNICATION_REPORT_ERROR:
		{
			printf("COMMUNICATION REPORT error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_HOST_COMM_MSG_LENGTH_MISMATCH:
		{
			printf("HOST COMM MSG LENGTH MISMATCH error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_PORT_NOT_EXIST:
		{
			printf("PORT NOT EXIST error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_MSG_NOT_READY:
		{
			printf("MESSAGE NOT READY error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_UNKNOWN_ERR:
		{
			printf("UNKNOWN error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_TIMER_INTERVAL_ERROR:
		{
			printf("TIMER INTERVAL value error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_MUTEX_INIT_ERROR:
		{
			printf("MUTEX initialization error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_MUTEX_LOCK_ERROR:
		{
			printf("MUTEX lock error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_MUTEX_UNLOCK_ERROR:
		{
			printf("MUTEX unlock error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR:
		{
			printf("SLEEP FUNCTION error number: %d\n",ePOE_STATUS);
			break;
		}
		case e_POE_STATUS_ERR_MAX_ENUM_VALUE:
		{
			printf("OUT OF RANGE ERROR VALUE number: %d\n",ePOE_STATUS);
			break;
		}
	}
}



/*=========================================================================
/ InitSoftware  functions
/========================================================================*/



/*---------------------------------------------------------------------
 *    description: Check if physical ports Appears twice in a matrix
 *
 *    input :   none
 *    output:   none
 *    return:   POE_TRUE            - Matrix is valid
 * 				POE_FALSE           - Matrix is invalid
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_CheckAppearanceTwiceMatrix()
{
	U8 i, j;

	for (i=0; i<MAX_CH_PER_SYSTEM; i++) {
		for (j=i+1; j<MAX_CH_PER_SYSTEM; j++) {
			if (ChannelParams[i].mscc_Vir2PhyArrTemporaryMatrix == ChannelParams[j].mscc_Vir2PhyArrTemporaryMatrix) /* check if the same number already existing */
				return POE_FALSE; /* fail */
		}
	}
	return POE_TRUE; /* ok */
}



/*---------------------------------------------------------------------
 *    description: Convert Vmain deci volts to IC Value
 *
 *    input :  IC DeciVoltsValue		- Vmain in deci volts units
 *    output:  none
 *    return:  IC register Vmain value
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_ConvertDeciVoltToICValue(_IN U16 DeciVoltageValue)
{
	U16 Tmp;

	Tmp=((U16)(DeciVoltageValue*100)/VMAIN_LSB_FACTOR);
	return Tmp;
}



/*---------------------------------------------------------------------
 *    description: Convert IC Vmain Value To deci volts
 *
 *    input :  IC DeciVoltsValue		- IC register Vmain deci volts value
 *    output:  none
 *    return:  Vmain Voltage value in deci volts units
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_ConvertICValueToDeciVolt(_IN U16 ICDeciVoltageValue)
{
	U32 Tmp;

	Tmp=ICDeciVoltageValue;
	Tmp *= 100;
	Tmp/=VMAIN_FACTOR;
	return (U16)(Tmp+1);
}



/*---------------------------------------------------------------------
 *    description: Convert IC ports register Value To deci volts
 *
 *    input :  IC DeciVoltsValue		- IC ports register deci volts value
 *    output:  none
 *    return:  Ports voltage value in deci volts units
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_ConvertPortValueToDeciVolt(_IN U16 ICDeciVoltageValue)
{
	U32 Tmp;

	Tmp = ICDeciVoltageValue;
	Tmp *= 100;
	Tmp /= VPORT_FACTOR;
	return (U16)Tmp;
}



/*---------------------------------------------------------------------
 *    description: Convert mA to IC Value
 *
 *    input :  ICCurrentValue		- Current value in mA units
 *    output:  none
 *    return:  IC register current value
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_ConvertICValueTo_ma(_IN U16 ICCurrentValue)
{
	long Tmp;

	Tmp=ICCurrentValue;
	Tmp*=CH_CURR_FACTOR;
	Tmp/=200; /* mult factor by 5 & div by 1000, to get 0.305mA */

	return Tmp;
}




/*---------------------------------------------------------------------
 *    description: Convert IC Vmain Value To deci volts
 *
 *    input :  ICDeciVoltsValue		- IC register Vmain deci volts value
 *    output:  none
 *    return:  Vmain Voltage value in deci volts units
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_ConvertFromDeciWattToWatts(_IN U16 ICDeciWattsValue)
{
	return ICDeciWattsValue / 10;
}




/*---------------------------------------------------------------------
 *    description: Convert IC Value To mW
 *
 *    input :  ICPowerValue		- IC register Power value
 *    output:  none
 *    return:  Power value in mW units
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_ConvertICValueTo_mw(_IN U16 ICPowerValue)
{
	long Tmp;

	Tmp=ICPowerValue*10;
	Tmp/=ONE_WATT_FACTOR;
	Tmp*=100;

	return Tmp;
}



/*---------------------------------------------------------------------
 *    description: Convert Celzius to IC Value
 *
 *    input :  ICTemperatureValue		- temperature value in celzius units
 *    output:  none
 *    return:  IC register temperature value
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_CelziusToICValue(_IN U8 CelziusValue)
{
	U32 TmpCalc=1514;

	CelziusValue+=40;
	TmpCalc*=CelziusValue;
	TmpCalc/=10;
	TmpCalc=68400-(U16)TmpCalc;
	TmpCalc/=100;

	return ((U16)TmpCalc);
}



/*---------------------------------------------------------------------
 *    description: Convert IC Value To Celzius
 *
 *    input :  ICTemperatureValue		- IC register temperature value
 *    output:  none
 *    return:  temperature value in celzius units
 *---------------------------------------------------------------------*/
U8 mscc_POE_UTIL_ICValueToCelzius(_IN U16 ICTemperatureValue)
{
	U32 TmpCalc;

	if (ICTemperatureValue > 684)
		TmpCalc = 0;
	else
		TmpCalc = 684 - (U16)ICTemperatureValue;
	TmpCalc *= 1000;
	TmpCalc /= 1514;
	TmpCalc -= 40;

	return ((U8)TmpCalc);
}

/*---------------------------------------------------------------------
 *
 *   CalcAbsValue
 *
 *
 *   This function calcs absolute value of 2 numbers
 *
 *   input :  2 numbers (Val1, Val2)
 *
 *   output:  abs(Val1 - Val2)
 *---------------------------------------------------------------------*/
U16 mscc_POE_UTIL_CalcAbsValue(_IN U16 val1,_IN U16 val2)
{
	U16 delta;

	if(val1 > val2)
		delta = val1 - val2;
	else
		delta = (~val1) + val2 + 1;

	return delta;
}
