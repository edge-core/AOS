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
 *  Description: contain the functions that operates for each task.
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_CMD_H_
	#define _MSCC_POE_CMD_H_

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
	S32 mscc_POE_CMD_GetChannelsGroupEnDisState(_OUT U8 startChannel,_IN U8 *EnDisData);
	S32 mscc_POE_CMD_GetChannelLatch			(_IN U8 logicalPortNumber,_OUT U8 *pLatchData);
	S32 mscc_POE_CMD_GetInterruptMask           (_IN U8 IC_Number,_OUT mscc_ProtocolInterrutRegister_t *pProtocolInterrutRegister);
	S32 mscc_POE_CMD_GetInterruptRegister       (_IN U8 IC_Number,_OUT mscc_ProtocolInterrutRegister_t *pProtocolInterrutRegister);
	S32 mscc_POE_CMD_SetInterruptMask			(_IN U16 InterruptMask_Data);
	S32 mscc_POE_CMD_SetSingleBitInSysFlags		(_IN U8 bitIndex, _IN U8 dataEnDis);
	S32 mscc_POE_CMD_SetSysFlags				(_IN U16 SysFlags_RegData);
	S32 mscc_SendPreSysConfigCommands			(void);
	S32 mscc_SendPostSysConfigCommands			(void);
	S32 mscc_POE_CMD_GetChannelPower			(_IN U8 logicalPortNumber,_OUT U16 *pPortPowerCons_mW);
	S32 mscc_POE_CMD_GetChannelStatus           (_IN U8 logicalPortNumber,_OUT U8 *pCustomerPortStatus,_OUT U8 *bIsPortOn, _OUT U8 *AtType);
	S32 mscc_POE_CMD_GetChannelClass			(_IN U8 logicalPortNumber,_OUT U8 *pPort_Class_Data);
	S32 mscc_POE_CMD_GetChannelPPL				(_IN U8 logicalPortNumber,_OUT U16 *pPPL_Data_mW);
	S32 mscc_POE_CMD_SetChannelPPL				(_IN U8 logicalPortNumber,_IN U16 PPL_Data_mW);
	S32 mscc_POE_CMD_GetChannelTPPL				(_IN U8 logicalPortNumber,_OUT U16 *pTPPL_Data_mW);
	S32 mscc_POE_CMD_SetChannelTPPL				(_IN U8 logicalPortNumber,_IN U16 TPPL_Data_mW);
	S32 mscc_POE_CMD_SetAllChannelPPL           (_IN U16 PPL_Data_mW);
	S32 mscc_POE_CMD_SetForcePower				(_IN U8 logicalPortNumber,U16 Cmd);
	S32 mscc_POE_CMD_GetChannelPowerInfo		(_IN U8 logicalPortNumber,_OUT PowerInfo_t *pPowerInfo);
	S32 mscc_POE_CMD_GetChannelCounter			(_IN U8 logicalPortNumber,_IN U16 PortCnt_ADDR,_OUT U8 *pCounterValue);
	S32 mscc_POE_CMD_GetVmain					(_IN U8 IC_Number,_OUT U16 *pVMAIN_Data_deciVolts);
	S32 mscc_POE_CMD_SetVmainThersholds         (_IN U16 VmainHighThershold_Data_deciVolts,_IN U16 VmainLowThershold_Data_deciVolts);
	S32 mscc_POE_CMD_GetVmainHighThershold		(_IN U8 IC_Number,_OUT U16 *pVmainHighThershold_Data_deciVolts);
	S32 mscc_POE_CMD_GetVmainLowThershold		(_IN U8 IC_Number,_OUT U16 *pVmainLowThershold_Data_deciVolts);
	S32 mscc_POE_CMD_GetICConfiguration         (_IN U8 IC_Number,_OUT U8 *pIs_Auto_mode_master_capabilities,_OUT mscc_StandardMode_e *pStandardMode_e,_OUT U8 *pIs_support_12_ports);
	S32 mscc_POE_CMD_GetActiveBank              (_IN U8 IC_Number,_OUT U8 *pActiveBank);
	S32 mscc_POE_CMD_SetTemperatureAlarmThershold(_IN U8 IC_Number,_IN U8 TemperatureAlarm_celsius);
	S32 mscc_POE_CMD_GetTemperatureAlarmThershold(_IN U8 IC_Number,_OUT U8 *pTemperatureAlarm_celsius);
	S32 mscc_POE_CMD_Set_Single_PORT_CFG		 (_IN U8 logicalPortNumber,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue);
	S32 mscc_POE_CMD_Set_Single_PORT_CFG_without_disable_port(_IN U8 physicalPortNumber,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue);
	S32 mscc_POE_CMD_Get_Single_PORT_CFG		(_IN U8 logicalPortNumber,_OUT PORT_CFG_t *pPORT_CFG);
	S32 mscc_POE_CMD_Set_All_PORTS_CFG			(_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue);
	S32 mscc_POE_CMD_Set_PORTS_CFG_to_a_single_IC(_IN U8 icIndex,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue);
	S32 mscc_POE_CMD_Get_Total_Power            (_OUT U16 *pSysTotalRealPowerConsData,_OUT U16 *pSysTotalCalcPowerConsData);
	S32 mscc_POE_CMD_Get_SysTotalCalcPower      (_OUT U16 *SysTotalCalcPowerConsData_dW);

	U16 mscc_POE_CMD_IgnoreHighPriority();

	/****************** Layer2 functions *******************/
	U16 mscc_POE_CMD_ConvertPSEAllocation(_IN U16 val,_IN U8 channel);
	S32 mscc_POE_CMD_LLDP_AndLayer2PM(void);


#endif /* _MSCC_POE_CMD_H_ */
