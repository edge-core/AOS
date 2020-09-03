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
 *  Description: contain the "Low level" functions implementation.
 *
 *
 *************************************************************************/

/*=========================================================================
/ INCLUDES
/========================================================================*/
#include "mscc_poe_ic_func.h"
#include "mscc_poe_ic_communication.h"
#include "mscc_poe_ic_param_def.h" /* include peripheral declarations */
#include "mscc_poe_comm_protocol.h"
#include "mscc_poe_util.h"
#include "mscc_poe_cmd.h"


/*=========================================================================
/ LOCAL VARIABLES
/========================================================================*/
U16 CFGC_USRREG_BeforeSysConfig;

U16 wOriginalSysPowerBudget_dW = 0;
U16 wNewPowerBudget_dW = 0;


/*=========================================================================
/ LOCAL PROTOTYPES
/========================================================================*/
S32 mscc_poe_cmd_GetInterruptData(_IN U8 IC_Number,_IN U16 RegAddress,_OUT mscc_ProtocolInterrutRegister_t *pProtocolInterrutRegister);
S32 mscc_poe_cmd_Get_PORTS_CFG_from_a_single_IC (_IN U8 icIndex,_OUT PORT_CFG_t PORT_CFG[]);
S32 mscc_poe_cmd_Set_PORTS_CFG_to_a_single_IC(_IN U8 icIndex,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue);

/****************** Layer2 functions *******************/
U32 mscc_poe_cmd_PM_Check(_IN U8 ChNum);
S32 mscc_poe_cmd_BuildPriorityTable(void);
void mscc_poe_cmd_ConvertPDRequests(void);
void mscc_poe_cmd_LLDPTimeoutCounter(void);
U16 mscc_poe_cmd_Sqrt(_IN U16 val);


/*=========================================================================
/ FUNCTIONS
/========================================================================*/




/*---------------------------------------------------------------------
 *    description:     This function get Telemetry of all system ports, indicating their Enable/Disable status.
 *   				   Status – 1 = Enable; 0 = Disable.
 *					   the data is organized as EnDis bit per port.
 *
 *    input :   startVirtualPortNumber - the first Virtual Port Number which we should start build the Enable/Disable data
 *
 *    output:   bData            - pointer to bytes array which contain the ports Enable/Disable status.
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelsGroupEnDisState(_OUT U8 startVirtualPortNumber,_IN U8 *EnDisData)
{
    U8 Index;
    PORT_CFG_t PORT_CFG;

    *EnDisData = 0;

    /* Loop over 8 channels and gather the data */
    for(Index = 0; Index< 8 ;Index++)
    {
        result = mscc_POE_CMD_Get_Single_PORT_CFG(_IN ChannelParams[Index+startVirtualPortNumber].mscc_Vir2PhyArrActiveMatrix ,_OUT &PORT_CFG);
        if(result != e_POE_STATUS_OK) return result;

        if(PORT_CFG.Bits.PSE_Enable == 1)
            *EnDisData |= (1<<Index);
    }

    return result;
}




/*---------------------------------------------------------------------
 *    description:     This function build the latch byte.
 *
 *					    Latch structure: (Port latch) - Indicates that certain events have occurred: The latches are of the Clear-On-Read type.
 *						bit0 = 1 indicates an Underload latch condition
 *						bit1 = 1 indicates an Overload latch condition
 *						bit2 = 1 indicates a Force On current condition
 *						bit3, bit4 = indicate Underload (UDL) sticky counter
 *						bit5 =1 indicates short circuit condition.
 *						Bit6, 7 = indicate detection failure sticky counter
 *
 *    input :   physicalPortNumber  - number of port
 *
 *    output:   pLatchData          - pointer to latch data byte
 *
 *    return:   e_POE_STATUS_OK     - operation succeed
 * 			    != e_POE_STATUS_OK  - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelLatch(_IN U8 physicalPortNumber,_OUT U8 *pLatchData)
{
	U8 PhyIcNumber, PhyPortNumber;
	U16 PortIndications_RegData;
	U16 Port_CR_RegData;
	U16 PortUnderloadCnt_RegData;
	U16 PortInvalidSigCnt_RegData;

    *pLatchData = 0;

    /* calculate PhyIcNumber and PhyPortNumber */
    result = mscc_POE_IC_FUNC_Log2Phy(physicalPortNumber,&PhyIcNumber ,&PhyPortNumber);
    if(result != e_POE_STATUS_OK) return result;

    /* cleared the port using the indications clear sync event */
    result = mscc_IC_COMM_WriteDataToSpecificIC(PhyIcNumber,IndicationClearPortSelect_ADDR,PhyPortNumber);
    if(result != e_POE_STATUS_OK) return result;

    /* SetI2C_EXT_SYNC */
    result = mscc_IC_COMM_WriteDataToAllICs(I2C_ExtSyncType_ADDR,0x0008);
    if(result != e_POE_STATUS_OK) return result;

    /* EXT_EV_IRQ */
    result = mscc_IC_COMM_WriteDataToAllICs(EXT_EV_IRQ_ADDR,0x0008);
    if(result != e_POE_STATUS_OK) return result;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0Indications_ADDR,&PortIndications_RegData);
    if(result != e_POE_STATUS_OK) return result;

    Port_CR_RegData=0;
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_CR_ADDR ,&Port_CR_RegData);
    if(result != e_POE_STATUS_OK) return result;

    PortUnderloadCnt_RegData=0;
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0UnderloadCnt_ADDR ,&PortUnderloadCnt_RegData);
    if(result != e_POE_STATUS_OK) return result;

    PortInvalidSigCnt_RegData=0;
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0InvalidSigCnt_ADDR,&PortInvalidSigCnt_RegData);
    if(result != e_POE_STATUS_OK) return result;

    /* Build the Latch data byte*/
    *pLatchData =   PortIndications_RegData & BIT0 ;       /* Bit0-> 1 indicates an Underload latch condition */
    *pLatchData |=  ((PortIndications_RegData >>1) & BIT0) << 1 ;      /* Bit1-> 1 indicates an Overload latch condition */
    *pLatchData |=  ((((Port_CR_RegData & 3) == 2) ? 1 : 0) << 2 );               /* Bit2-> 1 indicates a Force On current condition */
    *pLatchData |=  (((PortUnderloadCnt_RegData > 3) ? 3 : PortUnderloadCnt_RegData) << 3 );   /* Bit3,4-> indicate Underload (UDL) sticky counter status */
    *pLatchData |=  ((PortIndications_RegData >>2) & 1) << 5;       /* Bit5-> 1 indicates short circuit condition */
    *pLatchData |=  (((PortInvalidSigCnt_RegData > 3) ? 3 :(U8) PortInvalidSigCnt_RegData) << 6 );  /* Bit6,7-> indicates detection failure sticky counter */

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This command gets the register mask value that enables each event of the interrupt function.
 *					   The Mask register bits define either masked or unmasked for the Interrupt register
 * 					   (0 = masked, 1 = unmasked)
 *
 *    input :   IC_Number                 - number of IC
 *
 *    output:   protocolInterrutRegister  - a pointer to structure of type ProtocolInterrutRegister_t which contain mask register bits
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    != e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetInterruptMask(_IN U8 IC_Number,_OUT mscc_ProtocolInterrutRegister_t *pProtocolInterrutRegister)
{
    (*pProtocolInterrutRegister).Word = 0;

    result = mscc_poe_cmd_GetInterruptData(IC_Number, IntOutEvEn_ADDR, pProtocolInterrutRegister);

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This command gets the interrupt register value.
 *					   Interrupt register latches a transition when an event occurs.
 * 					   The transition might be one or more of several port status changes,
 * 					   PoE device status event/s or system event/s, depending on event definition.
 *
 *    input :   IC_Number                - number of IC
 *
 *    output:   protocolInterrutRegister - a pointer to structure of type ProtocolInterrutRegister_t which contain interrupt register bits
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    != e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetInterruptRegister(_IN U8 IC_Number,_OUT mscc_ProtocolInterrutRegister_t *pProtocolInterrutRegister)
{
	U16 ActiveSlaveList_Reg;

    (*pProtocolInterrutRegister).Word = 0;

    result = mscc_poe_cmd_GetInterruptData(IC_Number, CFGC_EXTIE_ADDR, pProtocolInterrutRegister);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index, ActiveSlaveList_ADDR,&ActiveSlaveList_Reg);
    if(result != e_POE_STATUS_OK)
    	return result;

    (*pProtocolInterrutRegister).Bits.PoEDeviceFault = (((ActiveSlaveList_Reg &0xFF) ^ ((ActiveSlaveList_Reg>>8) & 0xFF)) != 0)?1:0;

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This command gets the interrupt register and interrupt mask data.
 *
 *    input :   IC_Number         - number of IC
 * 				RegAddress        - register address has two values : CFGC_EXTIE_ADDR - for interrupt register
 * 																      IntOutEvEn_ADDR - for mask register
 *
 *    output:   protocolInterrutRegister  - a pointer to structure of type ProtocolInterrutRegister_t which contain interrupt register bits
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    != e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_poe_cmd_GetInterruptData(_IN U8 IC_Number,_IN U16 RegAddress,_OUT mscc_ProtocolInterrutRegister_t *protocolInterrutRegister)
{
    mscc_IntOutEvEn_t intOutEvEn;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,RegAddress,&intOutEvEn.Word);
    if(result != e_POE_STATUS_OK)
    	return result;

    (*protocolInterrutRegister).Bits.PortTurnedOn = intOutEvEn.Bits.PortOnMask;
    (*protocolInterrutRegister).Bits.PortTurnedOff = intOutEvEn.Bits.PortOffMask;
    (*protocolInterrutRegister).Bits.DetectionUnsuccessful = intOutEvEn.Bits.DetectionFailedMask ;
    (*protocolInterrutRegister).Bits.PortFault = intOutEvEn.Bits.PortFaultMask ;
    (*protocolInterrutRegister).Bits.PortWasInUnderLoad = intOutEvEn.Bits.PortUDLMask  ;
    (*protocolInterrutRegister).Bits.PortWasInOverLoad = intOutEvEn.Bits.PortFaultDuringSU ;
    (*protocolInterrutRegister).Bits.PortWasInPM = intOutEvEn.Bits.PortPMMask ;
    (*protocolInterrutRegister).Bits.PortSpareEvent = 0;
    (*protocolInterrutRegister).Bits.DisconnectionTemperature = intOutEvEn.Bits.OverTempMask ;
    (*protocolInterrutRegister).Bits.UserDefinedTemperature = intOutEvEn.Bits.TempAlarmMask ;
    (*protocolInterrutRegister).Bits.PoEDeviceFault = 0;
    (*protocolInterrutRegister).Bits.PoEDeviceSpareEvent = 0;
    (*protocolInterrutRegister).Bits.NoMoreConnect = intOutEvEn.Bits.PowerDeniedMask;
    (*protocolInterrutRegister).Bits.VmainFault =  intOutEvEn.Bits.VmainHighMask | intOutEvEn.Bits.VmainLowATMask | intOutEvEn.Bits.VmainLowAFMask;
    (*protocolInterrutRegister).Bits.SystemSpareEvent1 = 0;
    (*protocolInterrutRegister).Bits.SystemSpareEvent2 = 0;

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This command sets the interrupt mask.
 *
 *    input :   InterruptMask_Data - Interrupt Mask Data
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    != e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetInterruptMask(_IN U16 InterruptMask_Data)
{
    mscc_ProtocolInterrutRegister_t protocolInterrutRegister;

    protocolInterrutRegister.Word = InterruptMask_Data;

    mscc_IntOutEvEn_t intOutEvEn;
    intOutEvEn.Word = 0;

    intOutEvEn.Bits.PortOnMask = protocolInterrutRegister.Bits.PortTurnedOn;
    intOutEvEn.Bits.PortOffMask = protocolInterrutRegister.Bits.PortTurnedOff ;
    intOutEvEn.Bits.DetectionFailedMask = protocolInterrutRegister.Bits.DetectionUnsuccessful;
    intOutEvEn.Bits.PortFaultMask =protocolInterrutRegister.Bits.PortFault;
    intOutEvEn.Bits.PortUDLMask = protocolInterrutRegister.Bits.PortWasInUnderLoad;
    intOutEvEn.Bits.PortFaultDuringSU = protocolInterrutRegister.Bits.PortWasInOverLoad;
    intOutEvEn.Bits.PortPMMask = protocolInterrutRegister.Bits.PortWasInPM;
    intOutEvEn.Bits.PowerDeniedMask = protocolInterrutRegister.Bits.NoMoreConnect;
    intOutEvEn.Bits.OverTempMask = protocolInterrutRegister.Bits.DisconnectionTemperature;
    intOutEvEn.Bits.TempAlarmMask = protocolInterrutRegister.Bits.UserDefinedTemperature;
    intOutEvEn.Bits.VmainLowAFMask = protocolInterrutRegister.Bits.VmainFault;
    intOutEvEn.Bits.VmainLowATMask = protocolInterrutRegister.Bits.VmainFault;
    intOutEvEn.Bits.VmainHighMask = protocolInterrutRegister.Bits.VmainFault;
    intOutEvEn.Bits.RPREventMask = 0;

    result = mscc_IC_COMM_WriteDataToAllICs(IntOutEvEn_ADDR,intOutEvEn.Word);

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This command Set single bit in SysFlags Register Data
 *
 *    input :   bitIndex          - the index of the bit to modify
 * 				dataEnDis         - the data of the bit (1 or 0)
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    != e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetSingleBitInSysFlags(_IN U8 bitIndex, _IN U8 dataEnDis)
{
	U16 SysFlags_RegData;

    /* read System Flags register data */
    result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index ,SysFlags_ADDR,&SysFlags_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* reset bit Index */
    SysFlags_RegData &=  ~(1 << bitIndex);
    /* set bit Index Data  */
    SysFlags_RegData |= (dataEnDis << bitIndex);

    /* send registers SysFlags modified data */
    result = mscc_POE_CMD_SetSysFlags(_IN SysFlags_RegData);

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This command Set SysFlags Register Data
 *
 *    input :   SysFlags_RegData          - register data
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK               - operation succeed
 * 			    != e_POE_STATUS_OK            - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetSysFlags(_IN U16 SysFlags_RegData)
{
    result = mscc_SendPreSysConfigCommands();
    if(result != e_POE_STATUS_OK)
    	goto LableSendPostSysConfigCommands;

    result = mscc_IC_COMM_WriteDataToAllICs(SysFlags_ADDR,SysFlags_RegData);

LableSendPostSysConfigCommands:

    tmpResult = mscc_SendPostSysConfigCommands();
    UPDATE_RESULT(result,tmpResult);

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This command sets the IC to config mode 	from automode
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK               - operation succeed
 * 			    != e_POE_STATUS_OK            - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_SendPreSysConfigCommands(void)
{
    /* SetDisPortsCmd */
    result = mscc_IC_COMM_WriteDataToAllICs(DisPortsCmd_ADDR,0xFFFF);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = OS_Sleep_mS(30);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* SetSW_MODE */
    result = mscc_IC_COMM_WriteDataToAllICs(SW_ConfigReg_ADDR,0xDC03);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* SetI2C_EXT_SYNC */
    result = mscc_IC_COMM_WriteDataToAllICs(I2C_ExtSyncType_ADDR,0x0020);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* EXT_EV_IRQ */
    result = mscc_IC_COMM_WriteDataToAllICs(EXT_EV_IRQ_ADDR,0x0020);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = OS_Sleep_mS(30);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index ,CFGC_USRREG_ADDR,&CFGC_USRREG_BeforeSysConfig);
    return result;
}




/*---------------------------------------------------------------------
 *    description:     This command sets the IC to automode	from config mode
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK               - operation succeed
 * 			    != e_POE_STATUS_OK            - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_SendPostSysConfigCommands(void)
{

    /* SetSW_MODE */
    result = mscc_IC_COMM_WriteDataToAllICs(SW_ConfigReg_ADDR,0xDC00);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* SetI2C_EXT_SYNC */
    result = mscc_IC_COMM_WriteDataToAllICs(I2C_ExtSyncType_ADDR,0x0020);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* EXT_EV_IRQ */
    result = mscc_IC_COMM_WriteDataToAllICs(EXT_EV_IRQ_ADDR,0x0020);
    if(result != e_POE_STATUS_OK)
    	return result;

    tmpResult = OS_Sleep_mS(30);
    UPDATE_RESULT(result,tmpResult);

    /* SetDisPortsCmd */
    tmpResult = mscc_IC_COMM_WriteDataToAllICs(DisPortsCmd_ADDR,0x0000);
    UPDATE_RESULT(result,tmpResult);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = mscc_IC_COMM_WriteDataToAllICs(CFGC_USRREG_ADDR,CFGC_USRREG_BeforeSysConfig);
    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function returns the power of a channel in deciwatts
 *
 *    input :   physicalPortNumber - port number
 *
 *    output:   PortPowerCons_mW   - real port consuption from IC
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    != e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelPower(_IN U8 physicalPortNumber,_OUT U16 *pPortPowerCons_mW)
{
	U16 PortPowerCons_RegData;

	*pPortPowerCons_mW = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0PowerCons_ADDR, &PortPowerCons_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pPortPowerCons_mW = mscc_POE_UTIL_ConvertICValueTo_mw(PortPowerCons_RegData);

    return result;
}




/*---------------------------------------------------------------------
 *    description:     This function returns the actual port status	and port type - AF/AT
 *
 *    input :   physicalPortNumber  - port number
 *
 *    output:   pCustomerPortStatus  - actual port status as defined in communication protocol
 * 			    bIsPortOn            - True : when port is ON , False : when port is OFF
 *              AtType               - actual port type
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelStatus (_IN U8 physicalPortNumber,_OUT U8 *pCustomerPortStatus,_OUT U8 *bIsPortOn, _OUT U8 *AtType)
{
	U16 InternalPortStatus_RegData;
	U16 PortLastDisconnect_RegData;
	InternalPortStatus_e PortLastDisconnectStatus,InternalPortStatusData, portStatus;
	ExternalPortStatus_e ExternalPortStatus;

    *pCustomerPortStatus = e_CustomerUnKnownICStatus;
    *AtType = 0;
    *bIsPortOn = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_SR_ADDR ,&InternalPortStatus_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0LastDisconnect_ADDR ,&PortLastDisconnect_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    PortLastDisconnectStatus = PortLastDisconnect_RegData;

    ExternalPortStatus = ((InternalPortStatus_RegData >> 8) & 7);
    *bIsPortOn = (ExternalPortStatus == e_ExternalPortStatus_DeliveringPower)?POE_TRUE:POE_FALSE;

    InternalPortStatusData =(InternalPortStatus_e) ((U8) InternalPortStatus_RegData);
    *AtType = ((InternalPortStatus_RegData >> 11)&1);

    portStatus = InternalPortStatusData;
    if((PortLastDisconnectStatus > e_InternalPortStatus_Disabled )&& ((InternalPortStatusData ==  e_InternalPortStatus_Startup)
                                                                     || (InternalPortStatusData ==  e_InternalPortStatus_StartupTM)
                                                                     || (InternalPortStatusData ==  e_InternalPortStatus_Searching)
                                                                     || (InternalPortStatusData ==  e_InternalPortStatus_TestMode)
                                                                     || (InternalPortStatusData ==  e_InternalPortStatus_ValidSig)))
    portStatus = PortLastDisconnectStatus;

    switch(portStatus)
    {
    	case e_InternalPortStatus_On:    /* 0 -  PortOn - Port was turned on due to a valid signature (res \ cap) */
        {
            *pCustomerPortStatus = e_CustomerChannelOKRes;   /* Valid PD Resistor Signature Detected */
            break;
        }

    	case e_InternalPortStatus_PortOnTM:           /* 1 -  PortOnTM - Port was turned on due to Force Power */
        {
            *pCustomerPortStatus = e_CustomerForcePowerON;
            break;
        }

    	case e_InternalPortStatus_Startup:            /* 2 -  Startup - Port is in startup */
        {
            *pCustomerPortStatus = e_CustomerInit;
            break;
        }

    	case e_InternalPortStatus_StartupTM:         /* 3 -  StartupTM - Port is in startup as force power */
        {
            *pCustomerPortStatus = e_CustomerInit;
            break;
        }

    	case e_InternalPortStatus_Searching:         /* 4 -  Searching - Port needs detection or during detection */
        {
            *pCustomerPortStatus = e_CustomerInit;
            break;
        }

    	case e_InternalPortStatus_InvalidSig:         /* 5 -  InvalidSig - Invalid signature has been detected */
        {
            *pCustomerPortStatus = e_CustomerNotRes;
            break;
        }

    	case e_InternalPortStatus_ClassError:         /* 6 -  ClassError - Error in classification has been detected */
        {
            *pCustomerPortStatus = e_CustomerClassError;
            break;
        }

    	case e_InternalPortStatus_TestMode:          /* 7 -  TestMode - Port needs to be turned on as Test Mode Force Power */
        {
            *pCustomerPortStatus = e_CustomerInit;
            break;
        }

    	case e_InternalPortStatus_ValidSig:           /* 8 -  ValidSig - A valid signature has been detected */
        {
            *pCustomerPortStatus = e_CustomerInit;
            break;
        }

    	case e_InternalPortStatus_Disabled:          /* 9 -  Disabled -  Port is disabled */
        {
            *pCustomerPortStatus = e_CustomerChannelOFF;
            break;
        }

    	case e_InternalPortStatus_StartupOVL:         /* 10 -  StartupOVL - Overload during startup */
        {
            *pCustomerPortStatus = e_CustomerOVL;
            break;
        }

    	case e_InternalPortStatus_StartupUDL:        /* 11 - StartupUDL - Underload during startup */
        {
            *pCustomerPortStatus = e_CustomerUDL;
            break;
        }

    	case e_InternalPortStatus_StartupShort:       /* 12 - StartupShort - Short during startup */
        {
            *pCustomerPortStatus = e_CustomerShortCircuit;
            break;
        }

    	case e_InternalPortStatus_DvDtFail:          /* 13 - DvDtFail - Failure in the Dv/Dt algorithm */
        {
            *pCustomerPortStatus = e_CustomerOVL;
            break;
        }

    	case e_InternalPortStatus_TestError:         /* 14 - TestError - Port was turned on as Test Mode and has error */
        {
            *pCustomerPortStatus = e_CustomerForcePowerError;
            break;
        }

    	case e_InternalPortStatus_OVL:               /* 15 - OVL - Overload detected */
        {
            *pCustomerPortStatus = e_CustomerOVL;
            break;
        }

    	case e_InternalPortStatus_UDL:               /* 16 - UDL - Underload detected */
        {
            *pCustomerPortStatus = e_CustomerUDL;
            break;
        }

    	case e_InternalPortStatus_ShortCircuit:      /* 17 - ShortCircuit - Short circuit detected */
        {
            *pCustomerPortStatus = e_CustomerShortCircuit;
            break;
        }

    case e_InternalPortStatus_PM:                /* 18 - PM  port was turned off due to PM */
        {
            *pCustomerPortStatus = e_CustomerPowerManagement;
            break;
        }

    case e_InternalPortStatus_SysDisabled:        /* 19 - SysDisabled - Chip level error */
        {
            *pCustomerPortStatus = e_CustomerFPGA_Error;
            break;
        }

    case e_InternalPortStatus_Unknown:            /* 20 - Unknown - General chip error */
        {
            *pCustomerPortStatus = e_CustomerUnKnownICStatus;
            break;
        }

    default:
        {
            *pCustomerPortStatus = e_CustomerUnDefined;
            break;
        }
    }

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This function returns the actual ports class
 *
 *    input :   physicalPortNumber - port number
 *
 *    output:   Port_Class_Data    - actual port class
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelClass(_IN U8 physicalPortNumber,_OUT U8 *pPort_Class_Data)
{
	U16 PortClass_RegData;

	*pPort_Class_Data = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_Class_ADDR,&PortClass_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* decided class */
    *pPort_Class_Data = (U8)(PortClass_RegData >> 8);

    if((*pPort_Class_Data) > e_Class4)
    	*pPort_Class_Data = e_Class0;

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function returns the actual Port Power Limit
 *
 *    input :   physicalPortNumber - port number
 *
 *    output:   pPPL_Data_mW       - Port Power Limit
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelPPL(_IN U8 physicalPortNumber,_OUT U16 *pPPL_Data_mW)
{
	 U16 PPL_Data_RegData;

	*pPPL_Data_mW = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_PPL_ADDR,&PPL_Data_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* convert from deciWatt to mW */
    *pPPL_Data_mW = PPL_Data_RegData * 100;

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This function sets the actual Port Power Limit
 *
 *    input :   physicalPortNumber - port number
 * 				PPL_Data_mW        - Port Power Limit [mili Watts]
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetChannelPPL(_IN U8 physicalPortNumber,_IN U16 PPL_Data_mW)
{
	U16 PPL_Data_RegData;

    /* convert from mWatt to deciWatt */
    PPL_Data_RegData = (U16)(PPL_Data_mW / 100);

    result = mscc_IC_COMM_WriteDataToSpecificPort(physicalPortNumber,Port0_PPL_ADDR ,PPL_Data_RegData);

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This function returns the actual Temporary Port Power Limit
 *
 *    input :   physicalPortNumber  - port number
 *
 *    output:   pTPPL_Data_mW       - Temporary Port Power Limit [mili Watts]
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelTPPL(_IN U8 physicalPortNumber,_OUT U16 *pTPPL_Data_mW)
{
	U16 TPPL_Data_RegData;

	*pTPPL_Data_mW = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_TPPL_ADDR ,&TPPL_Data_RegData);
    if(result != e_POE_STATUS_OK) return result;

    /* convert from deciWatt to miliWatt */
    *pTPPL_Data_mW = TPPL_Data_RegData * 100;

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This function sets the actual Temporary Port Power Limit
 *
 *    input :   physicalPortNumber - port number
 * 				TPPL_Data_mW       - Temporary Port Power Limit [mili Watts]
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetChannelTPPL(_IN U8 physicalPortNumber,_IN U16 TPPL_Data_mW)
{
	 U16 TPPL_Data_RegData;

    /* convert from mW to deciW */
    TPPL_Data_RegData  = (U16) (TPPL_Data_mW / 100);

    result = mscc_IC_COMM_WriteDataToSpecificPort(physicalPortNumber,Port0_TPPL_ADDR ,TPPL_Data_RegData);

    return result;
}




/*---------------------------------------------------------------------
 *    description:      This function sets the Port power limits for the all ports in the system
 *
 *    input :   PPL_Data_mW         - Port Power Limit ,units in mW
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetAllChannelPPL(_IN U16 PPL_Data_mW)
{
	U16 PPL_Data_RegData;
	U8 icIndex;
	U16 NumberOFBytesToWrite;
	U8 chIndex;

	U8 dataArr[MAX_CH_PER_IC*2];

    /* convert from mWatt to deciWatt */
    PPL_Data_RegData = (U16)(PPL_Data_mW / 100);

    for(icIndex = 0; icIndex < SystemParams.NumOfActiveICsInSystem; icIndex++)
    {
        NumberOFBytesToWrite = IcParams[icIndex].NumOfChannesInIC*2;

        for(chIndex = 0; chIndex <= IcParams[icIndex].NumOfChannesInIC; chIndex++)
        {
            dataArr[chIndex * 2] = mscc_GetFirstByte(PPL_Data_RegData);
            dataArr[(chIndex * 2) + 1] = mscc_GetSecondByte(PPL_Data_RegData);
        }

        result = mscc_IC_COMM_I2C_Write(_IN IcParams[icIndex].IC_Address,_IN Port0_PPL_ADDR,  _IN dataArr, _IN NumberOFBytesToWrite);
        if(result != e_POE_STATUS_OK) return result;
    }

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function sets a single port configuration to force power mode
 *
 *    input :   physicalPortNumber - port number
 * 				Cmd                - 00 - Port Enabled , 10 - Force Power
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetForcePower(_IN U8 physicalPortNumber,U16 Cmd)
{
	U8 ForcePower;

    if(Cmd > 1)
        return(FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(36));

    if(Cmd == 0)
        ForcePower = 0;
    else
        ForcePower = 2;

    result = mscc_POE_CMD_Set_Single_PORT_CFG(physicalPortNumber,POE_TRUE,POE_FALSE,POE_FALSE,POE_FALSE,ForcePower,0,0,0);

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This function returns the port's Power Information
 *
 *    input :   physicalPortNumber  - port number
 *
 *    output:	PowerInfo_t         - a structure that contain port's Power Information :
 * 										chCurrent - port's calculated current
 *	 									chPower   - port's Power Consumption
 *	 									chVolt    - port's Voltage
 *										MainVolt  - Vmain Voltage
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelPowerInfo(_IN U8 physicalPortNumber,_OUT PowerInfo_t *pPowerInfo)
{
    U16 TmpRegData;
    U8 PhyIcNumber, PhyPortNumber;
    U16 Vmain_RegData;

    /* Vmain Voltage */
    result = mscc_POE_IC_FUNC_Log2Phy(physicalPortNumber,&PhyIcNumber ,&PhyPortNumber);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(PhyIcNumber,VMAIN_ADDR,&Vmain_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    pPowerInfo->MainVolt = mscc_POE_UTIL_ConvertICValueToDeciVolt(Vmain_RegData);

    /* Calculated Current */
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,P0_ISENSE_ADDR,&TmpRegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    pPowerInfo->chCurrent = mscc_POE_UTIL_ConvertICValueTo_ma(TmpRegData);

    /* Power Consumption */
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber, Port0PowerCons_ADDR,&TmpRegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    pPowerInfo->chPower = mscc_POE_UTIL_ConvertICValueTo_mw(TmpRegData);

    /* Port Voltage */
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,VPM_VPORT_0_MES_ADDR,&TmpRegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    pPowerInfo->chVolt = mscc_POE_UTIL_ConvertPortValueToDeciVolt(TmpRegData);

    return result;
}


/*---------------------------------------------------------------------
 *    description:     This function returns the single port's counter value (0 to 3)
 * 					   port's counter value is ceiling to 3.
 * 					   available counters are: PortInvalidSigCnt, PortPowerDeniedCnt ,
 * 					   PortOverloadCnt,PortUnderloadCnt, PortShortCnt,PortClassErrorCnt
 *
 *    input :   physicalPortNumber  - port number
 * 				Port0Cnt_ADDR		- the address of selected port's counter.
 *
 *    output:	pCounterValue        - port's counter value (0 to 3)
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetChannelCounter(_IN U8 physicalPortNumber,_IN U16 PortCnt_ADDR,_OUT U8 *pCounterValue)
{
	U16 PortCounterValue_Data;

    *pCounterValue = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,PortCnt_ADDR ,&PortCounterValue_Data);
    if(result != e_POE_STATUS_OK) return result;

    *pCounterValue = (PortCounterValue_Data > 3 ) ? 3 :PortCounterValue_Data;

    return result;
}




/*---------------------------------------------------------------------
 *    description:     This function returns the Actual momentary input line voltage (VMAIN Voltage)
 *
 *    input :   IC_Number             - The IC number
 *
 *    output:   pVMAIN_Data_deciVolts - Actual momentary input line voltage
 *
 *    return:   e_POE_STATUS_OK          - operation succeed
 * 			    != e_POE_STATUS_OK       - operation failed
 *---------------------------------------------------------------------*/

S32 mscc_POE_CMD_GetVmain(_IN U8 IC_Number,_OUT U16 *pVMAIN_Data_deciVolts)
{
	U16 Vmain_RegData;

	*pVMAIN_Data_deciVolts = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,VMAIN_ADDR,&Vmain_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pVMAIN_Data_deciVolts = mscc_POE_UTIL_ConvertICValueToDeciVolt(Vmain_RegData);

    return result;
}




/*---------------------------------------------------------------------
 *    description:     This function sets Vmain High,Vmain AT Low and Vmain AF Low Thersholds
 *
 *    input :   VmainHighThershold_Data_deciVolts - Vmain High Thershold in deciVolts units
 * 				VmainLowThershold_Data_deciVolts  - Vmain Low Thershold in deciVolts units
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetVmainThersholds(_IN U16 VmainHighThershold_Data_deciVolts,_IN U16 VmainLowThershold_Data_deciVolts)
{
	U16 VmainHighThershold_RegData;
	U16 VmainLowThershold_RegData;

    VmainHighThershold_RegData= mscc_POE_UTIL_ConvertDeciVoltToICValue(VmainHighThershold_Data_deciVolts);
    VmainLowThershold_RegData = mscc_POE_UTIL_ConvertDeciVoltToICValue(VmainLowThershold_Data_deciVolts);

    /* write enable key */
    result = mscc_IC_COMM_WriteDataToAllICs(CFGC_PROTKEY_ADDR,0xAB);
    if(result != e_POE_STATUS_OK)
    	goto EnablePROTKEY;

    /* write Vmain High Thershold Data */
    result = mscc_IC_COMM_WriteDataToAllICs(VmainHighTh_ADDR,VmainHighThershold_RegData);
    if(result != e_POE_STATUS_OK)
    	goto EnablePROTKEY;

    /* write Vmain AT Low Thershold Data */
    result = mscc_IC_COMM_WriteDataToAllICs(VmainAtLowTh_ADDR,VmainLowThershold_RegData);
    if(result != e_POE_STATUS_OK)
    	goto EnablePROTKEY;

    /* write Vmain AF Low Thershold Data */
    result = mscc_IC_COMM_WriteDataToAllICs(VmainAfLowTh_ADDR,VmainLowThershold_RegData);

EnablePROTKEY:

    /* write disable key */
    tmpResult = mscc_IC_COMM_WriteDataToAllICs(CFGC_PROTKEY_ADDR,0);
    UPDATE_RESULT(result,tmpResult);

    return result;
}




/*---------------------------------------------------------------------
 *    description:     This function returns the Vmain High Thershold
 *
 *    input :   IC_Number
 *
 *    output:   pVmainHighThershold_Data_deciVolts - Vmain High Thershold in deciVolts units
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetVmainHighThershold(_IN U8 IC_Number,_OUT U16 *pVmainHighThershold_Data_deciVolts)
{
	U16 VmainHighThershold_RegData;

	*pVmainHighThershold_Data_deciVolts = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,VmainHighTh_ADDR,&VmainHighThershold_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pVmainHighThershold_Data_deciVolts = mscc_POE_UTIL_ConvertICValueToDeciVolt(VmainHighThershold_RegData);

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function returns the Vmain AT Low Thershold
 *
 *    input :   IC_Number
 *
 *    output:   pVmainLowThershold_Data_deciVolts - Vmain AT Low Thershold in deciVolts units
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetVmainLowThershold(_IN U8 IC_Number,_OUT U16 *pVmainLowThershold_Data_deciVolts)
{
	U16 VmainLowThershold_RegData;

	*pVmainLowThershold_Data_deciVolts = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,VmainAtLowTh_ADDR,&VmainLowThershold_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pVmainLowThershold_Data_deciVolts = mscc_POE_UTIL_ConvertICValueToDeciVolt(VmainLowThershold_RegData);

    return result;
}




/*---------------------------------------------------------------------
 *    description:     This function returns the IC Configuration
 *
 *    input :   IC_Number
 *
 *    output:   pIs_Auto_mode_master_capabilities - Auto mode master capabilities - Auto mode or enhanced mode
 * 				pStandardMode_e                   - High power capabilities -  AT enabled or disabled
 * 				pIs_support_12_ports              - support 8/12 ports
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetICConfiguration(_IN U8 IC_Number,_OUT U8 *pIs_Auto_mode_master_capabilities,_OUT mscc_StandardMode_e *pStandardMode_e,_OUT U8 *pIs_support_12_ports)
{
    /*
    bit 2 - Auto mode master capabilities - bit 2 	: 0 - Auto mode 1 - enhanced only
    bit 3 - High power capabilities - bit 3 		: 0 - AT enabled 1 - AT disabled
    bit 4 - 8/12 port support - bit 4 				: 0 - 12 port support 1 - 8 port support
    */

    U16 FUSE_ARR2_Data;
    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,FUSE_ARR2_ADDR,&FUSE_ARR2_Data);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pIs_Auto_mode_master_capabilities = (((FUSE_ARR2_Data >> 2)&1)==1)?0:1;
    *pStandardMode_e = (((FUSE_ARR2_Data >> 3)&1)==1)?0:1;
    *pIs_support_12_ports = (((FUSE_ARR2_Data >> 4)&1)==1)?0:1;

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function returns the Active Bank
 *
 *    input :   IC_Number
 *
 *    output:   pActiveBank
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetActiveBank(_IN U8 IC_Number,_OUT U8 *pActiveBank)
{
	U16 CFGC_PWRGD_ADDR_regData;

	*pActiveBank = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,CFGC_PWRGD_ADDR,&CFGC_PWRGD_ADDR_regData);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pActiveBank = (U8)( CFGC_PWRGD_ADDR_regData & 7);  /* mask 3 first power good bits */
    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function sets the Temperature Alarm Thershold
 *
 *    input :   IC_Number
 * 				TemperatureAlarm_celsius  - the user defined temperature alarm thershold
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_SetTemperatureAlarmThershold(_IN U8 IC_Number,_IN U8 TemperatureAlarm_celsius)
{
	U16 ICUserTemperature_RegData;

	/* write enable key */
    result = mscc_IC_COMM_WriteDataToSpecificIC(IC_Number,CFGC_PROTKEY_ADDR,0xAB);
    if(result != e_POE_STATUS_OK)
    	goto EnablePROTKEY2;

    ICUserTemperature_RegData = mscc_POE_UTIL_CelziusToICValue(TemperatureAlarm_celsius); /*Convert Form Celzios To IC Value */
    result = mscc_IC_COMM_WriteDataToSpecificIC(IC_Number,TempAlarmTh_ADDR,ICUserTemperature_RegData);

EnablePROTKEY2:

    /* write disable key */
    tmpResult = mscc_IC_COMM_WriteDataToSpecificIC(IC_Number,CFGC_PROTKEY_ADDR,0);
    UPDATE_RESULT(result,tmpResult);

    return result;
}



/*---------------------------------------------------------------------
 *    description:     This function returns the Temperature Alarm Thershold
 *
 *    input :   IC_Number
 *
 *    output:   pTemperatureAlarm_celsius - the user defined temperature alarm thershold
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_GetTemperatureAlarmThershold (_IN U8 IC_Number,_OUT U8 *pTemperatureAlarm_celsius)
{
	U16 ICUserTemperature_RegData;

	*pTemperatureAlarm_celsius = 0;

    result = mscc_IC_COMM_ReadDataFromSpecificIC(IC_Number,TempAlarmTh_ADDR,&ICUserTemperature_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    *pTemperatureAlarm_celsius = (U16)mscc_POE_UTIL_ICValueToCelzius(ICUserTemperature_RegData); /* Convert Form Celzios To IC Value */

    return result;
}



/*--------------------------------------------------------------------------------------------------------------------------
 *    description:      This function sets the control register fields for a single port. each field has boolean flag
 *                      variable that mention whether to write the data variable to the specific field.
 *
 *    input :   physicalPortNumber  - port number
 * 				bPSE_Enable         - Port Enable Status flag      POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPairControl        - Port Pair Control flag       POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortMode           - Port Type Definition flag    POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortPriority       - Port Priority Level flag     POE_TRUE - change data , POE_FALSE - don't change data
 * 				PSE_EnableValue		- Port Enable Status data      00 - Port Disabled , 01 - Port Enabled
 * 				PairControlValue    - Port Pair Control data       01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 				PortModeValue       - Port Type Definition data    00 - af.    01 - at.
 * 				PortPriorityValue	- Port Priority Level data	   00 - Critical , 01 - High , 10 - Low
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *----------------------------------------------------------------------------------------------------------------------------*/
S32 mscc_POE_CMD_Set_Single_PORT_CFG(_IN U8 physicalPortNumber,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue)
{
    U16 Port_CR_RegData;
    PORT_CFG_t PORT_CFG;
    U8 PhyIcNumber, PhyPortNumber;
    U16 MaskBit;
    U16 Port_CR_Modified_RegData;

    /* read actual port's configuration */
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_CR_ADDR,&Port_CR_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* build PORT_CFG struct */
    PORT_CFG.Byte = (U8) Port_CR_RegData;

    /* Modify PORT_CFG struct with new data */
    if(bPSE_Enable == POE_TRUE)
        PORT_CFG.Bits.PSE_Enable = PSE_EnableValue;
    if(bPairControl == POE_TRUE)
        PORT_CFG.Bits.PairControl = PairControlValue;
    if(bPortMode == POE_TRUE)
        PORT_CFG.Bits.PortMode = PortModeValue;
    if(bPortPriority == POE_TRUE)
        PORT_CFG.Bits.PortPriority = PortPriorityValue;

    /* calculate PhyIcNumber and PhyPortNumber in order to disable the port */
    result = mscc_POE_IC_FUNC_Log2Phy(physicalPortNumber,&PhyIcNumber ,&PhyPortNumber);
    if(result != e_POE_STATUS_OK)
    	return result;

    MaskBit = 1<<PhyPortNumber;

    /* SetDisPortsCmd - disable single port */
    result = mscc_IC_COMM_WriteDataToSpecificIC(PhyIcNumber,DisPortsCmd_ADDR,MaskBit);
    if(result != e_POE_STATUS_OK)
    	goto LableEnableSinglePort;

    result = OS_Sleep_mS(30);
    if(result != e_POE_STATUS_OK)
    	goto LableEnableSinglePort;

    /* costruct the new modified port configuration data */
    Port_CR_Modified_RegData =  PORT_CFG.Bits.PSE_Enable | (PORT_CFG.Bits.PairControl << 2) |
                                    (PORT_CFG.Bits.PortMode << 4) | (PORT_CFG.Bits.PortPriority << 6 );

    /* set the new modified port configuration data to the chip */
    tmpResult = mscc_IC_COMM_WriteDataToSpecificPort(physicalPortNumber,Port0_CR_ADDR,Port_CR_Modified_RegData);
    UPDATE_RESULT(result, tmpResult);

 LableEnableSinglePort:


    /* disable the SetDisPortsCmd which enables all ports */
    tmpResult = mscc_IC_COMM_WriteDataToSpecificIC(PhyIcNumber,DisPortsCmd_ADDR,0);
    UPDATE_RESULT(result,tmpResult);
    if(result != e_POE_STATUS_OK)
        return result;

    result = OS_Sleep_mS(30);
    return result;
}











/*--------------------------------------------------------------------------------------------------------------------------
 *    description:      This function sets the control register fields for a single port. each field has boolean flag
 *                      variable that mention whether to write the data variable to the specific field.
 *
 *    input :   physicalPortNumber  - port number
 * 				bPSE_Enable         - Port Enable Status flag      POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPairControl        - Port Pair Control flag       POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortMode           - Port Type Definition flag    POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortPriority       - Port Priority Level flag     POE_TRUE - change data , POE_FALSE - don't change data
 * 				PSE_EnableValue		- Port Enable Status data      00 - Port Disabled , 01 - Port Enabled
 * 				PairControlValue    - Port Pair Control data       01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 				PortModeValue       - Port Type Definition data    00 - af.    01 - at.
 * 				PortPriorityValue	- Port Priority Level data	   00 - Critical , 01 - High , 10 - Low
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *----------------------------------------------------------------------------------------------------------------------------*/
S32 mscc_POE_CMD_Set_Single_PORT_CFG_without_disable_port(_IN U8 physicalPortNumber,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue)
{
    U16 Port_CR_RegData;
    PORT_CFG_t PORT_CFG;
    U16 Port_CR_Modified_RegData;

    /* read actual port's configuration */
    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_CR_ADDR,&Port_CR_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* build PORT_CFG struct */
    PORT_CFG.Byte = (U8) Port_CR_RegData;

    /* Modify PORT_CFG struct with new data */
    if(bPSE_Enable == POE_TRUE)
        PORT_CFG.Bits.PSE_Enable = PSE_EnableValue;
    if(bPairControl == POE_TRUE)
        PORT_CFG.Bits.PairControl = PairControlValue;
    if(bPortMode == POE_TRUE)
        PORT_CFG.Bits.PortMode = PortModeValue;
    if(bPortPriority == POE_TRUE)
        PORT_CFG.Bits.PortPriority = PortPriorityValue;

    /* costruct the new modified port configuration data */
    Port_CR_Modified_RegData =  PORT_CFG.Bits.PSE_Enable | (PORT_CFG.Bits.PairControl << 2) |
                                    (PORT_CFG.Bits.PortMode << 4) | (PORT_CFG.Bits.PortPriority << 6 );

    /* set the new modified port configuration data to the chip */
    result = mscc_IC_COMM_WriteDataToSpecificPort(physicalPortNumber,Port0_CR_ADDR,Port_CR_Modified_RegData);
    return result;
}






/*---------------------------------------------------------------------
 *    description:      This function returns the control register fields from a single port.
 * 							PSE_Enable	: 2;
 *							PairControl	: 2;
 *							PortMode 	: 2;
 *							PortPriority : 2;
 *
 *    input :   physicalPortNumber - port number
 *
 *    output:   PORT_CFG_t       - structure that contain the ports control register fields:
 * 								    PSE_EnableValue		- Port Enable Status data    00 - Port Disabled , 01 - Port Enabled
 * 									PairControlValue    - Port Pair Control data     01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 									PortModeValue       - Port Type Definition data  00 - af.    01 - at.
 * 									PortPriorityValue	- Port Priority Level data	 00 - Critical , 01 - High , 10 - Low
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_Get_Single_PORT_CFG(_IN U8 physicalPortNumber,_OUT PORT_CFG_t *pPORT_CFG)
{
    U16 Port_CR_RegData;

    result = mscc_IC_COMM_ReadDataFromSpecificPort(physicalPortNumber,Port0_CR_ADDR,&Port_CR_RegData);
    if(result != e_POE_STATUS_OK)
    	return result;

    pPORT_CFG->Byte = (U8) Port_CR_RegData;
    return result;
}


/*---------------------------------------------------------------------
 *    description:      This function sets the control register fields for the all ports that contains in the selected ICs
 * 						(all the ICs between startIcIndex to stopIcIndex).
 * 						each field has boolean flag variable that mention whether to write the data variable to the specific field.
 *
 *    input :	bPSE_Enable         - Port Enable Status flag      POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPairControl        - Port Pair Control flag       POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortMode           - Port Type Definition flag    POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortPriority       - Port Priority Level flag     POE_TRUE - change data , POE_FALSE - don't change data
 * 				PSE_EnableValue		- Port Enable Status data      00 - Port Disabled , 01 - Port Enabled
 * 				PairControlValue    - Port Pair Control data       01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 				PortModeValue       - Port Type Definition data    00 - af.    01 - at.
 * 				PortPriorityValue	- Port Priority Level data	   00 - Critical , 01 - High , 10 - Low
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK         - operation succeed
 * 			    != e_POE_STATUS_OK      - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_Set_All_PORTS_CFG(_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue)
{
	U8 icIndex;

	/* SetDisPortsCmd - disable all IC's ports */
    result = mscc_IC_COMM_WriteDataToAllICs(DisPortsCmd_ADDR,0x0FFF);
    if(result != e_POE_STATUS_OK)
    	goto LableEnableSinglePort2;

    result = OS_Sleep_mS(30);
    if(result != e_POE_STATUS_OK)
    	goto LableEnableSinglePort2;

    for(icIndex = 0; icIndex <= SystemParams.NumOfActiveICsInSystem-1; icIndex++)
    {
        result = mscc_poe_cmd_Set_PORTS_CFG_to_a_single_IC(icIndex,_IN  bPSE_Enable,_IN  bPairControl,_IN  bPortMode,_IN  bPortPriority,_IN  PSE_EnableValue,_IN  PairControlValue,_IN  PortModeValue,_IN  PortPriorityValue);
        if(result != e_POE_STATUS_OK)
        	goto LableEnableSinglePort2;
    }

  LableEnableSinglePort2:

    /* SetDisPortsCmd - enable all IC's ports */
    tmpResult = mscc_IC_COMM_WriteDataToAllICs(DisPortsCmd_ADDR,0);
    UPDATE_RESULT(result,tmpResult);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = OS_Sleep_mS(30);
    return result;
}



/*---------------------------------------------------------------------
 *    description:      This function returns the control register fields from a single IC's ports .
 * 							PSE_Enable	: 2;
 *							PairControl	: 2;
 *							PortMode 	: 2;
 *							PortPriority : 2;
 *
 *    input :   icIndex        - IC index
 *
 *    output:   PORT_CFG_t     - structure array that contain the ports control register fields:
 * 								 PSE_EnableValue	- Port Enable Status data    00 - Port Disabled , 01 - Port Enabled
 * 								 PairControlValue   - Port Pair Control data     01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 								 PortModeValue      - Port Type Definition data  00 - af.    01 - at.
 * 								 PortPriorityValue	- Port Priority Level data	 00 - Critical , 01 - High , 10 - Low
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_poe_cmd_Get_PORTS_CFG_from_a_single_IC(_IN U8 icIndex,_OUT PORT_CFG_t PORT_CFG[])
{
	static U8 RxData[MAX_CH_PER_IC*2];
	U8 chIndex;
    U16 NumberOFBytesToRead =  IcParams[icIndex].NumOfChannesInIC * 2;
    U16 Port_CR;

    result = mscc_IC_COMM_I2C_Read (_IN IcParams[icIndex].IC_Address ,_IN  Port0_CR_ADDR,_OUT RxData, _IN NumberOFBytesToRead);
    if(result != e_POE_STATUS_OK)
    	return result;

    for(chIndex = 0; chIndex < IcParams[icIndex].NumOfChannesInIC; chIndex++)
    {
        Port_CR = mscc_GetExtractData(RxData[chIndex * 2], RxData[(chIndex * 2) + 1]);
        PORT_CFG[chIndex].Byte = (U8)Port_CR;
    }

    return result;
}




/*---------------------------------------------------------------------
 *    description:      This function sets the control register fields to a single IC's ports .
 * 							PSE_Enable	: 2;
 *							PairControl	: 2;
 *							PortMode 	: 2;
 *							PortPriority : 2;
 *
 *    input :   icIndex			    - IC index
 *              bPSE_Enable         - Port Enable Status flag      POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPairControl        - Port Pair Control flag       POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortMode           - Port Type Definition flag    POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortPriority       - Port Priority Level flag     POE_TRUE - change data , POE_FALSE - don't change data
 * 				PSE_EnableValue		- Port Enable Status data      00 - Port Disabled , 01 - Port Enabled
 * 				PairControlValue    - Port Pair Control data       01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 				PortModeValue       - Port Type Definition data    00 - af.    01 - at.
 * 				PortPriorityValue	- Port Priority Level data	   00 - Critical , 01 - High , 10 - Low
 *
 *    output:  	none
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_Set_PORTS_CFG_to_a_single_IC(_IN U8 icIndex,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue)
{
    /* SetDisPortsCmd - disable specific IC's ports */
    result = mscc_IC_COMM_WriteDataToSpecificIC(icIndex,DisPortsCmd_ADDR,0x0FFF);
    if(result != e_POE_STATUS_OK)
    	goto LableEnableSinglePort3;

    result = OS_Sleep_mS(30);
    if(result != e_POE_STATUS_OK)
    	goto LableEnableSinglePort3;

    result = mscc_poe_cmd_Set_PORTS_CFG_to_a_single_IC(icIndex,_IN  bPSE_Enable,_IN  bPairControl,_IN  bPortMode,_IN  bPortPriority,_IN  PSE_EnableValue,_IN  PairControlValue,_IN  PortModeValue,_IN  PortPriorityValue);

  LableEnableSinglePort3:

    /* SetDisPortsCmd - enable specific IC's ports */
    tmpResult = mscc_IC_COMM_WriteDataToSpecificIC(icIndex,DisPortsCmd_ADDR,0);
    UPDATE_RESULT(result,tmpResult);
    if(result != e_POE_STATUS_OK)
    	return result;

    result = OS_Sleep_mS(30);
    return result;
}







/*---------------------------------------------------------------------
 *    description:      This function sets the control register fields to a single IC's ports .
 * 							PSE_Enable	: 2;
 *							PairControl	: 2;
 *							PortMode 	: 2;
 *							PortPriority: 2;
 *
 *    input :   icIndex		        - IC index
 *              bPSE_Enable         - Port Enable Status flag      POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPairControl        - Port Pair Control flag       POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortMode           - Port Type Definition flag    POE_TRUE - change data , POE_FALSE - don't change data
 * 				bPortPriority       - Port Priority Level flag     POE_TRUE - change data , POE_FALSE - don't change data
 * 				PSE_EnableValue		- Port Enable Status data      00 - Port Disabled , 01 - Port Enabled
 * 				PairControlValue    - Port Pair Control data       01 - Alternative A , 10 - Alternative B (Backoff Enable)
 * 				PortModeValue       - Port Type Definition data    00 - af.    01 - at.
 * 				PortPriorityValue	- Port Priority Level data	   00 - Critical , 01 - High , 10 - Low
 *
 *    output:  	none
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_poe_cmd_Set_PORTS_CFG_to_a_single_IC(_IN U8 icIndex,_IN U8 bPSE_Enable,_IN U8 bPairControl,_IN U8 bPortMode,_IN U8 bPortPriority,_IN U8 PSE_EnableValue,_IN U8 PairControlValue,_IN U8 PortModeValue,_IN U8 PortPriorityValue)
{
	U8 chIndex;
	U16 NumberOFBytesToWrite;
	U8 Txdata[MAX_CH_PER_IC*2];
	PORT_CFG_t PORT_CFG[MAX_CH_PER_IC];

    /* get actual control register fields data  */
    result = mscc_poe_cmd_Get_PORTS_CFG_from_a_single_IC(icIndex,PORT_CFG);
    if(result != e_POE_STATUS_OK)
    	return result;

    /* change relevants control register fields data  */
    for(chIndex = 0; chIndex <= IcParams[icIndex].NumOfChannesInIC; chIndex++)
    {
        if(bPSE_Enable == POE_TRUE)     PORT_CFG[chIndex].Bits.PSE_Enable = PSE_EnableValue;
        if(bPairControl == POE_TRUE)    PORT_CFG[chIndex].Bits.PairControl = PairControlValue;
        if(bPortMode == POE_TRUE )      PORT_CFG[chIndex].Bits.PortMode = PortModeValue;
        if(bPortPriority == POE_TRUE)   PORT_CFG[chIndex].Bits.PortPriority = PortPriorityValue;
    }

    /* build bytes array to transmit with modified data */
    NumberOFBytesToWrite = IcParams[icIndex].NumOfChannesInIC * 2;

    for(chIndex = 0; chIndex < IcParams[icIndex].NumOfChannesInIC; chIndex++)
    {
        Txdata[chIndex*2] = 0;
        Txdata[(chIndex*2)+1] =  PORT_CFG[chIndex].Bits.PSE_Enable | (PORT_CFG[chIndex].Bits.PairControl << 2) |
                                 (PORT_CFG[chIndex].Bits.PortMode << 4) | (PORT_CFG[chIndex].Bits.PortPriority << 6 );
    }

    /* write modified data to specific IC */
    result = mscc_IC_COMM_I2C_Write(_IN IcParams[icIndex].IC_Address,_IN Port0_CR_ADDR,  _IN Txdata, _IN NumberOFBytesToWrite);
    return result;
}




/*---------------------------------------------------------------------
 *    description:      This function gets the Total calculated and actual power consumed by all ports
 *
 *    input :   None
 *
 *    output:  	pSysTotalRealPowerConsData   -  the actual power consumed by all ports
 *              pSysTotalCalcPowerConsData   -  the sum of all ports power, allocated as defined by IEEE standard
 * 											   802.3af-2003, or actually consumed, according to the Calculated Power Management Mode
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_Get_Total_Power(_OUT U16 *pSysTotalRealPowerConsData,_OUT U16 *pSysTotalCalcPowerConsData)
{
	U16 SysTotalCalcPowerCons_regData = 0;
	U16 SysTotalRealPowerCons_regData = 0;

	*pSysTotalRealPowerConsData = 0;
	*pSysTotalCalcPowerConsData = 0;

	/* sync command */
	result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,UpdateRLPMParams_ADDR, 1);
	if (result != e_POE_STATUS_OK)
		return result;

	result = OS_Sleep_mS(20);
	if (result != e_POE_STATUS_OK)
		return result;

	/* Power Consumption */
	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysTotalRealPowerCons_ADDR, &SysTotalRealPowerCons_regData);
	if (result != e_POE_STATUS_OK)
		return result;

	/* convert from deciWatt to Watt */
	*pSysTotalRealPowerConsData = SysTotalRealPowerCons_regData / 10;

	/* Calculated Power */
	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysTotalCalcPowerCons_ADDR, &SysTotalCalcPowerCons_regData);
	if (result != e_POE_STATUS_OK)
		return result;

	/* convert from deciWatt to Watt */
	*pSysTotalCalcPowerConsData = SysTotalCalcPowerCons_regData / 10;

	return result;
}





/*---------------------------------------------------------------------
 *    description:      This function gets the Total calculated Power
 *
 *    input :   None
 *
 *    output:  	pSysTotalCalcPowerConsData   -  the sum of all ports power, allocated as defined by IEEE standard
 * 											   802.3af-2003, or actually consumed, according to the Calculated Power Management Mode
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_Get_SysTotalCalcPower(_OUT U16 *pSysTotalCalcPowerConsData_dW)
{
	U16 SysTotalCalcPowerCons_regData = 0;

	*pSysTotalCalcPowerConsData_dW = 0;

	/* sync command */
	result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,	UpdateRLPMParams_ADDR, 1);
	if (result != e_POE_STATUS_OK)
		return result;

	result = OS_Sleep_mS(20);
	if (result != e_POE_STATUS_OK)
		return result;

	/* Calculated Power */
	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,
			SysTotalCalcPowerCons_ADDR, &SysTotalCalcPowerCons_regData);
	if (result != e_POE_STATUS_OK)
		return result;

	*pSysTotalCalcPowerConsData_dW = SysTotalCalcPowerCons_regData;

	return result;
}




/**************** Layer2 functions ******************/



/*---------------------------------------------------------------------
 *    description:
 *              This is the main function of the Layer2. From here, all other functions
 *				are called.
 *
 *    input :   none
 *
 *    output:  	none
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
S32 mscc_POE_CMD_LLDP_AndLayer2PM(void)
{
	/* performs Layer2 functionality if enabled and only if not working with dynamic power */
	if(Masks.Layer2 && bStaticPM_CalcMode)
	{
		/* decrement LLDP counters every 15 seconds */
		mscc_poe_cmd_LLDPTimeoutCounter();

		/* Convert PD power requests and save their new TPPL's */
		if(L2_Sync_Flag)
		{
			mscc_poe_cmd_ConvertPDRequests();
			mscc_poe_cmd_PM_Check(NONE_CHANNEL);
		}
	}

	return e_POE_STATUS_OK;
}



/*---------------------------------------------------------------------
 *    description:
 *              This Function if power consumption is in no more connect zone.
 *				It calculates the GB by fixed or auto GB
 *
 * 				This Function check if there is a power management problem.
 * 				if there is a problem this function shout down channels till
 * 				there is no longer problem.
 *
 *    input :   ChNum  -  channel number
 *
 *    output:  	none
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *---------------------------------------------------------------------*/
U32 mscc_poe_cmd_PM_Check(_IN U8 ChNum)
{
	#define L2_DELTA 5  /* 5 watts */

	S32	PowerTPPL_ChSum_dW = 0;
	U8	RealCH;
	U8 	Five_watts_dW;
	U8  ChannelIndex ;
	U8 PortStatus;
	U16 All_TPPL_Req_Sum_dW;
	U8 PortStatusData;
	U8 AtType;
	U8 bIsPortOn;
	U16 Current_TPPL_Data_dW;
	U16 Current_TPPL_Data_mW;
	U16 wPortPower_dW;
	S16 swPortAdditionPower_dW;
	U8 ActivePowerBank;
	U16  SYS_POWER_CONSUME_STATIC_dW;
	U16 ConvertedPDReqPower_dW;
	U16 PowerBudget_dW;

	/* update static power consume from master */
	result =  mscc_POE_CMD_Get_SysTotalCalcPower(&(SYS_POWER_CONSUME_STATIC_dW));
	if(result != e_POE_STATUS_OK)
		return result;

	/* Check whether we work with Static/Dynamic Power Management */
	Five_watts_dW = ONE_WATT_FACTOR * L2_DELTA;  /* 5deciwatts --> 50 */

	All_TPPL_Req_Sum_dW = 0;
	/* sum all the the port's requested TPPL power */
	for(ChannelIndex = 0 ; ChannelIndex < MAX_CH_PER_SYSTEM; ChannelIndex++)
	{
		if(ChannelParams[ChannelIndex].L2Data.CalcReq == POE_TRUE)
			All_TPPL_Req_Sum_dW += ChannelParams[ChannelIndex].L2Data.ConvertedPDReqPower_dW;
	}

	/* update Power Budget Index */
	result = mscc_POE_CMD_GetActiveBank(_IN  SystemParams.IC_Master_Index,_OUT &ActivePowerBank);
	if(result != e_POE_STATUS_OK)
		return result;

	/* Max Power Budget Available */
	result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(ActivePowerBank*2) ,&PowerBudget_dW);
	if(result != e_POE_STATUS_OK)
		return result;

	/* Check If we can supply all the requiered TPPL power without scanning all ports */
	/* Check If Need To Shut Down Channels (If The calculated Power Consume Is Greater Than The Power Bank Budget) */
	if((SYS_POWER_CONSUME_STATIC_dW + All_TPPL_Req_Sum_dW + Five_watts_dW) < PowerBudget_dW)
	{
		for(ChannelIndex = 0 ; ChannelIndex < SystemParams.NumOfChannelsInSystem; ChannelIndex++)
		{
			/* Supply all the Port's TPPL power requested */
			if(ChannelParams[ChannelIndex].L2Data.CalcReq)
			{
				result = mscc_POE_CMD_SetChannelTPPL(ChannelIndex,(U16)( ChannelParams[ChannelIndex].L2Data.ConvertedPDReqPower_dW * 100)  /*TPPL_mW*/);
				ChannelParams[ChannelIndex].L2Data.CalcReq = POE_FALSE; /* Set the flag off */
			}
		}
	}
	else /* we need to scan all TPPL ports - one by one.... */
	{
		/* update priority list is needed */
		if(PortsPriorityChanged == POE_TRUE)
		{
			PortsPriorityChanged = POE_FALSE;
			mscc_poe_cmd_BuildPriorityTable();
		}

		for(ChannelIndex = 0; ChannelIndex < SystemParams.NumOfChannelsInSystem; ChannelIndex++)
		{
			/* Find the Physical(real) channel by PRIORITY */
			RealCH = PriorityTable[ChannelIndex];

			/* port is not layer2 or tppl port that has no request*/
			if(!ChannelParams[RealCH].L2Data.CalcReq)
				continue;

			PortStatusData = 0;
			AtType = 0;
			bIsPortOn = 0;

			result = mscc_POE_CMD_GetChannelStatus(RealCH, &PortStatusData, &bIsPortOn,&AtType);
			if (result != e_POE_STATUS_OK)
				return result;

			PortStatus = PortStatusData;

			/* Check the status of the channel */
			/* If the value is higher than BIT the cahnnel is not ON, so skip it */
			if(!bIsPortOn)
				continue;

			ConvertedPDReqPower_dW = ChannelParams[RealCH].L2Data.ConvertedPDReqPower_dW;

			/* Read RealCH TPPL value */
			Current_TPPL_Data_mW = 0;
			result = mscc_POE_CMD_GetChannelTPPL(RealCH, &Current_TPPL_Data_mW);
			if (result != e_POE_STATUS_OK)
				return result;

			/* convert from mWatt to deciWatt */
			Current_TPPL_Data_dW = Current_TPPL_Data_mW / 100;

			/* if pd power request is in PB set the new TPPL value to the port */
			/* check if we can supply all TPPL request value to the port */
			if((SYS_POWER_CONSUME_STATIC_dW + PowerTPPL_ChSum_dW + ConvertedPDReqPower_dW - Current_TPPL_Data_dW + Five_watts_dW) < PowerBudget_dW)
			{
				/* set TPPL value to the port */
				result = mscc_POE_CMD_SetChannelTPPL(RealCH,(U16)( ConvertedPDReqPower_dW * 100)  /*TPPL_mW*/);
				if (result != e_POE_STATUS_OK)
					return result;

				/* Calculate the TPPL ports addition total power */
				PowerTPPL_ChSum_dW += ConvertedPDReqPower_dW - Current_TPPL_Data_dW;

			}
			else  /* If this port can be reduced and stays at least with 5Watt, don't shut it off!! */
			{	  /* Copy this port to the reduction struct */

				swPortAdditionPower_dW = PowerBudget_dW - SYS_POWER_CONSUME_STATIC_dW - PowerTPPL_ChSum_dW - Five_watts_dW;

				if(swPortAdditionPower_dW > 0)
				{
					wPortPower_dW = (U16) swPortAdditionPower_dW + Current_TPPL_Data_dW;

					/* If this port can be reduced and stays at least with 5Watt, don't shut it off!!
					 * Copy this port to the reduction struct */
					L2Reduction.ChNum = RealCH;
					L2Reduction.Allocated = mscc_POE_CMD_ConvertPSEAllocation(wPortPower_dW, RealCH);

					/* set TPPL value to the port */
					result = mscc_POE_CMD_SetChannelTPPL(RealCH,(U16)( wPortPower_dW * 100)  /*TPPL_mW*/);
					if (result != e_POE_STATUS_OK)
						return result;

					PowerTPPL_ChSum_dW += swPortAdditionPower_dW;
				}
			}

			/* Set the flag off */
			ChannelParams[RealCH].L2Data.CalcReq = POE_FALSE;
		}
	}

	return result;
}



/*----------------------------------------------------------------------------------------------------
 *    description:      This function Build Priority Table when index 0 is the highest priority.
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   e_POE_STATUS_OK      - operation succeed
 * 			    != e_POE_STATUS_OK   - operation failed
 *-------------------------------------------------------------------------------------------------*/
S32 mscc_poe_cmd_BuildPriorityTable(void)
{
	static U8 RxData[MAX_CH_PER_IC * 2];

    U8 TmpPriorityArr[SystemParams.NumOfChannelsInSystem];
    U8 chPhysicalIndex = 0;
    U8 icIndex;
    U8 chIndex;
    PORT_CFG_t PORT_CFG;
    U8 NumberOFBytesToRead;
    U16 Port_CR;

    for(icIndex = 0; icIndex < SystemParams.NumOfActiveICsInSystem; icIndex++)
    {
    	NumberOFBytesToRead = IcParams[icIndex].NumOfChannesInIC *2;
    	result = mscc_IC_COMM_I2C_Read (_IN IcParams[icIndex].IC_Address ,_IN  Port0_CR_ADDR,_OUT RxData, _IN NumberOFBytesToRead);
    	if(result != e_POE_STATUS_OK)
    		return result;

    	for(chIndex = 0; chIndex < IcParams[icIndex].NumOfChannesInIC; chIndex++)
    	{
    		Port_CR = mscc_GetExtractData(RxData[chIndex * 2], RxData[(chIndex * 2) + 1]);
    		PORT_CFG.Byte = (U8)Port_CR;
    		TmpPriorityArr[chPhysicalIndex] = PORT_CFG.Bits.PortPriority;
    		chPhysicalIndex++;
    	}
    }

    chPhysicalIndex = 0;
    /* arrange critical ports priority */
    for(chIndex = 0; chIndex < SystemParams.NumOfChannelsInSystem; chIndex++)
    {
    	if(TmpPriorityArr[chIndex] == e_InternalPortPriority_Critical )
    	{
    		PriorityTable[chPhysicalIndex] = chIndex;
    		chPhysicalIndex++;
    	}
    }
    /* arrange High ports priority */
    for(chIndex = 0; chIndex < SystemParams.NumOfChannelsInSystem; chIndex++)
    {
    	if(TmpPriorityArr[chIndex] == e_InternalPortPriority_High )
    	{
    		PriorityTable[chPhysicalIndex] = chIndex;
    		chPhysicalIndex++;
    	}
    }
    /* arrange Low ports priority */
    for(chIndex = 0; chIndex < SystemParams.NumOfChannelsInSystem; chIndex++)
    {
        if(TmpPriorityArr[chIndex] == e_InternalPortPriority_Low )
        {
        	PriorityTable[chPhysicalIndex] = chIndex;
        	chPhysicalIndex++;
        }
    }

    return result;
}



/*----------------------------------------------------------------------------------------------------
 *    description:      This function converts all PD power requests and saves their TPPL value.
 * 						P(PSE) = P(PD) + IIR;
 *						P(PSE) = P(PD) + [(P(PSE)/Vmain)^2]*R
 *						R*X^2 - 2500*X + 2500*PDReq = 0
 *						X2 = 25000 - 223*SQRT(12500-2R*PDReq)
 *      				Ppse = (25*(250 - SQR(62500-140*Ppd))) / 7
 *						More detailed information on the formula below is described in the ECS.
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   none
 *
 *-------------------------------------------------------------------------------------------------*/
void mscc_poe_cmd_ConvertPDRequests(void)
{
	#define CONST_2WATT 20
	#define MAX_CABLE_RESISTANCE 28 /* 28 ohm for 2R (assuming 100M cable length) */
	U16 tmp;
	U8 ChannelIndex;

   	for(ChannelIndex = 0; ChannelIndex < MAX_CH_PER_SYSTEM; ChannelIndex++)
	{
		/* If the req flag is not set, go to next channel */
   	    if(!(ChannelParams[ChannelIndex].L2Data.CalcReq))
   	    	continue;

		/* calculating 2R*PDReq * 5 = 140 * PDReq */
		tmp = (U16)(140 * ChannelParams[ChannelIndex].L2Data.PDReqPower_dW);

		/* calculating the new TPPL if the square root is not negative!!! */
		if(tmp < 62500)
		{
			/* calculating the square root */
			tmp = mscc_poe_cmd_Sqrt(62500 - tmp);
			/* verify that there was no error in the square root calculation and not dividing by zero!!! */
			if(tmp != MAX_WORD)
			{
				tmp = 250 - tmp;
				tmp *= 25;
				tmp /= 7;
				tmp = (U16)tmp - (U16)(ChannelParams[ChannelIndex].L2Data.PDReqPower_dW);
				tmp = (U16)(tmp * ChannelParams[ChannelIndex].L2Data.CableLen) / (U16)L2_MAX_CABLE_LEN;
				tmp += ChannelParams[ChannelIndex].L2Data.PDReqPower_dW;
			}
			else
			{
				tmp = ChannelParams[ChannelIndex].L2Data.PDReqPower_dW + CONST_2WATT;
			}
		}
		/* TPPL in deciwatts unit) */
		ChannelParams[ChannelIndex].L2Data.ConvertedPDReqPower_dW = tmp;
	}

	/* we went over all channels - clear the L2 sync flag */
   	L2_Sync_Flag = POE_FALSE;
}



/*----------------------------------------------------------------------------------------------------
 *    description:
 * 				This function is called only when there is a need to reduce power
 *				of a specific channel. The new power value is calculated here and
 *				is saved in the Layer2's reduction struct with it's channel number.
 *
 *    input :   val       - PSE actual real port power
 * 	            channel   - channel number
 *
 *    output:   none
 *
 *    return:   converted PSE Allocation
 *
 *-------------------------------------------------------------------------------------------------*/
U16 mscc_POE_CMD_ConvertPSEAllocation(_IN U16 val,_IN U8 channel)
{
	U32 tmp;

	tmp = (U32)((U32)val * (U32)val);
	tmp *= (U32)((ChannelParams[channel].L2Data.CableLen + 12) >> 2);
	tmp /= (U32)50000;
	return ((U16)((U32)val - (U32)tmp));
}




/*----------------------------------------------------------------------------------------------------
 *    description:
 * 				This Function handles the LLDP timeout for all channels. When 45 seconds
 *				passes without LLDP communication, the LLDP counter is initialized to 0 and
 *				is no longer defined as LLDP compliance.
 *
 *      		important: This function is based on the DELAY_ONE_SEC constant - this function
 *				must run every 15 seconds.
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   none
 *
 *-------------------------------------------------------------------------------------------------*/
void mscc_poe_cmd_LLDPTimeoutCounter(void)
{
	#define TICKS_PER_REDUCTION 15
	static U8 L2Counter = 0;
	U8 RealCH;
	U8 PortStatusData = 0;
	U8 AtType = 0;
	U8 bIsPortOn=0;
	U16 PortPowerLimit_Data_mW = 0;
	U16 TemporaryPortPowerLimit_Data_mW = 0;
	U8 Port_Class_Data = 0;   /* power class */

	L2Counter++;

	if(L2Counter == TICKS_PER_REDUCTION)
	{
		for (RealCH = 0; RealCH < MAX_CH_PER_SYSTEM; RealCH++)
		{
			/* decrement each channel counter if greater then 0. */
		 	if(ChannelParams[RealCH].L2Data.PDCount > 0)
		 	{
		 		/* if port is no longer LLDP :  AT -> TPPL = PPL , AF -> TPPL = Class or PPL - the lowest among them */
		 		if(ChannelParams[RealCH].L2Data.PDCount == 1)
		 		{
		 			result = mscc_POE_CMD_GetChannelStatus(RealCH, &PortStatusData,&bIsPortOn, &AtType);
		 			if (result != e_POE_STATUS_OK)
		 				return;

		 			if (!bIsPortOn)
		 				continue;

		 			result = mscc_POE_CMD_GetChannelPPL(RealCH, &PortPowerLimit_Data_mW);
		 			if (result != e_POE_STATUS_OK)
		 				return;

		 			if(AtType)
		 			{
		 				TemporaryPortPowerLimit_Data_mW = PortPowerLimit_Data_mW;
		 			}
		 			else
		 			{
		 				/* get power class */
		 				result = mscc_POE_CMD_GetChannelClass(RealCH,&Port_Class_Data);
		 				if(result != e_POE_STATUS_OK)
		 					return;

		 				TemporaryPortPowerLimit_Data_mW = mscc_MIN(ClassPower_mW[Port_Class_Data],PortPowerLimit_Data_mW);
		 			}

		 			/* set new Channel TPPL value */
		 			result = mscc_POE_CMD_SetChannelTPPL(RealCH, TemporaryPortPowerLimit_Data_mW);
		 			if (result != e_POE_STATUS_OK)
		 				return;
		 		}

		 		ChannelParams[RealCH].L2Data.PDCount--;
		 	}
		}

		/* if reached to TICKS_PER_REDUCTION, start count from the beginning */
		L2Counter = 0;
	}
}



/*----------------------------------------------------------------------------------------------------
 *    description:
 * 				This Function returns square root of a given value, using
 *				the Babylonian method. The input values to be calculated
 *				must be in the range of 6500 - 62500. Thus, the answers
 *				are between 80 - 250.  We take to closest number possible above - 260.
 *
 *				S = original value to calculate
 *		        X0 = 260	- upper limit of the possible answer (HASAM ELION)
 *              X(n+1) = ( X(n) +  S / X(n) ) / 2
 *
 *    input :   val  -  given value to calculate
 *
 *    output:   none
 *
 *    return:   square root of the given value
 *
 *-------------------------------------------------------------------------------------------------*/
U16 mscc_poe_cmd_Sqrt(_IN U16 val)
{
	#define X0 260    		 /* the closest number possible */
	#define MAX_ITERATION 8	 /* in case we reached this limit, the function wasn't convergenced successfully */
	U16 x[2];
	U8 iteration_cnt = 0;
	U16 *p1 = x, *pTemp;     /* pointer to x[0] */
	U16 *p2 = &x[1]; /* pointer to x[1] */

	/* X0 is closest as possible to the final answer  ~X0 */
	*p1 = X0;
	*p2 = MAX_WORD;

	while( (*p1 != *p2) && (iteration_cnt < MAX_ITERATION) )  /* finish when the current calculation is equal to the one before. */
	{
		*p2 = val / (*p1);
		*p2 += (*p1);
		*p2 /= 2;
		pTemp = p1;
		p1 = p2;
		p2 = pTemp;
		iteration_cnt++;
	}

	if(iteration_cnt == MAX_ITERATION)
		*p1 = MAX_WORD;  /* if we reached the iteration limit, return error (MAX_WORD) */

	return (U16)(*p1);
}






/*----------------------------------------------------------------------------------------------------
 *    description:
 * 				This Function returns square root of a given value, using
 *				the Babylonian method. The input values to be calculated
 *				must be in the range of 6500 - 62500. Thus, the answers
 *				are between 80 - 250.  We take to closest number possible above - 260.
 *
 *				S = original value to calculate
 *		        X0 = 260	- upper limit of the possible answer (HASAM ELION)
 *              X(n+1) = ( X(n) +  S / X(n) ) / 2
 *
 *    input :   val  -  given value to calculate
 *
 *    output:   none
 *
 *    return:   square root of the given value
 *
 *-------------------------------------------------------------------------------------------------*/





U16 mscc_POE_CMD_IgnoreHighPriority()
{
	static U8 RxData[16];

	U8 ActivePowerBank;

	switch (PriorityStep)
    {
		case 0:
        {
        	/* read registers */
            U16 NumberOFBytesToRead = 16;

            U16 SysTotalHighCons_dW;
            U16 SysTotalLowCons_dW;
            U16 SysTotalCriticalReq_dW;
            U16 SysTotalHighReq_dW;
            U16 SysTotalDeltaPower_Reg;
            POE_BOOL bSysTotalDeltaPower_PositiveSign;


            /* sync command */
            result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,UpdateRLPMParams_ADDR, 1);
            if (result != e_POE_STATUS_OK)
            	return result;

            result = OS_Sleep_mS(20);
            if (result != e_POE_STATUS_OK)
            	return result;

            /* read registers */
            result = mscc_IC_COMM_I2C_Read (_IN IcParams[SystemParams.IC_Master_Index].IC_Address ,_IN  SysTotalHighCons_ADDR,_OUT RxData, _IN NumberOFBytesToRead);
            if(result != e_POE_STATUS_OK)
            	return result;

            /* extract registers data */
            SysTotalHighCons_dW = mscc_GetExtractData(RxData[0], RxData[1]);
            SysTotalLowCons_dW = mscc_GetExtractData(RxData[2], RxData[3]);
            SysTotalCriticalReq_dW = mscc_GetExtractData(RxData[4], RxData[5]);
            SysTotalHighReq_dW = mscc_GetExtractData(RxData[6], RxData[7]);
            SysTotalDeltaPower_Reg = mscc_GetExtractData(RxData[14], RxData[15]);

            /* check whether register SysTotalDeltaPower is negative or positive (the MSb is a sign bit) */
            bSysTotalDeltaPower_PositiveSign = ((SysTotalDeltaPower_Reg & 0x8000) == 0) ? POE_TRUE : POE_FALSE;

            if (bSysTotalDeltaPower_PositiveSign)
               	return result;

            if ((SysTotalCriticalReq_dW > 0) && ((SysTotalHighCons_dW > 0) || (SysTotalLowCons_dW > 0)))
            {
                 goto GoNext;
            }
            else
            {
                if ((SysTotalHighReq_dW > 0) && (SysTotalLowCons_dW > 0))
                    goto GoNext;
                else
               	    return result;
            }

        GoNext:

            /* get selected active budget */
            result = mscc_POE_CMD_GetActiveBank(_IN  SystemParams.IC_Master_Index,_OUT &ActivePowerBank);
            if(result != e_POE_STATUS_OK)
            	return result;

            /* Get selected active bank power budget and save it in usOriginalSysPowerBudget varible */
            result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(ActivePowerBank*2) ,&wOriginalSysPowerBudget_dW);
            if(result != e_POE_STATUS_OK)
            	return result;

            /* add 32W to the selected active bank power budget and save it in usNewPowerBudget varible */
            wNewPowerBudget_dW = (wOriginalSysPowerBudget_dW + 320);

            /* write the usNewPowerBudget to the selected active bank power */
            result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(ActivePowerBank*2) ,wNewPowerBudget_dW);
            if(result != e_POE_STATUS_OK)
            	return result;

            /**** sync commands ****/

            /* I2C_ExtSyncType */
            result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,I2C_ExtSyncType_ADDR ,4);
            if(result != e_POE_STATUS_OK)
            	return result;

            /* EXT_EV_IRQ */
            result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,EXT_EV_IRQ_ADDR ,4);

            PriorityStep = 1;
            break;
        }
        case 1:
        {
        	PriorityStep = 2;
        	/* this step operate delay of 2 seconds between steps 1 and 3 */
        	break;
        }
        case 2:
        {
        	U16 wSysPowerBudget_dW;
        	PriorityStep = 0;

        	/* get selected active budget */
            result = mscc_POE_CMD_GetActiveBank(_IN  SystemParams.IC_Master_Index,_OUT &ActivePowerBank);
            if(result != e_POE_STATUS_OK)
                return result;

            /* Get selected active bank power budget and save it in usOriginalSysPowerBudget varible */
            result = mscc_IC_COMM_ReadDataFromSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(ActivePowerBank*2) ,&wSysPowerBudget_dW);
            if(result != e_POE_STATUS_OK)
            	return result;

            /* check that the power budget didn't change in the middle of the process */
            if (wSysPowerBudget_dW == wNewPowerBudget_dW)
            {
                /* write the Original power budget to the selected active bank power */
                result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,SysPowerBudget0_ADDR+(ActivePowerBank*2) ,wOriginalSysPowerBudget_dW);
                if(result != e_POE_STATUS_OK)
                	return result;

                /**** sync commands ****/

                /* I2C_ExtSyncType */
                result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,I2C_ExtSyncType_ADDR ,4);
                if(result != e_POE_STATUS_OK)
                	return result;

                /* EXT_EV_IRQ */
                result = mscc_IC_COMM_WriteDataToSpecificIC(SystemParams.IC_Master_Index,EXT_EV_IRQ_ADDR ,4);
            }

            break;
        }
    }

    return result;
}





