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

/*=========================================================================
/ INCLUDES
/========================================================================*/
#include "mscc_poe_ic_func.h"
#include "mscc_poe_ic_communication.h"
#include "mscc_poe_ic_param_def.h" /* include peripheral declarations */
#include "mscc_poe_util.h"
#include "mscc_arch_functions.h"

mscc_FPTR_Write mscc_fptr_write;
mscc_FPTR_Read mscc_fptr_read;
void *mscc_pUserData;


/*=========================================================================
/ FUNCTIONS
/========================================================================*/



/*---------------------------------------------------------------------
 *    description: Convert from logical port number to actual IC number and physical port number
 *
 *    input :   logicalChannelNumber		- pointer to a matrix that should be checked
 *    output:   PhyIcNumber 				- IC Number
 * 				PhyPortNumber               - physical port number
 *
 *    return:   e_POE_STATUS_OK         		- conversion succeed
 * 			    != 0          				- conversion failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_IC_FUNC_Log2Phy(_IN U8 phyChannelNumber,_OUT U8 *pPhyIcNumber ,_OUT U8 *pPhyPortNumber)
{
	U8 tempSum;
	U8 IcIndex;
	U8 found;

	if(phyChannelNumber >= MAX_CH_PER_SYSTEM)
		return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(37));

	tempSum = phyChannelNumber;
	found = POE_FALSE;

	for(IcIndex=0 ; IcIndex<MAX_IC_ON_BOARD && (!found) ; IcIndex++)
	{
		if(tempSum < IcParams[IcIndex].NumOfChannesInIC)
		{
			found = POE_TRUE;
			break;
		}
		else
			tempSum -= IcParams[IcIndex].NumOfChannesInIC;
	}

	if(!found)
		return (e_POE_STATUS_ERR_PORT_NOT_EXIST);
	else
	{
		*pPhyIcNumber = IcIndex;
		*pPhyPortNumber = tempSum;
	}

	return e_POE_STATUS_OK;
}








/*---------------------------------------------------------------------
 *    description: This Function Checks How Many ICs Are In The Board
 * 				   And wheather All The ICs are OK
 *
 *    input :   none
 *    output:   DeviceFail 				    - (1)Bits 0 to 7 indicate a failed PoE device(s).
 *														'1' = Fail or missing PoE Device
 *														'0' = PoE Device is OK.
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != 0          				- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_IC_FUNC_GetActiveSlaveList(_IN U8 *pDeviceFail)
{
	U16  DataRead;
	U8 tmpDeviceFail=0xFF;
	U8 IcIndex;

	*pDeviceFail = 0xFF;

	for(IcIndex=0;IcIndex<MAX_IC_ON_BOARD;IcIndex++)
	{
		IcParams[IcIndex].CurrentActiveSlaves = POE_FALSE;	/* init as: Fail or missing PoE Device */
		IcParams[IcIndex].InitialActiveSlaves = POE_FALSE;	/* init as: Fail or missing PoE Device */
	}

	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index, ActiveSlaveList_ADDR,&DataRead);
	if(result != e_POE_STATUS_OK)
		return result;

	DataRead <<= 1;
	DataRead |= 0x0101;	/* master is: PoE Device is OK */

	tmpDeviceFail=0;

	for(IcIndex=0;IcIndex<MAX_IC_ON_BOARD;IcIndex++)
	{
		IcParams[IcIndex].CurrentActiveSlaves = ((DataRead >> IcIndex) & BIT0);
		IcParams[IcIndex].InitialActiveSlaves = ((DataRead >> (IcIndex+8)) & BIT0);

		if((IcParams[IcIndex].InitialActiveSlaves == POE_FALSE) || (IcParams[IcIndex].CurrentActiveSlaves == POE_FALSE))
			tmpDeviceFail |=  BIT0 << IcIndex ;
	}

	*pDeviceFail = tmpDeviceFail;
	return result;
}



/*---------------------------------------------------------------------
 *    description: The purpose of this procedure is to properly assign port numbers per PoE device .
 * 				   The IC Status value is summarizes the relations between IC-Exp, IC-HW & IC-Ports.
 *
 *    input :   IC_Exp					- pointer to a matrix that should be checked
 * 				IC_HW       			- Number of ports verified by the internal communication
 *
 *    output:   ICStatus 				- summarizes the relations between IC-Exp, IC-HW & IC-Ports
 * 				IC_Ports                - allocated number of PoE device ports
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != 0          				- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_IC_FUNC_Get_IC_STATUS(_IN U8 IC_Exp,_IN U8 IC_HW,_OUT U8 *pICStatus,_OUT U8 *pIC_Ports)
{
	/* Check that the IC-number of ports is valid */
	if((IC_Exp !=0) && (IC_Exp !=8) && (IC_Exp !=12))
		return(FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(38));	/* Error - Value out of range */

	if((IC_HW !=0) && (IC_HW !=8) && (IC_HW !=12))
			return(FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(39));	/* Error - Value out of range */

	if((IC_Exp ==0) && (IC_HW ==0))
	{
		*pICStatus = e_IC_Status_None;
		*pIC_Ports = 0;
	}
	else if((IC_Exp == 0) && (IC_HW > 0))
	{
		*pICStatus = e_IC_Status_Unexpected_PoE_detection1;
		*pIC_Ports = IC_HW;
	}
	else if((IC_Exp > 0) && (IC_HW > 0) && (IC_Exp == IC_HW))
	{
		*pICStatus = e_IC_Status_Ok;
		*pIC_Ports = IC_HW;
	}
	else if((IC_Exp > 0) && (IC_HW == 0))
	{
		*pICStatus = e_IC_Status_Fail_Missing_PoE_Device;
		*pIC_Ports = IC_Exp;
	}
	else if((IC_Exp > 0) && (IC_HW > 0) && (IC_Exp < IC_HW))
	{
		*pICStatus = e_IC_Status_Different_PoE_device_was_detected1;
		*pIC_Ports = IC_HW;
	}
	else if((IC_Exp > 0) && (IC_HW > 0) && (IC_Exp > IC_HW))
	{
		*pICStatus = e_IC_Status_Different_PoE_device_was_detected2;
		*pIC_Ports = IC_HW;
	}
	else if((IC_Exp == 0) && (IC_HW > 0))
	{
		*pICStatus = e_IC_Status_Unexpected_PoE_detection2;
		*pIC_Ports = IC_HW;
	}

	return e_POE_STATUS_OK;
}




/*---------------------------------------------------------------------
 *    description:     This function Initialize the PoE API Software - matrix, number of ports ...
 *
 *    input :   pInitInfo           - pointer to mscc_InitInfo_t struct which contain initialization information data
 *    output:   none
 *    return:   e_POE_STATUS_OK     - operation succeed
 * 			    != e_POE_STATUS_OK  - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_InitSoftware(_IN mscc_InitInfo_t *pInitInfo)
{
	U8 chIndex;
	U8 IcIndex;
	U8 BankIndex;

	U8 Is_Auto_mode_master_capabilities;
	mscc_StandardMode_e pStandardMode_e;  /* At or AF mode */
	U8 Is_support_12_ports;

	U16 SysFlags_RegData;
	U8 IcutMaxFlag_Data;
	U8 PM_CalcMode_Data;

	Masks.Layer2 = POE_TRUE;
	Masks.Layer2PriorityByPD = POE_FALSE;
	Masks.Ignore_priority = POE_FALSE;
	Masks.AC_DisEnable = POE_FALSE;
	Masks.Back_off = POE_FALSE;  /* 0: Alt A (default) ,1: Alt B */
	SystemParams.bPM_PowerDisconnentProcess = POE_TRUE;
	PriorityStep = 0;

	L2_Sync_Flag = POE_FALSE;
	PortsPriorityChanged =  POE_TRUE;
	L2Reduction.ChNum = NONE_CHANNEL;

	bStaticPM_CalcMode = POE_FALSE;

	/* Class power Array initialization */
	ClassPower_mW[0] = 15400;
	ClassPower_mW[1] = 4000;
	ClassPower_mW[2] = 7000;
	ClassPower_mW[3] = 15400;
	ClassPower_mW[4] = 30000;


	/* Init IC structure */
	for(IcIndex=0;IcIndex < MAX_IC_ON_BOARD;IcIndex++)
	{
		IcParams[IcIndex].IC_HW = IC_NONE_CHANNELS;
		IcParams[IcIndex].NumOfChannesInIC = IC_NONE_CHANNELS;
		IcParams[IcIndex].IC_Exp = IC_NONE_CHANNELS;

		IcParams[IcIndex].IC_Address = 0xFF;
		IcParams[IcIndex].IC_Valid = POE_FALSE;
	}

	for(BankIndex=0;BankIndex < MAX_POWER_BUDGET;BankIndex++)
	{
		Budget[BankIndex].PowerSourceType = 0;
	}

	Is_Auto_mode_master_capabilities = 0;
	pStandardMode_e = e_AT_Mode;  /* At or AF mode */
	Is_support_12_ports = 0;

	SystemParams.NumOfChannelsInSystem = 0;

	/* Set IC structure */
	SystemParams.NumOfActiveICsInSystem = mscc_MIN(pInitInfo->NumOfActiveICsInSystem,MAX_IC_ON_BOARD);

	/* Host driver functions */
	mscc_fptr_write = pInitInfo->fptr_write;
	mscc_fptr_read = pInitInfo->fptr_read;

	/* struct pointer for general user purpose */
	mscc_pUserData = pInitInfo->pUserData;

	/* initialize to default matrix */
	for(chIndex=0;chIndex<MAX_CH_PER_SYSTEM ;chIndex++)
	{
		ChannelParams[chIndex].mscc_Vir2PhyArrActiveMatrix =  chIndex;
		ChannelParams[chIndex].mscc_Vir2PhyArrTemporaryMatrix =  chIndex;

		ChannelParams[chIndex].L2Data.CableLen = 0;
	}

	for(IcIndex=0;IcIndex < SystemParams.NumOfActiveICsInSystem;IcIndex++)
	{
		IcParams[IcIndex].IC_Address = pInitInfo->IC_Address[IcIndex];
		IcParams[IcIndex].IC_Exp = pInitInfo->NumOfExpectedChannesInIC[IcIndex];
		IcParams[IcIndex].IC_Valid = POE_TRUE;

		result = mscc_POE_CMD_GetICConfiguration(_IN IcIndex,_OUT &Is_Auto_mode_master_capabilities,_OUT &pStandardMode_e,_OUT &Is_support_12_ports);
		if(result != e_POE_STATUS_OK)
			return result;

		IcParams[IcIndex].IC_SupportData.IC_mode_status = IcParams[IcIndex].IC_SupportData.ModeRequest = IcParams[IcIndex].IC_SupportData.IC_HW_support = pStandardMode_e;
		IcParams[IcIndex].IC_HW = (Is_support_12_ports == POE_TRUE) ? IC_12_CHANNELS : IC_8_CHANNELS;

		mscc_POE_IC_FUNC_Get_IC_STATUS(_IN  IcParams[IcIndex].IC_Exp,_IN  IcParams[IcIndex].IC_HW ,_OUT &IcParams[IcIndex].IC_Status,_OUT &IcParams[IcIndex].NumOfChannesInIC);

		SystemParams.NumOfChannelsInSystem += IcParams[IcIndex].IC_HW;
	}

	/*
	 *PM Mode				Management Mode	Cat Ref	Total Allocated Power	Port Power Limit	Start Condition
	 *> 0	0x02	0x00					C2		Class 					Max.				None
	 */

	/* 0 - Static (according to class or PPL)
	 * 1 - Dynamic (according to real consumption) */

	/* read System Flags register data */
	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysFlags_ADDR, &SysFlags_RegData);
	if (result != e_POE_STATUS_OK)
		return result;

	IcutMaxFlag_Data =(U8) ( (SysFlags_RegData >> e_IcutMaxFlag) & 1);
	PM_CalcMode_Data = (U8)( (SysFlags_RegData >> e_PM_Mode) & 1);

	/* (C2) -  Class based calculation, Port Power is max available */
	if ((IcutMaxFlag_Data == 1) && (PM_CalcMode_Data ==0))
	{
		bStaticPM_CalcMode = POE_TRUE;
	}

	result = mscc_POE_CMD_SetVmainThersholds(_IN MAX_SHUTDOWN_VOLTAGE,_IN MIN_SHUTDOWN_VOLTAGE);
	if(result != e_POE_STATUS_OK)
		return result;

	/* enable interrupt mode */
	return mscc_POE_CMD_SetSingleBitInSysFlags(_IN  e_LSDEn_Or_IntOut, _IN  0);

	/* return e_POE_STATUS_OK; */
}
