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
 *  Description: contain global variables constants and types
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_DB_H_
	#define _MSCC_POE_DB_H_

	#ifdef __cplusplus
		extern "C" {
	#endif


	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
	#include "mscc_poe_global_types.h"


	/*=========================================================================
	/ CONSTANTS
	/========================================================================*/

	#define PRODUCT_NUMBER              9   /* for PD690xx */
	#define	SW_VERSION                  0x68 /* 1.0.4 */
	#define SYS_TYPE 					e_PD69000
	#define USER_CHECK_DEFAULT			0xa55a

	#define MASTER_TABLE_LENGTH  	    15
	#define NUM_PORTS_IN_GROUP          11

	#define ONE_WATT_FACTOR 			10
	#define CH_CURR_FACTOR 				61
	#define VMAIN_FACTOR 				164
	#define VMAIN_LSB_FACTOR 			61
	#define VPORT_FACTOR                168  /* for port on, the scale of the register is 2.44mv*24.3 = 59.29mv */

	#define	BASE_MAX_CHANNEL            24
	#define UNKNOWN_PRIORITY 			0
	#define L2_MAX_CABLE_LEN			100  /* in meters */
	#define L2_COUNTER_INIT				3  /* L2 counter maximum value */


	#define FAILED_EXECUTION_CONFLICT_IN_SUBJECT_BYTES(x) (1 + x) 	/* 0x0001-0x7FFF */
	#define FAILED_EXECUTION_WRONG_DATA_BYTE_VALUE(x)     (0x8001 + x) 	/* 0x8001-0x8FFF */
	#define FAILED_EXECUTION_UNDEFINED_KEY_VALUE   		  0XFFFF

	#define UPDATE_RESULT(varToUpdate, x)   ((varToUpdate == 0)&& (x != 0))? (varToUpdate = x):( varToUpdate = varToUpdate)


	/*=========================================================================
	/ TYPES
	/========================================================================*/


	typedef struct
	{
		U8 ModeRequest : 2;
		U8 IC_HW_support : 2;
		U8 IC_mode_status : 2;
	}IC_SupportData_t;



	typedef struct
	{
		 U8   NumOfChannelsInSystem;
		 U8   NumOfActiveICsInSystem;
		 U8   IC_Master_Index;
		 POE_BOOL   bPM_PowerDisconnentProcess;
	}SystemParams_t;


	typedef struct
	{
		 U8   NumOfChannesInIC;
		 U8   IC_Exp;
		 U8   IC_HW;
		 U8   IC_Valid;
		 U8   IC_Status;
		 U8   CurrentActiveSlaves;
		 U8   InitialActiveSlaves;
		 IC_SupportData_t   IC_SupportData;

		 U8   IC_Address;
		 U8   Customer_Private_Label;
		 U16  SysInterruptRegister;
	}IcParams_t;


	typedef union
	{
		U8 Byte;
		struct
		{
#if 0 /* big/little endian issue */
			U8 PSE_Enable	: 2;
			U8 PairControl	: 2;
			U8 PortMode 	: 2;
			U8 PortPriority : 2;
#else
			U8 PortPriority : 2;
			U8 PortMode     : 2;
			U8 PairControl  : 2;
			U8 PSE_Enable   : 2;
#endif
		}Bits;
	}PORT_CFG_t;




	typedef struct
	{
		U16 chCurrent;
	 	U16 chPower;
	 	U16 chVolt;
		U16 MainVolt;
	}PowerInfo_t;


	typedef union
	{
		U16 Word;
		struct
		{
			U8 PortOnMask 			:1;		/*[0]	port turned on*/
			U8 PortOffMask 			:1;		/*[1]	port turned off*/
			U8 DetectionFailedMask	:1;		/*[2]	detection failed*/
			U8 PortFaultMask		:1;		/*[3]	OVL or SC*/
			U8 PortUDLMask			:1;		/*[4]	Underload Detected*/
			U8 PortFaultDuringSU	:1;		/*[5]	OVL or SC during startup or DvDt fail*/
			U8 PortPMMask			:1;		/*[6]	port turned off due to PM*/
			U8 PowerDeniedMask		:1;		/*[7]	port power denied at startup*/
			U8 OverTempMask			:1;		/*[8]	over temp*/
			U8 TempAlarmMask		:1;		/*[9]	temp alarm*/
			U8 VmainLowAFMask		:1;		/*[10]	vmain low AF*/
			U8 VmainLowATMask		:1;		/*[11]	vmain low AT*/
			U8 VmainHighMask		:1;		/*[12]	vmain high*/
			U8 RPREventMask			:1;		/*[13]	RPR*/
			U8 SpareMask1			:1;		/*[14]	Spare1*/
			U8 SpareMask2			:1;		/*[15]	Spare2*/
		}Bits;
	}mscc_IntOutEvEn_t;



	typedef enum
	{
		e_DC_DiscoEn,				/* 0 */
		e_ExternalSyncDis,     		/* 1 */
		e_CapDis,     				/* 2 */
		e_DisPortsOverride,     	/* 3 */
		e_IcutMaxFlag,     			/* 4 */
		e_LSDEn_Or_IntOut,			/* 5 */
		e_PM_Mode,      			/* 6 */
		e_RPR_Disable,				/* 7 */
		e_DisAutoIntOut,			/* 8 */
		e_VmainATPolicyEn,			/* 9 */
		e_Class0EqAF,      			/* 10 */
		e_Class123EqAF,				/* 11 */
		e_ClassBypass2ndError,      /* 12 */
		e_ClassErrorEq0,      		/* 13 */
		e_ClassErrorEq4				/* 14 */
	}SysFlags_e;



	typedef enum
	{
		e_Alternative_A = 1,     	/* 1 */
		e_Alternative_B,     		/* 2 - backoff Enable */
	}PortPairControl_e;


	typedef struct
	{
		 U8 DeltaPower;
		 U8 PowerSourceType : 2;  /* power source type for each bank number */
	 }Power_t;





	/*----------Layer2 definitions-------------------------------*/

	/* Layer2 structure definition */
	typedef struct
	{
		U16 PDCount		  : 2;  /* indicates if port is LLDP enabled (!=0) or disabled(==0) */
		U8 CalcReq		  : 1;  /* mark port to be considered in the next run if LLDPAnalyzer() */
		U16 PDReqPower_dW : 10; /* PD request of power */
		U16 ConvertedPDReqPower_dW  	  : 10; /* current TPPL of a port */
		U8 CableLen;            /* cable length received from PD */
	}Layer2_t;


	typedef struct
	{
		U8 ChNum;
		U16 Allocated;
	} Layer2Reduction_t;


	/* all flags are left the same, so the individual mask cmd in the protocol won't have to be changed */
	typedef struct
	{
		U8 AC_DisEnable				:1;/* High - AC_Dis Work - when inactive DC disconnect is active */
		U8 Back_off					:1;/* High - Back_off */
		U8 Ignore_priority          :1;
		U8 Layer2					:1;/* [46] Enable Layer2 operation	Layer2PriorityByPD */
		U8 Layer2PriorityByPD		:1;/* [47] High- If Layer2 operation is enabled, Port Priority can be defined by PD. */
	}Masks_t;


	typedef struct
	{
		 Layer2_t              L2Data;
		 U8                    Priority         :2;
		 U8                    Standard			:1; /* 0 - AF, 1 - AT */

		 U8 			 	   mscc_Vir2PhyArrActiveMatrix;
		 U8 			 	   mscc_Vir2PhyArrTemporaryMatrix;

	} ChannelParams_t;

extern	U16                    ClassPower_mW[5];

extern	U8                     L2_Sync_Flag ;
extern	U8                     PortsPriorityChanged;
extern	U8                     PriorityTable[MAX_CH_PER_SYSTEM];

extern	Layer2Reduction_t	   L2Reduction;
extern	U8                     bStaticPM_CalcMode;

extern	SystemParams_t  	   SystemParams;
extern	IcParams_t             IcParams[MAX_IC_ON_BOARD];
extern	ChannelParams_t		   ChannelParams[MAX_CH_PER_SYSTEM];

extern	Power_t 	           Budget[MAX_POWER_BUDGET];
extern	Masks_t                Masks;
extern	U8                     PriorityStep;

	/* status */
extern	S32 				   result;
extern	S32 				   tmpResult;
extern	S32					   *pDeviceErrorInternal;


#endif  /* _MSCC_POE_DB_H_ */
