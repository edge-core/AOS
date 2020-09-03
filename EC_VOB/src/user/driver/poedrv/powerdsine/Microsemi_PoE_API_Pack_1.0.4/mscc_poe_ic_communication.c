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


/*=========================================================================
/ INCLUDES
/========================================================================*/

#include "mscc_poe_ic_func.h"
#include "mscc_poe_ic_communication.h"
#include "mscc_poe_util.h"
#include "mscc_poe_global_types.h"


/*=========================================================================
/ FUNCTIONS
/========================================================================*/

/*---------------------------------------------------------------------
 *    description: Write data byte array to the IC
 *
 *    input :   I2C_Address	                - device I2C address
 * 				RegisterAddress             - address of IC register
 * 				Txdata						- data byte array to transmit
 * 				number_of_bytes_to_write    - number of bytes to write to the IC
 *    output:   none
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          				- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_I2C_Write(_IN U8 I2C_Address,_IN U16 RegisterAddress,  _IN const U8* pTxdata, _IN U16 number_of_bytes_to_write)
{
/*
	U8 i;
	static U8 TxData[BUFFER_SIZE];

	TxData[0] = mscc_GetFirstByte( RegisterAddress);
	TxData[1] = mscc_GetSecondByte( RegisterAddress);

    for (i = 0; i < number_of_bytes_to_write; i++)
    	TxData[i+2] = pTxdata[i];
*/
//    *pDeviceErrorInternal = mscc_fptr_write(I2C_Address,I2C_7_BIT_ADDR_WITH_STOP_CONDITION, TxData, number_of_bytes_to_write+2,mscc_pUserData);
    *pDeviceErrorInternal = mscc_fptr_write(I2C_Address,RegisterAddress, pTxdata, number_of_bytes_to_write,mscc_pUserData);
    if(*pDeviceErrorInternal != e_POE_STATUS_OK)
    	result = e_POE_STATUS_ERR_COMMUNICATION_DRIVER_ERROR;

	return result;
}



/*---------------------------------------------------------------------
 *    description: Read data byte array from IC
 *
 *    input :   I2C_Address	                - device I2C address
 * 				RegisterAddress             - address of IC register
 * 				number_of_bytes_to_read     - number of bytes to read from the IC
 *    output:   Rxdata    					- recieved data byte array
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          				- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_I2C_Read (_IN U8 I2C_Address,_IN U16 RegisterAddress,_OUT  U8* pRxdata, _IN U16 number_of_bytes_to_read)
{
							     /* High regAddr	             low regAddr */
/*
	U8 data_out[] ={mscc_GetFirstByte( RegisterAddress) ,mscc_GetSecondByte( RegisterAddress)};

	*pDeviceErrorInternal =  mscc_fptr_write(I2C_Address,I2C_7_BIT_ADDR_WITHOUT_STOP_CONDITION,data_out, 2,mscc_pUserData);
	if(*pDeviceErrorInternal != e_POE_STATUS_OK)
	{
		result = e_POE_STATUS_ERR_COMMUNICATION_DRIVER_ERROR;
		return result;
	}
*/
//	*pDeviceErrorInternal = mscc_fptr_read (I2C_Address,I2C_7_BIT_ADDR_WITH_STOP_CONDITION,pRxdata, number_of_bytes_to_read,mscc_pUserData);
	*pDeviceErrorInternal = mscc_fptr_read (I2C_Address,RegisterAddress,pRxdata, number_of_bytes_to_read,mscc_pUserData);
	if(*pDeviceErrorInternal != e_POE_STATUS_OK)
	    result = e_POE_STATUS_ERR_COMMUNICATION_DRIVER_ERROR;

	return result;
}



/*---------------------------------------------------------------------
 *    description: Read register data from specific IC
 *
 *    input :   IC Number                  - index of IC (0 to 7)
 * 				RegisterAddress             - address of IC register
 *
 * 	  output: 	RegisterData     			- recieved register data
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_ReadDataFromSpecificIC(_IN U8 IC_Number,_IN U16 RegisterAddress,_OUT U16 *pRegisterData)
{
	U8 I2C_Address;

	if(IC_Number >= MAX_IC_ON_BOARD)
		return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(37));

	I2C_Address = IcParams[IC_Number].IC_Address;

	return mscc_IC_COMM_ReadDataFromSpecificICAddressAccess(_IN I2C_Address,_IN RegisterAddress,_OUT pRegisterData);
}




/*---------------------------------------------------------------------
 *    description: Read register data from specific IC
 *
 *    input :   IC Number                  - index of IC (0 to 7)
 * 				RegisterAddress             - address of IC register
 *
 * 	  output: 	RegisterData     			- recieved register data
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_ReadDataFromSpecificICAddressAccess(_IN U8 I2C_Address,_IN U16 RegisterAddress,_OUT U16 *pRegisterData)
{
	static U8 RxData[2];

	result = mscc_IC_COMM_I2C_Read (I2C_Address,RegisterAddress, RxData, 2);
	if(result != e_POE_STATUS_OK)
		return result;

	*pRegisterData = mscc_GetExtractData( RxData[0] ,  RxData[1]);

	return result;
}




/*---------------------------------------------------------------------
 *    description: Write register data to specific IC
 *
 *    input :   IC Number                  - index of IC (0 to 7)
 * 				RegisterAddress             - address of IC register
 *  			RegisterData     			- data to write to the register
 *
 * 	  output: 	none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_WriteDataToSpecificIC(_IN U8 IC_Number,_IN U16 RegisterAddress,_IN U16 RegisterData)
{
	U8 TxData[2];
	U8 IC_I2C_Address;

	if(IC_Number >= MAX_IC_ON_BOARD)
		return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(38));

	TxData[0] = mscc_GetFirstByte(RegisterData);
	TxData[1] = mscc_GetSecondByte(RegisterData);

    IC_I2C_Address = IcParams[IC_Number].IC_Address;

    return mscc_IC_COMM_I2C_Write(IC_I2C_Address, RegisterAddress, TxData, 2);
}



/*---------------------------------------------------------------------
 *    description: Write register data to all ICs by sending Broadcast transmission.
 *
 *    input :	RegisterAddress             - address of IC register
 *  			RegisterData     			- data to write to the register
 *
 * 	  output: 	none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_WriteDataToAllICs(_IN U16 RegisterAddress,_IN U16 RegisterData)
{
	U8 TxData[2];

	TxData[0] = mscc_GetFirstByte( RegisterData );
	TxData[1] = mscc_GetSecondByte( RegisterData );

	return mscc_IC_COMM_I2C_Write(BROADCAST_I2C_ADDRESS, RegisterAddress, TxData, 2);
}



/*---------------------------------------------------------------------
 *    description: Read register data from specific logical port
 *
 *    input :   ChNumber                  	- logical port number
 * 				RegisterAddress             - address of IC register
 *
 * 	  output: 	RegisterData     			- recieved register data
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_ReadDataFromSpecificPort(_IN U8 ChNumber,_IN U16 RegisterAddr,_OUT U16 *pRegisterData)
{

	U8 PhyIcNumber, PhyPortNumber;

	result = mscc_POE_IC_FUNC_Log2Phy(ChNumber,&PhyIcNumber ,&PhyPortNumber);
	if(result != e_POE_STATUS_OK)
		return result;

	RegisterAddr += PhyPortNumber * 2; 	/* calculate actual port address */

	return mscc_IC_COMM_ReadDataFromSpecificIC(PhyIcNumber,RegisterAddr,pRegisterData);
}



/*---------------------------------------------------------------------
 *    description: Write register data to specific logical port
 *
 *    input :   ChNumber                  	- logical port number
 * 				RegisterAddress             - address of IC register
 * 	   			RegisterData     			- data to write to the register
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_IC_COMM_WriteDataToSpecificPort(_IN U8 ChNumber,_IN U16 RegisterAddr,_IN U16 RegisterData)
{
	U8 PhyIcNumber, PhyPortNumber;
	U8 IC_I2C_Address;
	U8 TxData[2];

	result = mscc_POE_IC_FUNC_Log2Phy(ChNumber,&PhyIcNumber ,&PhyPortNumber);

    IC_I2C_Address = IcParams[PhyIcNumber].IC_Address;

    TxData[0] = mscc_GetFirstByte( RegisterData);
    TxData[1] = mscc_GetSecondByte( RegisterData);

    RegisterAddr += PhyPortNumber * 2; 	/* calculate actual port address */

    return mscc_IC_COMM_I2C_Write(IC_I2C_Address, RegisterAddr, TxData, 2);
}
