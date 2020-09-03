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
 *  Description: Analysis And Execution of the Commands
 *
 *
 *************************************************************************/

/*=========================================================================
 / INCLUDES
 /========================================================================*/

#include "mscc_arch_functions.h"
#include "mscc_poe_comm_protocol.h"
#include "mscc_poe_host_communication.h"
#include "mscc_poe_ic_communication.h"
#include <stdarg.h>
#include <string.h>
#include "mscc_poe_ic_param_def.h" /* include peripheral declarations */
#include "mscc_poe_ic_func.h"
#include "mscc_poe_global_types.h"
#include "mscc_poe_util.h"


/*=========================================================================
 / CONSTANTS
 /========================================================================*/
#define mscc_MAX_ALL_POWER_GROUP_NUM 8
#define mscc_ALLPORTPOWER_RESOLUTION 200

/*=========================================================================
 / TYPES
 /========================================================================*/


typedef struct {
	U8 Length;
	void *pDataPointer;
} LongMessage_t, *PLongMessage_t;


/*=========================================================================
 / GLOBALS
 /========================================================================*/
static U8 mscc_SyncCounter;
static U8 mscc_PrivateLabel=0;


/*=========================================================================
 / LOCAL PROTOTYPES
 /========================================================================*/

S32 mscc_ExecRequestSystemSupply(_IN U8 *pData);
S32 mscc_ExecGlobalRequest(_IN U8 *pData);
S32 mscc_ExecuteCommand(_IN U8 *pData);
S32 mscc_ExecuteRequest(_IN U8 *pData);
S32 mscc_ExecChannelCMD(_IN U8 *pData);
S32 mscc_ExecGlobalCMD(_IN U8 *pData);
S32 mscc_ExecChannelRequest(_IN U8 *pData);
void mscc_Answer(_IN U8 KeyWord,_IN S8 *pMsg,_IN va_list ap);
void mscc_ReportAnswer(_IN S8 *pMsg, ...);
void mscc_TelemetryAnswer(_IN S8 *pMsg, ...);

/*=========================================================================
 / FUNCTIONS
 /========================================================================*/

/*---------------------------------------------------------------------
 *    description:    This function  Analysis And Execution Command  at the Communication
 *
 *    input :   data - PACKAGE
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_AnalysisAndExecutionCommand(_IN U8 pDataIn[])
{
	U8 *ProtocolData = &pDataIn[2];
	mscc_SyncCounter = pDataIn[1];

	if (mscc_POE_UTIL_CommCheckSum(pDataIn) == POE_TRUE)
	{
		switch (pDataIn[0])
		{
			case B_Command:
			{
				result = mscc_ExecuteCommand(ProtocolData);
				if (result == mscc_DO_NOT_SEND_REPORT)
					return e_POE_STATUS_OK; /* The user's report is at the function "SendReport" */

				break;
			}

			case B_Request:
			{
				/* if result>0 than we have PROTOCOL_ERROR and we will report it */
				result = mscc_ExecuteRequest(ProtocolData);
				if (result <= e_POE_STATUS_OK)
					return result; /* The user's report is at the function "SendReport" */

				break;
			}

			default:
			{
				result = mscc_PROTOCOL_ERROR;
				break;
			}
		}

		mscc_SendReport(mscc_MAX(result,0)); /* Send the problem to the user */
	}
	else
	{
		mscc_ReportAnswer("%w%w", mscc_PROTOCOL_ERROR, mscc_PROTOCOL_ERROR);
	}

	if (result > e_POE_STATUS_OK) /* communication protocol error */
		return e_POE_STATUS_ERR_COMMUNICATION_REPORT_ERROR; /* POE_STATUS_ERR_COMMUNICATION_PROTOCOL */
	else /* software internal error */
		return result;
}

/*---------------------------------------------------------------------
 *    description:    This function  Execute Command
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_ExecuteCommand(_IN U8 *pData)
{
	switch (pData[0])
	{
	case B_Channel:
	{
		result = mscc_ExecChannelCMD(&pData[1]);
		break;
	}
	case B_Global:
	{
		result = mscc_ExecGlobalCMD(&pData[1]);
		break;
	}
	default:
		return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(1);
	}
	return result;
}

/*---------------------------------------------------------------------
 *    description:    This function Execute Request
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/

S32 mscc_ExecuteRequest(_IN U8 *pData)
{
	switch (pData[0])
	{
	case B_Channel:
	{
		result = mscc_ExecChannelRequest(&pData[1]);
		break;
	}
	case B_Global:
	{
		result = mscc_ExecGlobalRequest(&pData[1]);
		break;
	}
	default:
		return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(2);
	}
	return result;
}

/*---------------------------------------------------------------------
 *    description:    This function Execute Global command
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_ExecGlobalCMD(_IN U8 *pData)
{
	U8 Sub1 = pData[0];
	U8 Sub2 = pData[1];
	U8 Data0 = pData[2];
	U8 Data1 = pData[3];
	U8 Data2 = pData[4];

	switch (Sub1)
	{
	case B_DeviceParams: /* Set PoE Device Params */
	{
		U8 MipReq;

		/* Check that the CS is valid */
		if (Sub2 >= MAX_IC_ON_BOARD)
			return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(1);

		/* Check if number of Ports is a valid num */
		if ((Data0 != IC_12_CHANNELS) && (Data0 != IC_8_CHANNELS) && (Data0	!= IC_NONE_CHANNELS))
			return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(2);

		/* Check if TSH data is in range */
		if (Data1 > MaxTemperatureLimit)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(3));

		/* Check if MiPAT-Req is is range (0 or 1) */
		if (Data2 > 1)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(4));

		/* Update the IC expected number of Ports field */
		IcParams[Sub2].IC_Exp = Data0;

		/* Set IC Temperature */
		result = mscc_POE_CMD_SetTemperatureAlarmThershold(Sub2, Data1);
		if (result != e_POE_STATUS_OK)
			return result;

		/* get standard/mode parameter */
		MipReq = Data2 & 0x01;
		IcParams[Sub2].IC_SupportData.ModeRequest = MipReq;
		IcParams[Sub2].IC_SupportData.IC_mode_status = MipReq * IcParams[Sub2].IC_SupportData.IC_HW_support;

		result = mscc_POE_CMD_Set_PORTS_CFG_to_a_single_IC(
		  Sub2 /* _IN U8 IcIndex 		    */
		, POE_FALSE /* _IN U8 bPSE_Enable 		*/
		, POE_FALSE /* _IN U8 bPairControl 		*/
		, POE_TRUE /* _IN U8 bPortMode 		*/
		, POE_FALSE /* _IN U8 bPortPriority 	*/
		, 0 /* _IN U8 PSE_EnableValue 	*/
		, 0 /* _IN U8 PairControlValue 	*/
		, IcParams[Sub2].IC_SupportData.IC_mode_status /* _IN U8 PortModeValue 	*/
		, 0); /* _IN U8 PortPriorityValue */

		break;
	}

	case B_IrqMask: /* Set Interrupt Mask */
	{
		U16 InterruptMask_Data;
		InterruptMask_Data = mscc_GetExtractData(pData[1], pData[2]);
		result = mscc_POE_CMD_SetInterruptMask(InterruptMask_Data);
		break;
	}

	case B_Individual_Mask: /* Set Individual Mask */
	{
		if (Data0 > 1)
			return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(5);

		switch (Sub2)
		{
		case e_MaskKey_AC_DisEnable: /*  0x09 */
		{
			Masks.AC_DisEnable = Data0;
			result = mscc_POE_CMD_SetSingleBitInSysFlags(_IN e_DC_DiscoEn,_IN (Data0 == 0)?1:0);
			break;
		}
		case e_MaskKey_Back_off: /* 17 */
		{
			/*
			 protocol:
			 En/Dis '0' - Alternative A.
			 En/Dis '1' - Alternative B

			 registers:
			 00 - Reserved
			 01 - Alternative A
			 10 - Alternative B (Backoff Enable)
			 11 - Reserved
			 */

			U8 alternative = 1; /* Alternative A */
			if(Data0 == 1)
				alternative = 2; /* Alternative B */

			Masks.Back_off = Data0;

			result = mscc_POE_CMD_Set_All_PORTS_CFG(
			POE_FALSE /* _IN U8 bPSE_Enable 		*/
			,POE_TRUE /* _IN U8 bPairControl 		*/
			,POE_FALSE /* _IN U8 bPortMode 		*/
			,POE_FALSE /* _IN U8 bPortPriority 	*/
			,0 /* _IN U8 PSE_EnableValue 	*/
			,alternative/* _IN U8 PairControlValue 	*/
			,0 /* _IN U8 PortModeValue 	*/
			,0); /* _IN U8 PortPriorityValue */

			break;
		}
		case e_MaskKey_Ignore_priority:
		{
			U16 StartupCR_ADDR_RegData;

			result = mscc_IC_COMM_ReadDataFromSpecificIC(_IN SystemParams.IC_Master_Index,StartupCR_ADDR,&StartupCR_ADDR_RegData);
			if(result != e_POE_STATUS_OK)
				return result;

			if(Data0) /* check if startup power is enough for lower priority ports */
				StartupCR_ADDR_RegData |= (1 << 6);
			else /* don't try to start the following ports */
				StartupCR_ADDR_RegData &= ~(1 << 6);

			result = mscc_SendPreSysConfigCommands();
			if(result == e_POE_STATUS_OK)
				result = mscc_IC_COMM_WriteDataToAllICs(StartupCR_ADDR,StartupCR_ADDR_RegData);

			tmpResult = mscc_SendPostSysConfigCommands();
			UPDATE_RESULT(result,tmpResult);

			Masks.Ignore_priority = Data0;

		    break;
		}
		case e_MaskKey_Layer2: /*  0x2E */
		{
		    Masks.Layer2 = Data0;
		    break;
		}
		case e_MaskKey_Layer2PriorityByPD: /*  0x2F */
		{
		    Masks.Layer2PriorityByPD =  Data0;
		    break;
		}
		default:
		{
		    return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(3);
		}
	}

	break;
}

case B_Maskz: /* Set System Masks */
{
	U8 CapEn;
	U16 SysFlags_RegData;

	if ((Sub2> 7) || (Sub2 <4))
	return
		FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(6);

	SystemParams.bPM_PowerDisconnentProcess = (Sub2 & 0x1);
	CapEn = ((Sub2 >>1) & 0x1); /* cap disabled */

	/* read System Flags register data */
	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index ,SysFlags_ADDR,&SysFlags_RegData);
	if(result != e_POE_STATUS_OK) return result;

	/* Register:
	 0 - Cap enabled
	 1 - Cap disabled

	 Protocol:
	 0 = RES mode.
	 1 (default) = RES+CAP
	 */

	if(CapEn) /* cap enabled - set 0 to CapDis bit */
		SysFlags_RegData &= ~(1 << e_CapDis);
	else /* cap disabled - set 1 to CapDis bit */
		SysFlags_RegData |= (1 << e_CapDis);

	/* send registers SysFlags and StartupCR modified data */
	result = mscc_POE_CMD_SetSysFlags(SysFlags_RegData);

	break;
}
case B_Supply:
{
	switch (Sub2)
	{
		case B_PowerManageMode: /* Set PM Method  */
		{
			U8 IcutMaxFlag_Data = 0;
			U8 PM_CalcMode_Data = 0;
			U16 SysFlags_RegData;

			if ((Data0> 4) || (Data1> 2) || (Data2> 4))
				return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(7);

			/* read System Flags register data */
			result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index ,SysFlags_ADDR,&SysFlags_RegData);
			if(result != e_POE_STATUS_OK)
				return result;


			if((Data0 == 0x00)&&(Data1 ==0x02)&&(Data2==0x00)) /* D */
			{
				IcutMaxFlag_Data = 1;
				PM_CalcMode_Data = 1;
				bStaticPM_CalcMode = POE_FALSE;
			}
			else if((Data0 == 0x00)&&(Data1 ==0x00)&& ((Data2==0x00)||(Data2==0x01))) /*S1 or S2 */
			{
				IcutMaxFlag_Data = 0;
				PM_CalcMode_Data = 1;
				bStaticPM_CalcMode = POE_FALSE;
			}
			else if((Data0> 0x00)&&(Data1 ==0x01)&&(Data2==0x00)) /* C1 */
			{
				IcutMaxFlag_Data = 0;
				PM_CalcMode_Data = 0;
				bStaticPM_CalcMode = POE_FALSE;
			}
			else if((Data0> 0x00)&&(Data1 ==0x02)&&(Data2==0x00)) /* C2 */
			{
				IcutMaxFlag_Data = 1;
				PM_CalcMode_Data = 0;
				bStaticPM_CalcMode = POE_TRUE;
			}
			else
			{
				bStaticPM_CalcMode = POE_FALSE;
				return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(8);
			}

			/* reset IcutMaxFlag_Data bit */
			SysFlags_RegData &= ~(1 << e_IcutMaxFlag);
			/* set IcutMaxFlag_Data bit */
			SysFlags_RegData |= (IcutMaxFlag_Data << e_IcutMaxFlag);

			/* reset PM_CalcMode_Data bit */
			SysFlags_RegData &= ~(1 << e_PM_Mode);
			/* set PM_CalcMode_Data bit */
			SysFlags_RegData |= (PM_CalcMode_Data << e_PM_Mode);

			/* send registers SysFlags modified data */
			result = mscc_POE_CMD_SetSysFlags(SysFlags_RegData);

			break;
		}
		case B_PowerBudget: /* Set Power Banks */
		{
			U8 Bank = pData[2];
			U16 PowerLimit = mscc_GetExtractData(pData[3], pData[4]);
			U16 MaxShutdownVoltage_deciVolts = mscc_GetExtractData(pData[5], pData[6]);
			U16 MinShutdownVoltage_deciVolts = mscc_GetExtractData(pData[7], pData[8]);

			/* budget numbers are 0 - 8 */
			if(Bank >= MAX_BANK_IN_IC)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(9));

			/* check power budget limit ranges */
			if((PowerLimit> MAX_BUDGET_LIMIT) || (PowerLimit < MIN_BUDGET_LIMIT))
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(10));

			if((MinShutdownVoltage_deciVolts != mscc_DONT_CHANGE_VOLT) || (MaxShutdownVoltage_deciVolts != mscc_DONT_CHANGE_VOLT))
			{
				/* if the cmd is not to change the min value */
				if(MinShutdownVoltage_deciVolts == mscc_DONT_CHANGE_VOLT)
				{
					/* use old value */
					result = mscc_POE_CMD_GetVmainLowThershold(SystemParams.IC_Master_Index,&MinShutdownVoltage_deciVolts);
					if(result != e_POE_STATUS_OK) return result;
				}

				/* if cnd is not to change max value */
				if(MaxShutdownVoltage_deciVolts == mscc_DONT_CHANGE_VOLT)
				{
					/* use old value */
					result = mscc_POE_CMD_GetVmainHighThershold(SystemParams.IC_Master_Index,&MaxShutdownVoltage_deciVolts);
					if(result != e_POE_STATUS_OK) return result;
				}

				/* min value can't be more than max value */
				if (MaxShutdownVoltage_deciVolts < (MinShutdownVoltage_deciVolts + 30))
				return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(11));

				/* check that new value is in range */
				if (MinShutdownVoltage_deciVolts < MIN_SHUTDOWN_VOLTAGE)
				return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(12));

				/* check that value is in legal range */
				if (MaxShutdownVoltage_deciVolts> MAX_SHUTDOWN_VOLTAGE)
				return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(13));

				result = mscc_POE_CMD_SetVmainThersholds(_IN MaxShutdownVoltage_deciVolts,_IN MinShutdownVoltage_deciVolts);
				if(result != e_POE_STATUS_OK) return result;
			}

			PowerLimit *= 10; /* convert from watts to deciwatts */
			/* SysPowerBudget */
			result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(Bank*2) ,PowerLimit);
			if(result != e_POE_STATUS_OK) return result;

			/**** sync commands ****/

			/* I2C_ExtSyncType */
			result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,I2C_ExtSyncType_ADDR ,4);
			if(result != e_POE_STATUS_OK) return result;

			/* EXT_EV_IRQ */
			result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,EXT_EV_IRQ_ADDR ,4);

			break;
		}
		case B_PowerBudgetSourceType:  /* Set Power Bank Power Source Type */
		{
			U8 BankNumber = pData[2];
			U8 SourceType = pData[3];

			/* budget numbers are 0 - 8 */
			if(BankNumber >= MAX_BANK_IN_IC)
			     return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(14));

			/* check power budget limit ranges */
			if(SourceType >= e_power_source_Reserved)
			    return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(15));

			/* save the power source type of a specific bank number */
			Budget[BankNumber].PowerSourceType = SourceType;

			break;
		}

		default:
			return 2;
	}

	break;
}

case B_SystemStatus: /* Set System Status */
{
	result = mscc_IC_COMM_WriteDataToAllICs(CFGC_USRREG_ADDR,Sub2);
	if(result == e_POE_STATUS_OK)
		mscc_PrivateLabel = pData[1];

	break;
}

case B_TmpMatrix: /* Program Global Matrix */
{
	if(mscc_POE_UTIL_CheckAppearanceTwiceMatrix() == POE_TRUE)
	{
		U8 i;
		for (i=0; i<SystemParams.NumOfChannelsInSystem; i++) /* copy Temporary matrix to Active matrix */
			ChannelParams[i].mscc_Vir2PhyArrActiveMatrix = ChannelParams[i].mscc_Vir2PhyArrTemporaryMatrix;

		/*return DO_NOT_SEND_REPORT;*/
	}
	else
	{
		return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(16);
	}

	break;
}

default:
return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(4);
}

return result;
}

/*---------------------------------------------------------------------
 *    description:    This function Execute Channel command
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_ExecChannelCMD(_IN U8 *pData)
{
	U8 Sub = pData[0];
	U8 chLogNum = pData[1];
	U8 data0 = pData[2];
	U8 data1 = pData[3];
	U8 data2 = pData[4];
	U8 data3 = pData[5];
	U8 data5 = pData[7];
	U8 data6 = pData[8];
	U8 chPhyNum;

	/* check if Channel number is out of range */
	if ((chLogNum != B_All_channels) && (chLogNum >= MAX_CH_PER_SYSTEM))
		return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(17));

	/* convert logical port number to physical */
	if(chLogNum != B_All_channels)
		chPhyNum = ChannelParams[chLogNum].mscc_Vir2PhyArrActiveMatrix;
	else
		chPhyNum = chLogNum;

	switch (Sub)
	{
	case B_PortFullInit: /* Set Port Parameters */
	{
		U16 PPL_mW;

		/* check that enable disable cmd data is 0 or 1 */
		if (data0 > 1)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(18));

		PPL_mW = mscc_GetExtractData(data1, data2);

		if (PPL_mW > MAX_POWER_PER_PORT)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(19));

		/* check that priority cmd data range is 1 to 3 */
		if ((data3 < 1) || (data3 > 3))
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(20));

		PortsPriorityChanged = POE_TRUE;
		data3--; /* convert priority from 15bytes protocol to regisers representation */

		if (chPhyNum == B_All_channels)
		{
			result = mscc_POE_CMD_Set_All_PORTS_CFG(
			POE_TRUE, /* _IN U8 bPSE_Enable 		 */
			POE_FALSE, /* _IN U8 bPairControl 	 */
			POE_FALSE, /* _IN U8 bPortMode 		 */
			POE_TRUE, /* _IN U8 bPortPriority 	 */
			data0, /* _IN U8 PSE_EnableValue 	 */
			0, /* _IN U8 PairControlValue  */
			0, /* _IN U8 PortModeValue 	 */
			data3); /* _IN U8 PortPriorityValue */

			if (result != e_POE_STATUS_OK)
				return result;

			/* Set PPL */
			result = mscc_POE_CMD_SetAllChannelPPL(_IN PPL_mW);
			if (result != e_POE_STATUS_OK)
				return result;
		}
		else /* single channel */
		{
			result = mscc_POE_CMD_Set_Single_PORT_CFG(chPhyNum, /*  _IN U8 logicalPortNumber  */
			POE_TRUE, /*  _IN U8 bPSE_Enable        */
			POE_FALSE, /*  _IN U8 bPairControl       */
			POE_FALSE, /*  _IN U8 bPortMode			 */
			POE_TRUE, /*  _IN U8 bPortPriority      */
			data0, /*  _IN U8 PSE_EnableValue    */
			0, /*  _IN U8 PairControlValue   */
			0, /*  _IN U8 PortModeValue      */
			data3); /*  _IN U8 PortPriorityValue	 */

			if (result != e_POE_STATUS_OK)
				return result;

			/* Set PPL */
			result = mscc_POE_CMD_SetChannelPPL(chPhyNum, PPL_mW);
			if (result != e_POE_STATUS_OK)
				return result;
		}

		break;
	}

	case B_EnDis: /* Set Enable/Disable channels */
	{
		/*
		 Param1 : Cmd: 0 - Disable; 1 (default) - Enable.
		 Param2 : AF Mask (PD69000 only): 0 - only IEEE802.3af operation; N - stay with the last mode (IEEE802.3af or IEEE802.3at).
		 */

		U8 bAF_Mask;

		if (data0 > 1) /* Cmd */
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(21));

		if ((data1 > 1) && (data1 != B_Space)) /* AF Mask */
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(22));

		/* skip AF Mask parameter if  AF Mask data is 78 */
		bAF_Mask= POE_TRUE;

		if (data1 == B_Space) {
			data1 = 0;
			bAF_Mask = POE_FALSE;
		}

		if (chPhyNum == B_All_channels) {
			result = mscc_POE_CMD_Set_All_PORTS_CFG(
			POE_TRUE, /* _IN U8 bPSE_Enable 		 */
			POE_FALSE, /* _IN U8 bPairControl 	 */
			bAF_Mask, /* _IN U8 bPortMode 		 */
			POE_FALSE, /* _IN U8 bPortPriority 	 */
			data0, /* _IN U8 PSE_EnableValue 	 */
			0, /* _IN U8 PairControlValue  */
			data1, /* _IN U8 PortModeValue 	 */
			0); /* _IN U8 PortPriorityValue */
		}
		else /* single channel */
		{
			result = mscc_POE_CMD_Set_Single_PORT_CFG(chPhyNum, /* _IN U8 logicalPortNumber */
			POE_TRUE, /* _IN U8 bPSE_Enable 		*/
			POE_FALSE, /* _IN U8 bPairControl 		*/
			bAF_Mask, /* _IN U8 bPortMode 		*/
			POE_FALSE, /* _IN U8 bPortPriority 	*/
			data0, /* _IN U8 PSE_EnableValue 	*/
			0, /* _IN U8 PairControlValue 	*/
			data1, /* _IN U8 PortModeValue 	*/
			0); /* _IN U8 PortPriorityValue */
		}

		if (result != e_POE_STATUS_OK)
			return result;

		break;
	}

	case B_TemporarySupply: /* Set Temporary Power Limit for Channels */
	{
		U16 TPPL_mW;

		if (chPhyNum == B_All_channels) /* Layer2 can't work with all channels */
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(23));

		TPPL_mW = mscc_GetExtractData(data0, data1);

		if (TPPL_mW > MAX_POWER_PER_PORT)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(24));

		result = mscc_POE_CMD_SetChannelTPPL(chPhyNum, TPPL_mW);
		if (result != e_POE_STATUS_OK)
			return result;

		break;
	}

	case B_Supply: /* Set Power Limit for Channels */
	{
		U16 PPL_mW = mscc_GetExtractData(data0, data1);

		if (PPL_mW > MAX_POWER_PER_PORT)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(25));

		if (chPhyNum == B_All_channels)
			result = mscc_POE_CMD_SetAllChannelPPL(_IN PPL_mW);
		else /* single channel */
			result = mscc_POE_CMD_SetChannelPPL(chPhyNum, PPL_mW);

		if (result != e_POE_STATUS_OK)
			return result;

		break;
	}

	case B_Priority: /* Set Port Priority */
	{
		if ((data0 < 1) || (data0 > 3))
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(26));

		PortsPriorityChanged = POE_TRUE;
		data0--; /* convert from protocol priority representation to priority register representation */

		if (chPhyNum == B_All_channels)
		{
			result = mscc_POE_CMD_Set_All_PORTS_CFG(
			POE_FALSE, /* _IN U8 bPSE_Enable */
			POE_FALSE, /* _IN U8 bPairControl */
			POE_FALSE, /* _IN U8 bPortMode */
			POE_TRUE, /* _IN U8 bPortPriority */
			0, /* _IN U8 PSE_EnableValue */
			0, /* _IN U8 PairControlValue */
			0, /* _IN U8 PortModeValue */
			data0 /* _IN U8 PortPriorityValue */
			);
		}
		else  /* single channel */
		{
			result = mscc_POE_CMD_Set_Single_PORT_CFG(
					chPhyNum,
					POE_FALSE,  /* _IN U8 bPSE_Enable */
					POE_FALSE,  /* _IN U8 bPairControl */
					POE_FALSE,  /* _IN U8 bPortMode */
					POE_TRUE,   /* _IN U8 bPortPriority */
					0,          /* _IN U8 PSE_EnableValue */
					0,          /* _IN U8 PairControlValue */
					0,          /* _IN U8 PortModeValue */
					data0       /* _IN U8 PortPriorityValue */
					);
		}

		if (result != e_POE_STATUS_OK)
			return result;

		break;
	}

	case B_ForcePower: /* Set Force Power */
	{
		U8 ForcePower;

		/* check that enable disable cmd data is 0 or 1 */
		if (data0 > 1)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(27));

		if (data0 == 0)
			ForcePower = 1;
		else
			ForcePower = 2;

		if (chPhyNum == B_All_channels)
		{
			/* disable all ports */
			result = mscc_POE_CMD_Set_All_PORTS_CFG(
			POE_TRUE, /* _IN U8 bPSE_Enable */
			POE_FALSE, /* _IN U8 bPairControl */
			POE_FALSE, /* _IN U8 bPortMode */
			POE_FALSE, /* _IN U8 bPortPriority */
			0, /* _IN U8 PSE_EnableValue */
			0, /* _IN U8 PairControlValue */
			0, /* _IN U8 PortModeValue */
			0 /* _IN U8 PortPriorityValue */
			);

			if (result != e_POE_STATUS_OK)
				return result;

			result = OS_Sleep_mS(50);
			if (result != e_POE_STATUS_OK)
				return result;

			result = mscc_POE_CMD_Set_All_PORTS_CFG(
			POE_TRUE, /* _IN U8 bPSE_Enable */
			POE_FALSE, /* _IN U8 bPairControl */
			POE_FALSE, /* _IN U8 bPortMode */
			POE_FALSE, /* _IN U8 bPortPriority */
			ForcePower, /* _IN U8 PSE_EnableValue */
			0, /* _IN U8 PairControlValue */
			0, /* _IN U8 PortModeValue */
			0 /* _IN U8 PortPriorityValue */
			);

			if (result != e_POE_STATUS_OK)
				return result;
		}
		else /* single channel */
		{
			/* disable single port */
			result = mscc_POE_CMD_Set_Single_PORT_CFG(
					chPhyNum,
					POE_TRUE,   /* _IN U8 bPSE_Enable */
					POE_FALSE,  /* _IN U8 bPairControl */
					POE_FALSE,  /* _IN U8 bPortMode */
					POE_FALSE,  /* _IN U8 bPortPriority */
					0,          /* _IN U8 PSE_EnableValue */
					0,          /* _IN U8 PairControlValue */
					0,          /* _IN U8 PortModeValue */
					0);         /* _IN U8 bPortPriority */

			if (result != e_POE_STATUS_OK)
				return result;

			result = OS_Sleep_mS(50);
			if (result != e_POE_STATUS_OK)
				return result;

			result = mscc_POE_CMD_Set_Single_PORT_CFG(
					chPhyNum,
					POE_TRUE,    /* _IN U8 bPSE_Enable */
					POE_FALSE,   /* _IN U8 bPairControl */
					POE_FALSE,   /* _IN U8 bPortMode */
					POE_FALSE,   /* _IN U8 bPortPriority */
					ForcePower,	 /* _IN U8 bPSE_Enable */
					0,           /* _IN U8 bPairControl */
					0,           /* _IN U8 bPortMode */
					0);          /* _IN U8 bPortPriority */

			if (result != e_POE_STATUS_OK)
				return result;
		}

		break;
	}


	case B_Layer2_PD:  /* Set Port Layer2 PSE Data  */
	{
		U8 Type=data0;
		U16 PD_Request_Power = mscc_GetExtractData(_IN  data1, _IN  data2);
		U8 Cable_Length = data5;
		U8 Execute_LLDP = data6;
		U8 Priority;

		/* If layer2 feature is disabled or not working static power - max - return out of range */
		if( (!Masks.Layer2) || (!bStaticPM_CalcMode))
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(28));

		/* if power request is greater than the limit, return out of range */
		if( PD_Request_Power > MAX_POWER_PER_PORT )
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(29));

		/* set the new priority only if the priority bit is enabled and the provided priorty is not zero */
		if(Masks.Layer2PriorityByPD)
		{
			/* calculate the 2 bits of the channel priority */
			Priority = (U16)(Type & 3);

		 	if(Priority != UNKNOWN_PRIORITY)
			{
				/* set the new priority */
		 		Priority--;  /* convert to IC priority */

		 		PortsPriorityChanged = POE_TRUE; /* sign that priority has been changed */

				result = mscc_POE_CMD_Set_Single_PORT_CFG_without_disable_port(chPhyNum, /*  _IN U8 logicalPortNumber  */
							POE_FALSE,  /*  _IN U8 bPSE_Enable        */
							POE_FALSE,  /*  _IN U8 bPairControl       */
							POE_FALSE,  /*  _IN U8 bPortMode			 */
							POE_TRUE,   /*  _IN U8 bPortPriority      */
							0,          /*  _IN U8 PSE_EnableValue    */
							0,          /*  _IN U8 PairControlValue   */
							0,          /*  _IN U8 PortModeValue      */
							Priority);  /*  _IN U8 PortPriorityValue	 */

				if (result != e_POE_STATUS_OK)
					return result;
			}
		}

		/* if the cable length is bigger than the maximum possible, set it to maximum */
		if(Cable_Length > L2_MAX_CABLE_LEN)
			Cable_Length = L2_MAX_CABLE_LEN;

		/* saving cable length */
		ChannelParams[chPhyNum].L2Data.CableLen = Cable_Length;
		ChannelParams[chPhyNum].L2Data.PDReqPower_dW = PD_Request_Power;
		/* is a change between the request and the actual allocation, and this channel is not in the reduction struct, mark this flag to deal with this channel in the next run of ConvertPDReq() */
		ChannelParams[chPhyNum].L2Data.CalcReq = POE_TRUE;
		/* update L2 counter for max value (this channel is signed to have active L2) */
		ChannelParams[chPhyNum].L2Data.PDCount = L2_COUNTER_INIT;
		/* save the execution flag in our global variable */
		if(Execute_LLDP & 1)
			L2_Sync_Flag = POE_TRUE;

		break;
	}


	case B_TmpMatrix: /* Set Temporary Matrix */
	{
		if (chLogNum == B_All_channels)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(30));

		ChannelParams[chLogNum].mscc_Vir2PhyArrTemporaryMatrix = data0;
		/*return DO_NOT_SEND_REPORT;*/
		break;
	}


	default:
		return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(5);
	}

	return result;
}

/*---------------------------------------------------------------------
 *    description:    This function Send Report
 *
 *    input :   Status - data
 *
 *    output:   none
 *
 *    return:   none
 *---------------------------------------------------------------------*/
void mscc_SendReport(_IN U16 Status)
{
	mscc_ReportAnswer("%w", Status);
}

void mscc_ReportAnswer(_IN S8 *pMsg, ...)
{
	va_list ap;
	va_start(ap, pMsg );
	mscc_Answer(B_Report, pMsg, ap);
	va_end(ap);
}

void mscc_Answer(_IN U8 KeyWord,_IN S8 *pMsg,_IN  va_list ap)
{
    #define  _PACKAGE_SIZE	PACKAGE_SIZE + 9
	U8 RawMsg[_PACKAGE_SIZE];
	U8 i, y = 0;

	memset(RawMsg, B_Space, _PACKAGE_SIZE);
	RawMsg[y] = KeyWord; /* Set message KEY */
	y++;
	RawMsg[y] = mscc_SyncCounter; /* set message ECHO */
	y++;

	for (i = 0; (pMsg[i] != 0) && (i < _PACKAGE_SIZE); i++) {
		if (pMsg[i] != '%') {
			RawMsg[y] = pMsg[i];
			y++;
			continue;
		}

		switch (pMsg[i + 1]) {
		case 'c': {
			RawMsg[y] = va_arg(ap, int);
			y++;
			break;
		}
		case 'S': {
			PLongMessage_t plmval;

			plmval = va_arg(ap, LongMessage_t *);
			if ( (plmval->pDataPointer != NULL) && (plmval->Length != 0 )) {
				memcpy(&(RawMsg[y]), plmval->pDataPointer, plmval->Length);
				y += plmval->Length;
			}
			break;
		}
		case 'w': /* word - 2 bytes */
		{
			U16 arg= va_arg(ap, int);
			RawMsg[y++] = mscc_GetFirstByte(arg);
			RawMsg[y++] = mscc_GetSecondByte(arg);

			break;
		}
		case 'd': /* double -  4 bytes */
		{
			U32 arg= va_arg(ap, U32);
			RawMsg[y++] = (U8)(arg >> 24);
			RawMsg[y++] = (U8)(arg >> 16);
			RawMsg[y++] = (U8)(arg >> 8);
			RawMsg[y++] = (U8)arg;

			i++; /* for the %dw */
			break;
		}
		default: {
			RawMsg[y] = pMsg[i];
			break;
		}
		}

		i++;
	}

	mscc_POE_UTIL_CommMakeCheckSum(RawMsg); /* Calculate and Set Checksum in message */
	mscc_POE_HOST_WriteMsgToTxBuffer(RawMsg);
}

/*---------------------------------------------------------------------
 *    description:    This function Execute Channel command
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_ExecChannelRequest(_IN U8 *pData)
{
	U8 chLogNum = pData[1];
	U8 chPhyNum;

	/* Check that the requested Channel is in the range */
	if (chLogNum >= MAX_CH_PER_SYSTEM)
		return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(31));

	/* convert logical port number to physical */
	if(chLogNum != B_All_channels)
		chPhyNum = ChannelParams[chLogNum].mscc_Vir2PhyArrActiveMatrix;
	else
		chPhyNum = chLogNum;

	switch (pData[0])
	{
	case B_PortStatus: /* Get Single Port Status */
	{
		U8 PortStatusData = 0;
		U8 portEnDis = 0;
		U8 LatchData;
		U8 Port_Class_Data;
		U8 AtType = 0;
		U8 bIsPortOn;

		result = mscc_POE_CMD_GetChannelStatus(chPhyNum, &PortStatusData,&bIsPortOn, &AtType);
		if (result != e_POE_STATUS_OK)
			goto SendSinglePortStatusTelemetry;

		if (PortStatusData != e_CustomerChannelOFF)
			portEnDis = 1;

		result = mscc_POE_CMD_GetChannelLatch(chPhyNum, &LatchData); /* Build the Latch */
		if (result != e_POE_STATUS_OK)
			goto SendSinglePortStatusTelemetry;

		result = mscc_POE_CMD_GetChannelClass(chPhyNum, &Port_Class_Data);

	SendSinglePortStatusTelemetry:
		mscc_TelemetryAnswer("%c%c%c%c%c%c%c%c%c", portEnDis, PortStatusData, 0, LatchData, Port_Class_Data,
				B_Space, B_Space, B_Space, AtType);
		break;
	}

	case B_Supply: /* Get Port Power Limit  */
	{
		U16 PortPowerLimit_Data_mW = 0;
		U16 TemporaryPortPowerLimit_Data_mW = 0;

		result = mscc_POE_CMD_GetChannelPPL(chPhyNum, &PortPowerLimit_Data_mW);
		if (result != e_POE_STATUS_OK)
			goto SendPortPowerLimitTelemetry;

		result = mscc_POE_CMD_GetChannelTPPL(chPhyNum,	&TemporaryPortPowerLimit_Data_mW);

	SendPortPowerLimitTelemetry:
		mscc_TelemetryAnswer("%w%w", PortPowerLimit_Data_mW, TemporaryPortPowerLimit_Data_mW);
		break;
	}

	case B_Paramz: /* Get Port Measurements  */
	{
		PowerInfo_t PowerInfo= { 0, 0, 0, 0 };
		result = mscc_POE_CMD_GetChannelPowerInfo(chPhyNum, &PowerInfo);
		mscc_TelemetryAnswer("%w%w%w%c%w", PowerInfo.MainVolt, PowerInfo.chCurrent,
				PowerInfo.chPower, (U8)B_Space, PowerInfo.chVolt);
		break;
	}

	case B_Priority: /* Get Port Priority  */
	{
		U8 PortPriority = 0;
		PORT_CFG_t PORT_CFG;

		result = mscc_POE_CMD_Get_Single_PORT_CFG(chPhyNum, &PORT_CFG);
		if (result != e_POE_STATUS_OK)
			goto SendPortPriorityTelemetry;

		PortPriority = PORT_CFG.Bits.PortPriority+1; /* convert from register priority to 15bytes protocol priority */

		SendPortPriorityTelemetry: mscc_TelemetryAnswer("%c", PortPriority);
		break;
	}

	case B_ChannelMatrix: /* Get Physical Port Number from Active Matrix  */
	{
		mscc_TelemetryAnswer("%c", ChannelParams[chLogNum].mscc_Vir2PhyArrActiveMatrix);
		break;
	}


	case B_Layer2_PSE: /* Get Port Layer2 PSE Data  */
	{
		#define CH_ON_POE_PSE 7
		#define CH_OFF_POE_PSE 3

		U16 PSE_Allocated_Power = 0;
		U16 PDReqPower_dW = 0;
		U16	delta = 0;
		U8	MDI_Power_Status = CH_OFF_POE_PSE; /* port is off */
		U8 PSE_Power_Pair = e_Alternative_A; /* Alt A (default) */

		U8 Port_Class_Data = 0;   /* power class */
		U8 PortPriority = 0;
		mscc_Type_t Type;
		PORT_CFG_t PORT_CFG;
		U8 ActivePowerBank;

		U8 PortStatusData = 0;
		U8 AtType = 0;
		U8 bIsPortOn=0;

		U16 TemporaryPortPowerLimit_Data_mW = 0;

		result = mscc_POE_CMD_GetChannelStatus(chPhyNum, &PortStatusData,&bIsPortOn, &AtType);
		if (result != e_POE_STATUS_OK)
				goto Send_Layer2_PSE_Telemetry;

		/* Check the status of the channel
		 * If the channel is not working, so return req = PSE_Alloc <-- 0 */
		if(!bIsPortOn)
		{
			PSE_Allocated_Power = 0;
			PDReqPower_dW = 0;
		}
		else
		{
			/* if the requested channel is in the reduction struct, take the new power value from there! */
			if(L2Reduction.ChNum == chPhyNum)
			{
				/* get the new PSE allocated power from the reduction struct */
				PSE_Allocated_Power = L2Reduction.Allocated;
				/* clear the L2 reduction struct for next use */
				L2Reduction.ChNum = NONE_CHANNEL;

				PDReqPower_dW = ChannelParams[chPhyNum].L2Data.PDReqPower_dW;
			}
			else
			{
				/* if channel is no longer LLDP, we must init it's request power and return 0 */
				if(!ChannelParams[chPhyNum].L2Data.PDCount)
				{
					result = mscc_POE_CMD_GetChannelTPPL(chPhyNum,	&TemporaryPortPowerLimit_Data_mW);
					if (result != e_POE_STATUS_OK)
							goto Send_Layer2_PSE_Telemetry;

					PSE_Allocated_Power = mscc_POE_CMD_ConvertPSEAllocation((TemporaryPortPowerLimit_Data_mW / 100), chPhyNum);
					/* calculating the absolute value of 2 numbers */
					delta = mscc_POE_UTIL_CalcAbsValue(PSE_Allocated_Power, ChannelParams[chPhyNum].L2Data.PDReqPower_dW);
					/* if delta < 1W (5 in deciWatt), return the original value the PD request - to avoid pingpong between PSE and PD */
					if(delta < 10)
						PSE_Allocated_Power = ChannelParams[chPhyNum].L2Data.PDReqPower_dW;

					PDReqPower_dW = 0;
				}
				else
				{
					result = mscc_POE_CMD_GetChannelTPPL(chPhyNum,	&TemporaryPortPowerLimit_Data_mW);
					if (result != e_POE_STATUS_OK)
						goto Send_Layer2_PSE_Telemetry;

					PSE_Allocated_Power = mscc_POE_CMD_ConvertPSEAllocation((TemporaryPortPowerLimit_Data_mW / 100), chPhyNum);
					/* calculating the absolute value of 2 numbers */
					delta = mscc_POE_UTIL_CalcAbsValue(PSE_Allocated_Power, ChannelParams[chPhyNum].L2Data.PDReqPower_dW);
					/* if delta < 1W (5 in deciWatt), return the original value the PD request - to avoid pingpong between PSE and PD */
					if(delta < 10)
						PSE_Allocated_Power = ChannelParams[chPhyNum].L2Data.PDReqPower_dW;

					PDReqPower_dW = ChannelParams[chPhyNum].L2Data.PDReqPower_dW;
				}
			}
		}

		result = mscc_POE_CMD_Get_Single_PORT_CFG(chPhyNum, &PORT_CFG);
		if (result != e_POE_STATUS_OK)
			goto Send_Layer2_PSE_Telemetry;

		PortPriority = PORT_CFG.Bits.PortPriority + 1; /* convert from register priority to 15bytes protocol priority */

		/* get active Power Budget Index */
		result = mscc_POE_CMD_GetActiveBank(_IN  SystemParams.IC_Master_Index,_OUT &ActivePowerBank);
		if(result != e_POE_STATUS_OK)
			return result;

		Type.Bits.power_priority = PortPriority;   /* update the channel's priority */
		Type.Bits.Reserved =  0;
		Type.Bits.power_source =  Budget[ActivePowerBank].PowerSourceType;
		Type.Bits.power_type =  (AtType) ? e_Power_Type_2_PSE : e_Power_Type_1_PSE;

		/* get power class */
		result = mscc_POE_CMD_GetChannelClass(chPhyNum,&Port_Class_Data);
		if(result != e_POE_STATUS_OK)
			goto Send_Layer2_PSE_Telemetry;

		Port_Class_Data++;

		PSE_Power_Pair = e_Alternative_A; /* Alt A (default) */

		/* check if port configure as Alt B */
		if(Masks.Back_off)
		{
			result = mscc_POE_CMD_Get_Single_PORT_CFG(_IN chPhyNum,_OUT &PORT_CFG);
			if(result != e_POE_STATUS_OK)
				goto Send_Layer2_PSE_Telemetry;

			if(PORT_CFG.Bits.PairControl == e_Alternative_B)
				PSE_Power_Pair = 0;  /* configure port as Alt B */
		}

		/* Port is on */
		if(PortStatusData != e_CustomerChannelOFF)
			MDI_Power_Status = CH_ON_POE_PSE;  /* report port is on, support POE, PSE */

	Send_Layer2_PSE_Telemetry:

		mscc_TelemetryAnswer("%w%w%c%c%c%c%c",PSE_Allocated_Power, PDReqPower_dW,
			Type.Byte, Port_Class_Data, PSE_Power_Pair, MDI_Power_Status, ChannelParams[chPhyNum].L2Data.CableLen);
		break;
	}


	case B_TmpMatrix: /* Get Physical Port Number from Temporary Matrix  */
	{
		mscc_TelemetryAnswer("%c", ChannelParams[chLogNum].mscc_Vir2PhyArrTemporaryMatrix);
		break;
	}

	default:
		return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(6);
	}

	return result;
}

void mscc_TelemetryAnswer(_IN S8 *pMsg, ...)
{
	va_list ap;
	va_start(ap, pMsg );
	mscc_Answer(B_Telemetry, pMsg, ap);
	va_end(ap);
}

/*---------------------------------------------------------------------
 *    description:    This function Execute Global Request
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_ExecGlobalRequest(_IN U8 *pData)
{
	U8 Tel[13];
	U8 a, b;
	LongMessage_t plmval;
	U8 Sub1 = pData[0];
	U8 Sub2 = pData[1];

	switch (Sub1)
	{
	case B_Versionz:
	{
		switch (Sub2)
		{
		case B_SWversion: /* Get Software Version */
		{
			mscc_TelemetryAnswer("%c%c%c%w%c%c%w", 0, B_Space, PRODUCT_NUMBER,
					SW_VERSION, 0, 0, 0);
			break;
		}

		case B_PoEDeviceVersion: /* Get PoE Device Version */
		{
			U8 IcIndex;
			U16 ICVer[4];
			U16 HW_Version_regData;

			for (IcIndex=0; IcIndex<4; IcIndex++)
			{
				tmpResult = mscc_IC_COMM_ReadDataFromSpecificIC(IcIndex,
						CFGC_ICVER_ADDR, &HW_Version_regData);

				/* For PD69000: bits 0-6 define the SW version;
				 * bits 7-9: RTL version; bits 10-11: analog version;
				 *  bits 12-15: family prefix */

				if (tmpResult == e_POE_STATUS_OK)
					ICVer[IcIndex]= HW_Version_regData;
				else
					ICVer[IcIndex] = MAX_WORD;
			}

			mscc_TelemetryAnswer("%w%w%w%w", ICVer[0], ICVer[1], ICVer[2],
					ICVer[3]);
			break;
		}
		default:
			return (4);
		}
		break;
	}

	case B_DeviceParams: /* Get PoE Device Status */
	{
		U16 ICVer= MAX_WORD; /* marked by default as IC Not Valid */
		U8 Tempr = 0xFF;
		U8 TSH = 0;
		U16 HW_Version_regData;
		U16 Tempr_Reg;

		/* Check that the CS is valid */
		if ( (Sub2 >= (MAX_IC_ON_BOARD)))
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(32)); /* Error - Value out of range */

		result = mscc_IC_COMM_ReadDataFromSpecificIC(Sub2,CFGC_ICVER_ADDR,&HW_Version_regData);
		if(result != e_POE_STATUS_OK) goto SendGetPoEDeviceStatusTelemetry;

		ICVer = HW_Version_regData;

		/* Get IC Status and IC_Ports*/
		result = mscc_POE_IC_FUNC_Get_IC_STATUS(IcParams[Sub2].IC_Exp,IcParams[Sub2].IC_HW,&IcParams[Sub2].IC_Status,&IcParams[Sub2].NumOfChannesInIC);
		if(result != e_POE_STATUS_OK) goto SendGetPoEDeviceStatusTelemetry;

		/* Get HW Version AND Measured temperature from IC */
		if (IcParams[Sub2].IC_Valid == POE_TRUE)/* Is IC Valid? */
		{
			result = mscc_IC_COMM_ReadDataFromSpecificIC(Sub2,MeasuredTemp_ADDR,&Tempr_Reg);
			if(result != e_POE_STATUS_OK) goto SendGetPoEDeviceStatusTelemetry;

			/* Read Measured Temp from IC */
			Tempr = mscc_POE_UTIL_ICValueToCelzius(Tempr_Reg);

			result = mscc_POE_CMD_GetTemperatureAlarmThershold (_IN Sub2,_IN &TSH);
			if(result != e_POE_STATUS_OK) goto SendGetPoEDeviceStatusTelemetry;
		}

		SendGetPoEDeviceStatusTelemetry:
		mscc_TelemetryAnswer("%c%w%c%c%c%c%c%c%c%c",Sub2,ICVer,IcParams[Sub2].IC_Status,IcParams[Sub2].IC_Exp,IcParams[Sub2].IC_HW,IcParams[Sub2].NumOfChannesInIC,Tempr,TSH,IcParams[Sub2].IC_SupportData,(U8)0);

		break;
	}

	case B_IrqMask: /* Get Interrupt Mask*/
	{
		mscc_ProtocolInterrutRegister_t InterruptMask_Data;
		InterruptMask_Data.Word = 0;

		result = mscc_POE_CMD_GetInterruptMask(SystemParams.IC_Master_Index,&InterruptMask_Data);

		mscc_TelemetryAnswer("%w",(U16)InterruptMask_Data.Word);
		break;
	}

	case B_Supply:
	{
		return mscc_ExecRequestSystemSupply(&(pData[1]));
	}

	case B_AllPortClass: /* Get All Ports Class */
	{
		U8 y;
		U8 i;
		U8 Port_Class_Data = 0;

		/* Check if Group is out of range - 'Class Group'=data[1]-> range is 0 to 5 */
		if (pData[1]> 5)
			return (FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(33)); /* Error - Class Group is out of range */

		a = (U8)(pData[1]*16);/* Calculate the first channel */
		memset(Tel, 0, 13);

		/* Set 8 Bytes where each nibble hold info for one channel -> 16 Channels */
		for(y = 0; y < 8; y++)
		{
			/* Set Nibbles */
			for (b = 0, i = 0; i < 2;i++)
			{
				Port_Class_Data = 0;

				if(a < SystemParams.NumOfChannelsInSystem)
				{
					result = mscc_POE_CMD_GetChannelClass(ChannelParams[a].mscc_Vir2PhyArrActiveMatrix,&Port_Class_Data);
					if(result != e_POE_STATUS_OK)
						goto SendGetAllPortClassTelemetry;
				}

				Tel[y] |= (Port_Class_Data << b);
				a++; /* Next Channel */
				b = 4; /* Next Nibble */
			}
		}

	SendGetAllPortClassTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = 8;
		mscc_TelemetryAnswer("%c%S",Sub2,&plmval);

		break;
	}

	case B_SystemStatus: /* Get System Status */
	{
		U8 DeviceFail=0xFF;            /* initialize as: Fail or missing PoE Device */
		U8 TemperatureDisconnect=0xFF; /* initialize as: This PoE device caused disconnection due to high temperature */
		U8 TemperatureAlarm=0xFF;      /* initialize as: High temperature */
		U16 protocolInterruptRegister=0;
		U8 maskBit = 1;
		U8 IcIndex;
		U16 SystemErrorFlags_RegData = 0xFF;
		mscc_ProtocolInterrutRegister_t protocolInterruptRegister4SingleIC;
		U16 CFGC_USRREG_RegData;

		/* get Device Fail */
		result = mscc_POE_IC_FUNC_GetActiveSlaveList(&DeviceFail);
		if(result != e_POE_STATUS_OK)
			goto SendGetSystemStatusTelemetry;

		/* get temperature alarm bit per chip byte */
		for(IcIndex=0;IcIndex<SystemParams.NumOfActiveICsInSystem;IcIndex++)
		{
			SystemErrorFlags_RegData = 0xFF;
			tmpResult = mscc_IC_COMM_ReadDataFromSpecificIC(IcIndex,SystemErrorFlags_ADDR,&SystemErrorFlags_RegData);

			/* if read operation succeed - update Temperature Disconnect and Temperature Alarm data
			 * otherwise continue to check the next chip						  				   */
			if(tmpResult == e_POE_STATUS_OK)
			{
				TemperatureDisconnect &= ~((((SystemErrorFlags_RegData & BIT1) == 0)?POE_TRUE:POE_FALSE) << IcIndex);
				TemperatureAlarm &= ~((((SystemErrorFlags_RegData & BIT5) == 0)?POE_TRUE:POE_FALSE) << IcIndex);
			}
			maskBit <<=1;
		}

		/* get Interrupt Register */
		for(IcIndex=0;IcIndex<SystemParams.NumOfActiveICsInSystem;IcIndex++)
		{
			protocolInterruptRegister4SingleIC.Word = 0;
			tmpResult = mscc_POE_CMD_GetInterruptRegister( IcIndex, &protocolInterruptRegister4SingleIC);

			/* if read operation succeed - update Interrupt Register data
			 * otherwise continue to check the next chip			    */
			if(tmpResult == e_POE_STATUS_OK)
			protocolInterruptRegister |= protocolInterruptRegister4SingleIC.Word;
		}


		for(IcIndex=0;IcIndex<SystemParams.NumOfActiveICsInSystem;IcIndex++)
		{
			tmpResult = mscc_IC_COMM_ReadDataFromSpecificIC(IcIndex,CFGC_USRREG_ADDR,&CFGC_USRREG_RegData);
			if((tmpResult == e_POE_STATUS_OK) && (CFGC_USRREG_RegData != mscc_PrivateLabel))
				mscc_PrivateLabel = 0;
		}

		SendGetSystemStatusTelemetry:
		mscc_TelemetryAnswer("%c%c%c%c%c%c%c%c%c%w",0,0,0,0,mscc_PrivateLabel,0,DeviceFail,TemperatureDisconnect,TemperatureAlarm,protocolInterruptRegister);
		break;
	}

	case B_EnDis: /* Get All Ports Enable/Disable Mode */
	{
			memset(Tel, 0, 13);

			result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 0,_OUT &Tel[0]);
			if(result != e_POE_STATUS_OK)
				goto SendAllPortsEnDis;
			result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 8,_OUT &Tel[1]);
			if(result != e_POE_STATUS_OK)
				goto SendAllPortsEnDis;
			result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 16,_OUT &Tel[2]);
			if(result != e_POE_STATUS_OK)
				goto SendAllPortsEnDis;
			result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 24,_OUT &Tel[4]);
			if(result != e_POE_STATUS_OK)
				goto SendAllPortsEnDis;
			result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 32,_OUT &Tel[5]);
			if(result != e_POE_STATUS_OK)
				goto SendAllPortsEnDis;
			result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 40,_OUT &Tel[6]);

	SendAllPortsEnDis:
			plmval.pDataPointer = Tel;
			plmval.Length = 8;
			mscc_TelemetryAnswer("%S",&plmval);
			break;
		}

	case B_EnDis2:
	{
		memset(Tel, 0, 13);

		result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 48,_OUT &Tel[0]);
		if(result != e_POE_STATUS_OK)
			goto SendAllPortsEnDis2;
		result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 56,_OUT &Tel[1]);
		if(result != e_POE_STATUS_OK)
			goto SendAllPortsEnDis2;
		result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 64,_OUT &Tel[2]);
		if(result != e_POE_STATUS_OK)
			goto SendAllPortsEnDis2;
		result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 72,_OUT &Tel[4]);
		if(result != e_POE_STATUS_OK)
			goto SendAllPortsEnDis2;
		result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 80,_OUT &Tel[5]);
		if(result != e_POE_STATUS_OK)
			goto SendAllPortsEnDis2;
		result = mscc_POE_CMD_GetChannelsGroupEnDisState(_IN 88,_OUT &Tel[6]);

 SendAllPortsEnDis2:
		plmval.pDataPointer = Tel;
		plmval.Length = 8;
		mscc_TelemetryAnswer("%S",&plmval);
		break;
	}

	case B_Maskz: /* Get Masks Status */
	{
		U8 MASKS = 4; /* MaskBit2 must be always '1' */
		U16 SysFlags_RegData;
		U8 CapEn;

		MASKS |= SystemParams.bPM_PowerDisconnentProcess;

		/* Register:
		 0 -Cap enabled
		 1 - Cap disabled

		 Protocol:
		 0 = RES mode.
		 1 (default) = RES+CAP
		 */

		/* read System Flags register data */

		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index ,SysFlags_ADDR,&SysFlags_RegData);
		if(result != e_POE_STATUS_OK) goto SendGetMasksStatusTelemetry;

		CapEn = (((SysFlags_RegData >> e_CapDis) & 1) == 0)?1:0;

		MASKS |= (CapEn << 1);

		SendGetMasksStatusTelemetry:
		mscc_TelemetryAnswer("%c",MASKS);

		break;
	}

	case B_Individual_Mask: /* Get Individual Mask */
	{
		U8 Individual_Mask_Data = 0;
		PORT_CFG_t PORT_CFG;

		switch(Sub2)
		{
			case e_MaskKey_AC_DisEnable: /*  0x09 */
			{
				U16 SysFlags_RegData;

				result = mscc_IC_COMM_ReadDataFromSpecificIC(_IN SystemParams.IC_Master_Index,SysFlags_ADDR,&SysFlags_RegData);
				if(result == e_POE_STATUS_OK)
				{
					Masks.AC_DisEnable = (SysFlags_RegData >> e_DC_DiscoEn) & 1;
					Individual_Mask_Data = (U8)Masks.AC_DisEnable;
				}
				break;
			}
			case e_MaskKey_Back_off: /* 17 */
			{
				/*
				 protocol:
				 En/Dis '0' - Alternative A.
				 En/Dis '1' - Alternative B

				 registers:
				 00 - Reserved
				 01 - Alternative A
				 10 - Alternative B (Backoff Enable)
				 11 - Reserved
				 */

				result = mscc_POE_CMD_Get_Single_PORT_CFG(_IN ChannelParams[0].mscc_Vir2PhyArrActiveMatrix ,_OUT &PORT_CFG);
				if(result == e_POE_STATUS_OK)
				{
					Masks.Back_off = 0;

					if (PORT_CFG.Bits.PairControl == 2)
						Masks.Back_off = 1;

					Individual_Mask_Data = Masks.Back_off;
				}

				break;
			}

			case e_MaskKey_Ignore_priority:
			{
				U16 StartupCR_ADDR_RegData;

				result = mscc_IC_COMM_ReadDataFromSpecificIC(_IN SystemParams.IC_Master_Index,StartupCR_ADDR,&StartupCR_ADDR_RegData);
				if(result == e_POE_STATUS_OK)
				{
					Masks.Ignore_priority = (StartupCR_ADDR_RegData >> 6) & 1;
					Individual_Mask_Data = (U8)Masks.Ignore_priority;
				}
				break;
			}
			case e_MaskKey_Layer2: /*  46 */
			{
				Individual_Mask_Data = Masks.Layer2;
				break;
			}
			case e_MaskKey_Layer2PriorityByPD: /*  47 */
			{
				Individual_Mask_Data = Masks.Layer2PriorityByPD;
				break;
			}
			default:
			{
				return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(7);
			}
		}

		mscc_TelemetryAnswer("%c",Individual_Mask_Data);
		break;
	}

	case B_UDLcounter4: /* Get UDL Counters -  UDLCounter4 */
	{
		a = 72; /* Start With Channel 72 to 96 */
		goto UDLcounter_s;
	}

	case B_UDLcounter3: /* Get UDL Counters -  UDLCounter3 */
	{
		a = 48; /* Start With Channel 48 to 71 */
		goto UDLcounter_s;
	}

	case B_UDLcounter2: /* Get UDL Counters -  UDLCounter2 */
	{
		a = 24;/* Start With Channel 24 to 47 */
		goto UDLcounter_s;
	}

	case B_UDLcounter1: /* Get UDL Counters -  UDLCounter1 */
	{
		a = 0; /* Start With Channel 0 to 23 */
	}

	UDLcounter_s:
	{
		U8 y;
		U8 i;
		U8 CounterValue;

		memset(Tel, 0, 13);

		/* Loop Over 24 Channels - Two BITS per Channel */
		for(y = 0; y < 6; y++)
		{
			b=0;
			for (i = 0; i < 4; i++)
			{
				CounterValue = 0;

				if(a < SystemParams.NumOfChannelsInSystem)
				{
					result = mscc_POE_CMD_GetChannelCounter(ChannelParams[a].mscc_Vir2PhyArrActiveMatrix,Port0UnderloadCnt_ADDR,&CounterValue);
					if(result != e_POE_STATUS_OK) goto SendGetUDLCountersTelemetry;
				}

				Tel[y] |= (CounterValue << b);

				a++; /* Next Channel */
				b += 2;
			}
		}

	SendGetUDLCountersTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = 6;
		mscc_TelemetryAnswer("%S",&plmval);
		break;
	}

	case B_DetCnt4: /* Get Detection Failure Counters  -  DetCnt4 */
	{
		a = 72;/* Start With Channel 72 to 96 */
		goto DetectionCounter_s;
	}

	case B_DetCnt3: /* Get Detection Failure Counters  -  DetCnt3 */
	{
		a = 48; /* Start With Channel 48 to 71 */
		goto DetectionCounter_s;
	}

	case B_DetCnt2: /* Get Detection Failure Counters  -  DetCnt2 */
	{
		a = 24; /* Start With Channel 24 to 47 */
		goto DetectionCounter_s;
	}

	case B_DetCnt1: /* Get Detection Failure Counters  -  DetCnt1 */
	{
		a = 0; /* Start With Channel 0 to 23 */
	}

	DetectionCounter_s:
	{
		U8 y;
		U8 i;
		U8 CounterValueRegData;

		memset(Tel, 0, 6);

		/* Loop Over 24 Channels - Two BITS per Channel */
		for(y = 0; y < 6; y++)
		{
			b=0;

			for (i = 0; i < 4; i++)
			{
				CounterValueRegData = 0;

				if(a < SystemParams.NumOfChannelsInSystem)
				{
					result = mscc_POE_CMD_GetChannelCounter(ChannelParams[a].mscc_Vir2PhyArrActiveMatrix,Port0InvalidSigCnt_ADDR, &CounterValueRegData);
					if(result != e_POE_STATUS_OK) goto SendGetDetectionFailureCountersTelemetry;
				}

				Tel[y] |= (CounterValueRegData << b);

				a++; /* Next Channel */
				b += 2;
			}
		}

		SendGetDetectionFailureCountersTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = 6;
		mscc_TelemetryAnswer("%S",&plmval);
		break;
	}

	case B_Latches4: /* Get Latches  -  Latches */
	{
		a = 72; /* Start With Channel 72 to 96 */
		goto Latches_s;
	}

	case B_Latches3: /* Get Latches  -  Latches2 */
	{
		a = 48; /* Start With Channel 48 to 71 */
		goto Latches_s;
	}

	case B_Latches2: /* Get Latches  -  Latches3 */
	{
		a = 24; /* Start With Channel 24 to 47 */
		goto Latches_s;
	}

	case B_Latches: /* Get Latches  -  Latches4 */
	{
		a = 0; /* Start With Channel 0 to 23 */
	}

	Latches_s:
	{
		U8 y;
		U8 i;
		U16 PortLatch_RegData;
		U8 PhyIcNumber, PhyPortNumber;
		U8 Port_Latch_Data;

		memset(Tel, 0, 13);

		for(y = 0; y < 6; y++)
		{
			b=0;
			for (i = 0; i < 4; i++)
			{
				PortLatch_RegData = 0;

				if(a < SystemParams.NumOfChannelsInSystem)
				{
					/* calculate PhyIcNumber and PhyPortNumber */
					result = mscc_POE_IC_FUNC_Log2Phy(ChannelParams[a].mscc_Vir2PhyArrActiveMatrix,&PhyIcNumber ,&PhyPortNumber);
					if(result != e_POE_STATUS_OK)
						goto SendGetLatchesTelemetry;

					/* cleared the port using the indications clear sync event */
					result = mscc_IC_COMM_WriteDataToSpecificIC(PhyIcNumber,IndicationClearPortSelect_ADDR,PhyPortNumber);
					if(result != e_POE_STATUS_OK)
						goto SendGetLatchesTelemetry;

					/* SetI2C_EXT_SYNC */
					result = mscc_IC_COMM_WriteDataToAllICs(I2C_ExtSyncType_ADDR,0x0008);
					if(result != e_POE_STATUS_OK)
						goto SendGetLatchesTelemetry;

					/* EXT_EV_IRQ */
					result = mscc_IC_COMM_WriteDataToAllICs(EXT_EV_IRQ_ADDR,0x0008);
					if(result != e_POE_STATUS_OK)
						goto SendGetLatchesTelemetry;

					result = mscc_IC_COMM_ReadDataFromSpecificPort(ChannelParams[a].mscc_Vir2PhyArrActiveMatrix,Port0Indications_ADDR,&PortLatch_RegData);
					if(result != e_POE_STATUS_OK)
						goto SendGetLatchesTelemetry;
				}

				/* decided Latch */
				Port_Latch_Data = (U8)(PortLatch_RegData & 3);
				Tel[y] |= (Port_Latch_Data << b);

				b+=2; /* Next OVL,UDL Group */
				a++; /* Next Channel */
			}
		}

	SendGetLatchesTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = 6;
		mscc_TelemetryAnswer("%S",&plmval);

		break;
	}

	case B_PortsStatus10: /* Get All Ports Status - PortsStatus10 */
	{
		b = 92; /* start loop */
		a = 96; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus9: /* Get All Ports Status - PortsStatus9 */
	{
		b = 81; /* start loop */
		a = 92; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus8: /* Get All Ports Status - PortsStatus8 */
	{
		b = 70; /* start loop */
		a = 81; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus7: /* Get All Ports Status - PortsStatus7 */
	{
		b = 59; /* start loop */
		a = 70; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus6: /* Get All Ports Status - PortsStatus6 */
	{
		b = 48; /* start loop */
		a = 59; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus5: /* Get All Ports Status - PortsStatus5 */
	{
		b = 37; /* start loop */
		a = 48; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus4: /* Get All Ports Status - PortsStatus4 */
	{
		b = 26; /* start loop */
		a = 37; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus2: /* Get All Ports Status - PortsStatus2 */
	{
		b = 11; /* start loop */
		a = 22; /* end of the loop */
		goto Get_Channels_Status;
	}

	case B_PortsStatus1: /* Get All Ports Status - PortsStatus1 */
	{
		b = 0; /* start loop */
		a = 11; /* end of the loop */
		goto Get_Channels_Status;
	}

	Get_Channels_Status:
	{
		U8 i;
		U8 AtType;
		U8 PortStatusData;
		U8 bIsPortOn;

		for ( i = 0; i < 13; i++)
			Tel[i] = e_CustomerUnKnownICStatus;

		for ( i = b; i < a; i++)
		{
			AtType = 0;
			PortStatusData = e_CustomerPortNotActive;

			if(i < SystemParams.NumOfChannelsInSystem)
			{
				result = mscc_POE_CMD_GetChannelStatus(ChannelParams[i].mscc_Vir2PhyArrActiveMatrix,&PortStatusData,&bIsPortOn,&AtType);
				if(result != e_POE_STATUS_OK)
					goto SendGetAllPortsStatusTelemetry;
			}

			Tel[i-b] = PortStatusData;
		}

		SendGetAllPortsStatusTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = a - b;
		mscc_TelemetryAnswer("%S",&plmval);
		break;
	}

	case B_PortsStatus3: /* Get All Ports Status - PortsStatus3 */
	{
		U8 i;
		U8 AtType;
		U8 PortStatusData;
		U8 bIsPortOn;

		memset(Tel, 0, 13);

		for (i = 22; i < 26; i++)
		{
			AtType = 0;
			PortStatusData = e_CustomerPortNotActive;

			if(i < SystemParams.NumOfChannelsInSystem)
			{
				result = mscc_POE_CMD_GetChannelStatus(ChannelParams[i].mscc_Vir2PhyArrActiveMatrix,&PortStatusData, &bIsPortOn, &AtType);
				if(result != e_POE_STATUS_OK)
					goto SendGetAllPortsStatus3Telemetry;
			}

			Tel[i-22] = PortStatusData;
		}

		SendGetAllPortsStatus3Telemetry:

		mscc_TelemetryAnswer("%c%c%c%c%c%c%c",Tel[0],Tel[1],0, 0,0,Tel[2],Tel[3]);
		break;
	}

	case B_AllPortsPower : /* Get All Ports Power  -  the resolution is 0.2w  */
	{
		U16 PortPowerCons_mW;
		U8 i;

		if(Sub2> mscc_MAX_ALL_POWER_GROUP_NUM)
		return(FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(34)); /* Error - Value out of range */

		/*get first & last ports in group */
		a = Sub2*NUM_PORTS_IN_GROUP;
		b = ( Sub2 == mscc_MAX_ALL_POWER_GROUP_NUM ? 96 : (a + NUM_PORTS_IN_GROUP) );

		/*fill power of all ports in group */
		for(i = a; i < b; i++)
		{
			PortPowerCons_mW = 0;

			if(i < SystemParams.NumOfChannelsInSystem)
			{
				result = mscc_POE_CMD_GetChannelPower(ChannelParams[i].mscc_Vir2PhyArrActiveMatrix,&PortPowerCons_mW);
				if(result != e_POE_STATUS_OK)
					goto SendGetAllPortsHighPowerTelemetry;
			}

			Tel[i - a] = (U8)( PortPowerCons_mW / mscc_ALLPORTPOWER_RESOLUTION); /* convert mW 1/5 Watt */
		}

		SendGetAllPortsHighPowerTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = b - a;
		mscc_TelemetryAnswer("%S",&plmval);
		break;
	}

	case B_PortsPower10: /* Get All Ports Power - PortsPower10 */
	{
		b = 92; /* start loop */
		a = 96; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower9: /* Get All Ports Power - PortsPower9 */
	{
		b = 81; /* start loop */
		a = 92; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower8: /* Get All Ports Power - PortsPower8 */
	{
		b = 70; /* start loop */
		a = 81; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower7: /* Get All Ports Power - PortsPower7 */
	{
		b = 59; /* start loop */
		a = 70; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower6: /* Get All Ports Power - PortsPower6 */
	{
		b = 48; /* start loop */
		a = 59; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower5: /* Get All Ports Power - PortsPower5 */
	{
		b = 37; /* start loop */
		a = 48; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower4: /* Get All Ports Power - PortsPower4 */
	{
		b = 26; /* start loop */
		a = 37; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower2: /* Get All Ports Power - PortsPower2 */
	{
		b = 11; /* start loop */
		a = 22; /* end of the loop */
		goto Get_Channels_power;
	}

	case B_PortsPower1: /* Get All Ports Power - PortsPower1 */
	{
		b = 0; /* start loop */
		a = 11; /* end of the loop */
		goto Get_Channels_power;
	}

	Get_Channels_power:
	{
		U8 i;
		U16 PortPowerRegData;

		memset(Tel, 0, 13);

		for (i = b; i < a; i++)
		{
			PortPowerRegData = 0;

			if(i < SystemParams.NumOfChannelsInSystem)
			{
				result = mscc_POE_CMD_GetChannelPower(ChannelParams[i].mscc_Vir2PhyArrActiveMatrix,&PortPowerRegData);
				if(result != e_POE_STATUS_OK)
					goto SendGetAllPortsPowerTelemetry;
			}

			if(PortPowerRegData> mscc_MAX_DECIWATT_VALUE_IN_BYTE)
				Tel[i-b] = 0xFF; /* max possible value */
			else
				Tel[i-b] = (U8)(PortPowerRegData/100); /* convert mW to deciWatt */
		}

		SendGetAllPortsPowerTelemetry:

		plmval.pDataPointer = Tel;
		plmval.Length = a - b;
		mscc_TelemetryAnswer("%S",&plmval);

		break;
	}

	case B_PortsPower3: /* Get All Ports Power - PortsPower3 */
	{
		U16 VMAIN_Data_deciVolts = 0;
		U16 PowerConsumption_regData = 0;
		U8 i;
		U16 Vmain_RegData;
		U16 UpdateRLPMParams_DATA;
		U8 ActivePowerBank;
		U16 SysPowerBudget_regData;
		U16 SysPowerBudgetData = 0;

		memset(Tel, 0, 13);

		for (i = 22; i < 26; i++)
		{
			U16 PortPower_RegData = 0;

			if(i < SystemParams.NumOfChannelsInSystem)
			{
				result = mscc_POE_CMD_GetChannelPower(ChannelParams[i].mscc_Vir2PhyArrActiveMatrix,&PortPower_RegData);
				if(result != e_POE_STATUS_OK)
					goto SendGetAllPortsPower3Telemetry;
			}

			if(PortPower_RegData> mscc_MAX_DECIWATT_VALUE_IN_BYTE)
				Tel[i-22] = 0xFF; /* max possible value */
			else
				Tel[i-22] = (U8)(PortPower_RegData/100);
		}

		/* VMAIN */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,VMAIN_ADDR,&Vmain_RegData);
		if(result != e_POE_STATUS_OK)
			goto SendGetAllPortsPower3Telemetry;
		VMAIN_Data_deciVolts = mscc_POE_UTIL_ConvertICValueToDeciVolt(Vmain_RegData);

		UpdateRLPMParams_DATA = 1;

		/* sync command */
		result = mscc_IC_COMM_WriteDataToSpecificIC( SystemParams.IC_Master_Index, UpdateRLPMParams_ADDR, UpdateRLPMParams_DATA);
		if(result != e_POE_STATUS_OK)
			goto SendGetAllPortsPower3Telemetry;

		result = OS_Sleep_mS(20);
		if(result != e_POE_STATUS_OK)
			goto SendGetAllPortsPower3Telemetry;

		/* Power Consumption */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysTotalRealPowerCons_ADDR,&PowerConsumption_regData);
		if(result != e_POE_STATUS_OK)
			goto SendGetAllPortsPower3Telemetry;

		/* selected Bank */
		result = mscc_POE_CMD_GetActiveBank(_IN  SystemParams.IC_Master_Index,_OUT &ActivePowerBank);
		if(result != e_POE_STATUS_OK)
			goto SendGetAllPortsPower3Telemetry;

		/* Max Power Available */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(ActivePowerBank*2) ,&SysPowerBudget_regData);
		if(result != e_POE_STATUS_OK)
			goto SendGetAllPortsPower3Telemetry;

		SysPowerBudgetData = mscc_POE_UTIL_ConvertFromDeciWattToWatts(SysPowerBudget_regData);

		SendGetAllPortsPower3Telemetry:

		mscc_TelemetryAnswer("%c%c%w%w%w%c%c",Tel[0],Tel[1],VMAIN_Data_deciVolts,PowerConsumption_regData,SysPowerBudgetData,Tel[2],Tel[3]);
		break;
	}

	default:
	return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(8);
}

return result;
}

/*---------------------------------------------------------------------
 *    description:    This function Execute Request System Supply
 *
 *    input :   pData - data from comm
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         		- operation succeed
 * 			    != e_POE_STATUS_OK          	- operation failed
 *---------------------------------------------------------------------*/
S32 mscc_ExecRequestSystemSupply(_IN U8 *pData)
{
	switch (pData[0]) {

	case B_PowerManageMode: /* Get PM Method */
	{
		/* default - S1 or S2 */
		U8 PM1 =0;
		U8 PM2 =0;
		U8 PM3 =0;

		U16 SysFlags_RegData;

		U8 IcutMaxFlag_Data;
		U8 PM_CalcMode_Data;

		/* read System Flags register data */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				                                       SysFlags_ADDR, &SysFlags_RegData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPMMethodTelemetry;

		IcutMaxFlag_Data =(U8) ( (SysFlags_RegData >> e_IcutMaxFlag) & 1);
		PM_CalcMode_Data = (U8)( (SysFlags_RegData >> e_PM_Mode) & 1);

		/*
		 *PM Mode				Management Mode	Cat Ref	Total Allocated Power	Port Power Limit	Start Condition
		 *PM-1	PM-2	PM-3
		 *0x00	0x02	0x00	Dynamic			D		Consumption 			Max. 				None
		 *0x00	0x00	0x00	Static			S1		Consumption				Predefined			None
		 *0x00	0x00	0x01					S2		Consumption 			Predefined			Class
		 *> 0	0x01	0x00	Class			C1		Class					class 				None
		 *> 0	0x02	0x00					C2		Class 					Max.				None
		 */


		/* 0 - Static (according to class or PPL)
		 * 1 - Dynamic (according to real consumption) */
		bStaticPM_CalcMode = POE_FALSE;

		/* Default (D) - Dynamic, Port Power Limit set to Max, no start condition */
		if ((IcutMaxFlag_Data == 1) && (PM_CalcMode_Data ==1))
		{
			PM2 = 2;
		}/* (C1) -  Class based calculation, Port Power is limited on class or pre defined value (PPL) */
		else if ((IcutMaxFlag_Data == 0) && (PM_CalcMode_Data ==0))
		{
			PM1 = 1;
			PM2 = 1;
		}/* (C2) -  Class based calculation, Port Power is max available */
		else if ((IcutMaxFlag_Data == 1) && (PM_CalcMode_Data ==0))
		{
			PM1 = 1;
			PM2 = 2;
			bStaticPM_CalcMode = POE_TRUE;
		}

	SendGetPMMethodTelemetry:

		mscc_TelemetryAnswer("%c%c%c", PM1, PM2, PM3);
		break;
	}

	case B_PowerBudget: /* Get Power Banks */
	{
		U8 BankNumber = pData[1];
		U16 SysPowerBudgetData = 0;
		U16 MaxShutdownVoltageData = 0;
		U16 MinShutdownVoltageData = 0;
		U16 SysPowerBudget_regData;
		U16 MaxShutdownVoltage_RegData;
		U16 MinShutdownVoltage_RegData;

		/* check if the received power banke number is in range */
		if (BankNumber >= MAX_BANK_IN_IC)
			return FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(35);

		/* System Power Budget */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				SysPowerBudget0_ADDR+(BankNumber*2), &SysPowerBudget_regData);

		if (result != e_POE_STATUS_OK)
			goto SendGetPowerBanksTelemetry;

		SysPowerBudgetData = mscc_POE_UTIL_ConvertFromDeciWattToWatts(SysPowerBudget_regData);

		/* Vmain High Th */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				VmainHighTh_ADDR, &MaxShutdownVoltage_RegData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerBanksTelemetry;

		MaxShutdownVoltageData = mscc_POE_UTIL_ConvertICValueToDeciVolt(MaxShutdownVoltage_RegData);

		/* Vmain Af Low Th */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				VmainAfLowTh_ADDR, &MinShutdownVoltage_RegData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerBanksTelemetry;

		MinShutdownVoltageData	= mscc_POE_UTIL_ConvertICValueToDeciVolt(MinShutdownVoltage_RegData);

SendGetPowerBanksTelemetry:

		mscc_TelemetryAnswer("%w%w%w%c%c", SysPowerBudgetData, MaxShutdownVoltageData,
				MinShutdownVoltageData, 0,Budget[BankNumber].PowerSourceType);
		break;
	}

	case B_Main: /* Get Power Supply Parameters */
	{
		U16 PowerConsumptionData = 0;
		U16 MaxShutdownVoltageData = 0;
		U16 MinShutdownVoltageData = 0;
		U8 ActivePowerBank = 0;
		U16 SysPowerBudgetData = 0;
		U16 PowerConsumption_regData;
		U16 MaxShutdownVoltage_regData;
		U16 MinShutdownVoltage_regData;
		U16 SysPowerBudget_regData;

		/* sync command */
		result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,	UpdateRLPMParams_ADDR, 1);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;

		result = OS_Sleep_mS(20);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;

		/* Power Consumption */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				SysTotalRealPowerCons_ADDR, &PowerConsumption_regData);

		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;

		PowerConsumptionData= mscc_POE_UTIL_ConvertFromDeciWattToWatts(PowerConsumption_regData);

		/* VmainHighTh */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				VmainHighTh_ADDR, &MaxShutdownVoltage_regData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;
		MaxShutdownVoltageData = mscc_POE_UTIL_ConvertICValueToDeciVolt(MaxShutdownVoltage_regData);

		/* VmainAfLowTh */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				VmainAfLowTh_ADDR, &MinShutdownVoltage_regData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;
		MinShutdownVoltageData	= mscc_POE_UTIL_ConvertICValueToDeciVolt(MinShutdownVoltage_regData);

		/* Get selected Power Bank */
		result = mscc_POE_CMD_GetActiveBank(_IN  SystemParams.IC_Master_Index,_OUT &ActivePowerBank);
		if(result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;

		/* Get System Power Limit */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				SysPowerBudget0_ADDR+(ActivePowerBank*2), &SysPowerBudget_regData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyParametersTelemetry;

		SysPowerBudgetData = mscc_POE_UTIL_ConvertFromDeciWattToWatts(SysPowerBudget_regData);

	SendGetPowerSupplyParametersTelemetry:

		mscc_TelemetryAnswer("%w%w%w%c%c%w%c", PowerConsumptionData,
				MaxShutdownVoltageData, MinShutdownVoltageData, 'N',
				ActivePowerBank, SysPowerBudgetData, 'N');
		break;
	}

	case B_ExpendedPowerInfo: /* Get Total Power */
	{
		U16 SysTotalRealPowerConsData = 0;
		U16 SysTotalCalcPowerConsData = 0;

		result = mscc_POE_CMD_Get_Total_Power(_OUT &SysTotalRealPowerConsData,_OUT &SysTotalCalcPowerConsData);
		if (result != e_POE_STATUS_OK)
			return result;

		mscc_TelemetryAnswer("%w%w", SysTotalRealPowerConsData,
				SysTotalCalcPowerConsData);
		break;
	}

	case B_Measurementz: /* Get Power Supply Voltage */
	{
		U16 VMAIN_Data_deciVolts = 0;
		U16 Vmain_RegData;

		/* VMAIN Voltage */
		result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
				VMAIN_ADDR, &Vmain_RegData);
		if (result != e_POE_STATUS_OK)
			goto SendGetPowerSupplyVoltageTelemetry;

		VMAIN_Data_deciVolts= mscc_POE_UTIL_ConvertICValueToDeciVolt(Vmain_RegData);

		SendGetPowerSupplyVoltageTelemetry:

		mscc_TelemetryAnswer("%w", VMAIN_Data_deciVolts);
		break;
	}

	case B_BackplanePowerData: /* Get BPM Data */
	{
		U16 CalculatedPowerCritical = 0;
		U16 CalculatedPowerHigh = 0;
		U16 CalculatedPowerLow = 0;
		U16 DeltaPower = 0;
		U8 I2C_Address;
		U8 Rxdata[18];
		U16 number_of_bytes_to_read;

		/* Calculated Power */
		I2C_Address = IcParams[SystemParams.IC_Master_Index].IC_Address;

		/* sync command */
		result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,	UpdateRLPMParams_ADDR, 1);
		if (result != e_POE_STATUS_OK)
			goto SendGetBPMDataTelemetry;

		result = OS_Sleep_mS(20);
		if (result != e_POE_STATUS_OK)
			goto SendGetBPMDataTelemetry;

		number_of_bytes_to_read = 18;
		result = mscc_IC_COMM_I2C_Read(_IN I2C_Address, _IN SysTotalCriticalCons_ADDR,_OUT Rxdata, _IN number_of_bytes_to_read);
		if (result != e_POE_STATUS_OK)
			goto SendGetBPMDataTelemetry;

		CalculatedPowerCritical = mscc_GetExtractData(Rxdata[0], Rxdata[1]);
		CalculatedPowerHigh = mscc_GetExtractData(Rxdata[2], Rxdata[3]);
		CalculatedPowerLow = mscc_GetExtractData(Rxdata[4], Rxdata[5]);
		DeltaPower = mscc_GetExtractData(Rxdata[16], Rxdata[17]);

SendGetBPMDataTelemetry:

		mscc_TelemetryAnswer("%w%w%w%w", CalculatedPowerCritical,
				CalculatedPowerHigh, CalculatedPowerLow, DeltaPower);
		break;
	}

	case B_BPMReqData: /* Get BPM Request Data */
	{
		U16 RequestedPowerCritical = 0;
		U16 RequestedPowerHigh = 0;
		U16 RequestedPowerLow = 0;
		U8 I2C_Address;
		U8 Rxdata[6];
		U16 number_of_bytes_to_read;

		I2C_Address = IcParams[SystemParams.IC_Master_Index].IC_Address;

		/* sync command */
		result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,	UpdateRLPMParams_ADDR, 1);
		if (result != e_POE_STATUS_OK)
			goto SendGetBPMRequestDataTelemetry;

		result = OS_Sleep_mS(20);
		if (result != e_POE_STATUS_OK)
			goto SendGetBPMRequestDataTelemetry;

		/* read Requested Power registers*/

		number_of_bytes_to_read = 6;
		result = mscc_IC_COMM_I2C_Read(_IN I2C_Address, _IN SysTotalCriticalReq_ADDR,_OUT Rxdata, _IN number_of_bytes_to_read);
		if (result != e_POE_STATUS_OK)
			goto SendGetBPMRequestDataTelemetry;

		RequestedPowerCritical = mscc_GetExtractData(Rxdata[0], Rxdata[1]);
		RequestedPowerHigh = mscc_GetExtractData(Rxdata[2], Rxdata[3]);
		RequestedPowerLow = mscc_GetExtractData(Rxdata[4], Rxdata[5]);

		SendGetBPMRequestDataTelemetry:

		mscc_TelemetryAnswer("%w%w%w", RequestedPowerCritical, RequestedPowerHigh,
				RequestedPowerLow);
		break;
	}

	default:
		return FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(9);
	}

	return result;
}
