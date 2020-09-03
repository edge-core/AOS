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
 *  Description:  Code Encryption for POE communication
 *
 *
 *************************************************************************/

#ifndef MSCC_POE_CODES_CONVERSION_H_
#define MSCC_POE_CODES_CONVERSION_H_

/*=========================================================================
/ INCLUDES
/========================================================================*/

#include "mscc_poe_global_types.h"

/*=========================================================================
/ CONSTANTS
/========================================================================*/

#define MaxTemperatureLimit			155
#define MAX_POWER_PER_PORT 			40000 /* mW */
#define BROADCAST_I2C_ADDRESS	    0
#define MAX_IC_ON_BOARD			    8
#define MAX_CH_PER_IC				12
#define MAX_CH_PER_SYSTEM			96
#define MAX_BANK_IN_IC			    8
#define MAX_BUDGET_LIMIT			6500
#define MIN_BUDGET_LIMIT			30
#define IC_12_CHANNELS			    12 /* Number of Channels for IC */
#define IC_8_CHANNELS				8  /* Number of Channels for IC */
#define IC_NONE_CHANNELS			0  /* Number of Channels for IC */
#define	MIN_SHUTDOWN_VOLTAGE		440  /* deci volts */
#define	MAX_SHUTDOWN_VOLTAGE		585  /* deci volts */
#define NONE_CHANNEL  				(MAX_CH_PER_SYSTEM+10)
#define MAX_POWER_BUDGET 			16
#define BUFFER_SIZE     			30




#ifndef NULL
    #define NULL (void *)0
#endif

#define	PACKAGE_SIZE				15
#define MAX_WORD 					0xFFFF

#define _IN      /* used to clarify if func param is in/out */
#define _OUT     /* used to clarify if func param is in/out */
#define _INOUT   /* used to clarify if func param is in/out */


/*=========================================================================
/ TYPES
/========================================================================*/

/* Types */
typedef char               S8   ;
typedef unsigned char      U8   ;
typedef signed short       S16  ;
typedef unsigned short     U16  ;
typedef long               S32  ;
typedef unsigned long      U32  ;


#define  BIT0          0x00000001
#define  BIT1          0x00000002
#define  BIT2          0x00000004
#define  BIT3          0x00000008
#define  BIT4          0x00000010
#define  BIT5          0x00000020
#define  BIT6          0x00000040
#define  BIT7          0x00000080
#define  BIT8          0x00000100
#define  BIT9          0x00000200
#define  BIT10         0x00000400
#define  BIT11         0x00000800
#define  BIT12         0x00001000
#define  BIT13         0x00002000
#define  BIT14         0x00004000
#define  BIT15         0x00008000
#define  BIT16         0x00010000
#define  BIT17         0x00020000
#define  BIT18         0x00040000
#define  BIT19         0x00080000
#define  BIT20         0x00100000
#define  BIT21         0x00200000
#define  BIT22         0x00400000
#define  BIT23         0x00800000
#define  BIT24         0x01000000
#define  BIT25         0x02000000
#define  BIT26         0x04000000
#define  BIT27         0x08000000
#define  BIT28         0x10000000
#define  BIT29         0x20000000
#define  BIT30         0x40000000
#define  BIT31         0x80000000



typedef enum
{
    I2C_7_BIT_ADDR_WITH_STOP_CONDITION     = 0x00,
    I2C_7_BIT_ADDR_WITHOUT_STOP_CONDITION      = 0x04
}I2cFlags_e;



/*
 * typedef: enum POE_BOOL
 *
 * Description: Enumeration of boolean.
 *
 * Enumerations:
 *    POE_FALSE - false.
 *    POE_TRUE  - true.
 */
typedef enum
{
	POE_FALSE = 0,
	POE_TRUE  = 1
} POE_BOOL;



//typedef S32 (*mscc_FPTR_Write)(_IN U8 I2C_Address, _IN I2cFlags_e Flags,_IN const U8* pTxdata,_IN U16 num_write_length,_IN void* pUserParam);
//typedef S32 (*mscc_FPTR_Read)(_IN U8 I2C_Address,_IN I2cFlags_e Flags,_OUT U8* pRxdata,_IN U16 length,_IN void* pUserParam);
typedef S32 (*mscc_FPTR_Write)(_IN U8 I2C_Address, _IN U16 Flags,_IN const U8* pTxdata,_IN U16 num_write_length,_IN void* pUserParam);
typedef S32 (*mscc_FPTR_Read)(_IN U8 I2C_Address,_IN U16 Flags,_OUT U8* pRxdata,_IN U16 length,_IN void* pUserParam);

enum MSCC_ProtocolCMD_e
{
    B_Command =           0x0,    /* 0 */
    B_Request =           0x2,    /* 2 */
    B_Telemetry =         0x3,    /* 3 */
    B_Channel=            0x05,   /* 5 */
    B_Global=             0x07,   /* 7 */
    B_Priority =          0x0A,   /* 10 */
    B_Supply=             0x0B,   /* 11 */
    B_EnDis=              0x0C,   /* 12 */
    B_PortStatus=         0x0E,   /* 14 */
    B_Main=               0x17,   /* 23 */
    B_PowerManage=        0x18,   /* 24 */
    B_Measurementz=       0x1A,   /* 26 */
    B_Versionz =          0x1E,   /* 30 */
    B_SWversion =         0x21,   /* 33 */
    B_Paramz=             0x25,   /* 37 */
    B_Maskz=              0x2B,   /* 43 */
    B_PortsStatus1=       0x31,   /* 49 */
    B_PortsStatus2=       0x32,   /* 50 */
    B_PortsStatus3=       0x33,   /* 51 */
    B_Latches=            0x3A,   /* 58 */
    B_SystemStatus=       0x3D,   /* 61 */
    B_UserByte=           0x41,   /* 65 */
    B_TmpMatrix=          0x43,   /* 67 */
    B_ChannelMatrix=      0x44,   /* 68 */
    B_PortsStatus4=       0x47,   /* 71 */
    B_PortsStatus5=       0x48 ,  /* 72 */
    B_Latches2=           0x49 ,  /* 73 */
    B_PortFullInit=       0x4A ,  /* 74 */
    B_PortsPower1=        0x4B ,  /* 75 */
    B_PortsPower2=        0x4C ,  /* 76 */
    B_PortsPower3=        0x4D ,  /* 77 */
    B_Space=              0x4E ,  /* 78 */
    B_PortsPower4=        0x4F  , /* 79 */
    B_PortsPower5=        0x50   ,/* 80 */
    B_ForcePower=         0x51   ,/* 81 */
    B_Report=             0x52   ,/* 82 */
    B_Reset =             0x55   ,/* 85 */
    B_Individual_Mask=    0x56   ,/* 86 */
    B_PowerBudget=        0x57   ,/* 87 */
    B_UDLcounter1=        0x59   ,/* 89 */
    B_UDLcounter2=        0x5A   ,/* 90 */
    B_PoEDeviceVersion=   0x5E   ,/* 94 */
    B_PowerManageMode=    0x5F   ,/* 95 */
    B_ExpendedPowerInfo=  0x60   ,/* 96 */
    B_AllPortClass=       0x61   ,/* 97 */
    B_IrqMask=            0x63   ,/* 99 */
    B_All_channels=       0x80   ,/* 128 */
    B_DetCnt1=            0x85   ,/* 133 */
    B_DetCnt2=            0x86   ,/* 134 */
    B_DeviceParams=       0x87   ,/* 135 */
    B_EnDis2=             0x88   ,/* 136 */
    B_UDLcounter3=        0x89   ,/* 137 */
    B_UDLcounter4=        0x8A   ,/* 138 */
    B_PortsStatus6=       0x8B   ,/* 139 */
    B_PortsStatus7=       0x8C   ,/* 140 */
    B_PortsStatus8=       0x8D   ,/* 141 */
    B_PortsStatus9=       0x8E   ,/* 142 */
    B_PortsStatus10=      0x8F   ,/* 143 */
    B_PortsPower6=        0x90   ,/* 144 */
    B_PortsPower7=        0x91   ,/* 145 */
    B_PortsPower8=        0x92   ,/* 146 */
    B_PortsPower9=        0x93   ,/* 147 */
    B_PortsPower10=       0x94   ,/* 148 */
    B_Latches3=           0x95   ,/* 149 */
    B_Latches4=           0x96   ,/* 150 */
    B_DetCnt3=            0x98   ,/* 152 */
    B_DetCnt4=            0x99   ,/* 153 */
    B_AllPortsPower=      0x9C   ,/* 156 */
    B_TemporarySupply=    0xA2   ,/* 162 */
    B_BackplanePowerData= 0xA4   ,/* 164 */
    B_BPMReqData=         0xA5   ,/* 165 */
    B_Layer2_PD =           0xA6   ,/* 166 */
    B_PowerBudgetSourceType=0xA7 ,/* 167 */
    B_Layer2_PSE=         0xA8    /* 168 */
};


typedef enum
{
	e_MaskKey_AC_DisEnable = 0x9,
	e_MaskKey_Back_off = 0x11,
	e_MaskKey_Ignore_priority = 0x29,
	e_MaskKey_Layer2 = 0x2E,
	e_MaskKey_Layer2PriorityByPD = 0x2F
} mscc_mask_type_e;



typedef enum
{
    e_CustomerChannelOKCap = 0,                   /* channel ON Cap */
    e_CustomerChannelOKRes = 1,                   /*  channel ON Res	 */
    e_CustomerPortNotActive=12,
    e_CustomerUnDefined=17,
    e_CustomerFPGA_Error=18,                      /* FPGA Doesn't work proparly	 */
    e_CustomerChannelOFF=26,                      /* channel off becuse command */
    e_CustomerInit=27,                            /* channel OFF */
    e_CustomerNotRes=28,
    e_CustomerUDL=30,
    e_CustomerOVL=31,
    e_CustomerPowerManagement=32,
    e_CustomerForcePowerON=43,
    e_CustomerForcePowerError=44,
    e_CustomerShortCircuit=52,
    e_CustomerUnKnownICStatus=55,
    e_CustomerClassError=67
}mscc_CustomerPortStatus_e;


typedef union
{
    U16 Word;
    struct
    {
        U8 PortTurnedOn             :1; /* 0 */
        U8 PortTurnedOff            :1; /* 1 */
        U8 DetectionUnsuccessful    :1; /* 2 */
        U8 PortFault                :1; /* 3 */
        U8 PortWasInUnderLoad       :1; /* 4 */
        U8 PortWasInOverLoad        :1; /* 5 */
        U8 PortWasInPM              :1; /* 6 */
        U8 PortSpareEvent           :1; /* 7 */
        U8 DisconnectionTemperature :1; /* 8 */
        U8 UserDefinedTemperature   :1; /* 9 */
        U8 PoEDeviceFault           :1; /* 10 */
        U8 PoEDeviceSpareEvent      :1; /* 11 */
        U8 NoMoreConnect            :1; /* 12 */
        U8 VmainFault               :1; /* 13 */
        U8 SystemSpareEvent1        :1; /* 14 */
        U8 SystemSpareEvent2        :1; /* 15 */
    }Bits;
}mscc_ProtocolInterrutRegister_t;


typedef enum
{
    e_POE_STATUS_OK                                       =  0,
    e_POE_STATUS_ERR_POE_API_SW_INTERNAL                  = -1,
    e_POE_STATUS_ERR_COMMUNICATION_DRIVER_ERROR           = -2,
    e_POE_STATUS_ERR_COMMUNICATION_REPORT_ERROR           = -3,
    e_POE_STATUS_ERR_HOST_COMM_MSG_LENGTH_MISMATCH        = -4,
    e_POE_STATUS_ERR_PORT_NOT_EXIST                       = -5,
    e_POE_STATUS_ERR_MSG_NOT_READY                        = -6,
    e_POE_STATUS_UNKNOWN_ERR                              = -7,
    e_POE_STATUS_ERR_TIMER_INTERVAL_ERROR                 = -8,
    e_POE_STATUS_ERR_MUTEX_INIT_ERROR                     = -9,
    e_POE_STATUS_ERR_MUTEX_LOCK_ERROR                     = -10,
    e_POE_STATUS_ERR_MUTEX_UNLOCK_ERROR                   = -11,
    e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR                 = -12,
    e_POE_STATUS_ERR_MAX_ENUM_VALUE                       = -13,

}mscc_POE_STATUS_e;



typedef enum
{
    e_IC_Status_None,   /* No PoE Device is connected and also no PoE Device was expected to be connected */
    e_IC_Status_Ok,    /* The Configured PoE device is the one that is connected */
    e_IC_Status_Unexpected_PoE_detection1, /* Unexpected PoE Device was connected to the Sys and NO Save was done */
    e_IC_Status_Unexpected_PoE_detection2, /* Unexpected PoE Device was connected to the Sys and Save was done */
    e_IC_Status_Fail_Missing_PoE_Device, /* Missing PoE Device, No Save Done */
    e_IC_Status_Different_PoE_device_was_detected1 , /* Wrong PoE Device was connected to the Sys and NO Save was done */
    e_IC_Status_Different_PoE_device_was_detected2  /* Wrong PoE Device was connected to the Sys and Save was done */
} IC_Status_e;


typedef enum
{
    e_AF_Mode = 0,
    e_AT_Mode,
    e_MaxModeValue
}mscc_StandardMode_e;


typedef enum
{
	e_InternalPortPriority_Critical = 0,
	e_InternalPortPriority_High,
	e_InternalPortPriority_Low
}mscc_InternalPortPriorityLevel_e;


typedef struct
{
    /* IC parameters */
    U8   NumOfExpectedChannesInIC[MAX_IC_ON_BOARD];
    U8   IC_Address[MAX_IC_ON_BOARD];

    /* system parameters */
    U8   NumOfActiveICsInSystem;

    mscc_FPTR_Write fptr_write;
    mscc_FPTR_Read fptr_read;

    void *pUserData; /* user I2C communication driver data */

}mscc_InitInfo_t;


typedef struct
{
  /* FFU */
}mscc_CloseInfo_t;


typedef enum
{
    e_Class0=0,
    e_Class1,
    e_Class2,
    e_Class3,
    e_Class4
} ClassNum_e;


/*----------Layer2 definitions-------------------------------*/


	typedef enum
	{
		e_Power_Type_2_PSE ,
		e_Power_Type_2_PD ,
		e_Power_Type_1_PSE ,
		e_Power_Type_1_PD
	}mscc_power_type_e;


	typedef enum
	{
		e_power_source_Unknown ,
		e_power_source_Primary_power_source ,
		e_power_source_backup_source ,
		e_power_source_Reserved
	}mscc_PSE_power_source_e;



	typedef union
	{
		U8 Byte;
		struct
		{
			U8                    power_priority    :2;
			U8                    Reserved			:2;
			U8                    power_source 	    :2;
			U8                    power_type 		:2;
		}Bits;
	}mscc_Type_t;


#endif /* MSCC_POE_CODES_CONVERSION_H_ */
