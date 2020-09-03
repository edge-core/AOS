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
 *  Description: the protocol file Analysis And Execution the Commands
 *
 *
 *************************************************************************/

#ifndef _MSCC_POE_COMM_PROTOCOL_H_
	#define _MSCC_POE_COMM_PROTOCOL_H_

	#ifdef __cplusplus
		extern "C" {
	#endif


	/*=========================================================================
	/ INCLUDES
	/========================================================================*/
	#include "mscc_poe_global_types.h"
	#include "mscc_arch_functions.h"


	/*=========================================================================
	/  CONSTANTS
	/ =======================================================================*/
	#define mscc_DO_NOT_SEND_REPORT 		0x7fff
	#define mscc_PROTOCOL_ERROR 			MAX_WORD
	#define mscc_MAX_LENGHT_ALLOWED			26
	#define mscc_DONT_CHANGE_VOLT			0xFFFF
	#define	mscc_MAX_DECIWATT_VALUE_IN_BYTE 25500  /* maximum power for old port power request */
	#define mscc_ASIC_POWER_LIMIT_FACTOR 	10749 /* factor sum of square currents from protocol cmd: 1000/(0.305^2) */


	/*=========================================================================
	/ TYPES
	/=========================================================================*/


    typedef enum
    {
		e_InternalPortStatus_On,                /*      0 -  PortOn - Port was turned on due to a valid signature (res \ cap) */
		e_InternalPortStatus_PortOnTM,          /*      1 -  PortOnTM - Port was turned on due to Force Power */
		e_InternalPortStatus_Startup,           /*      2 -  Startup - Port is in startup */
		e_InternalPortStatus_StartupTM ,        /*      3 -  StartupTM - Port is in startup as force power */
		e_InternalPortStatus_Searching ,        /*      4 -  Searching - Port needs detection or during detection */
		e_InternalPortStatus_InvalidSig,        /*      5 -  InvalidSig - Invalid signature has been detected */
		e_InternalPortStatus_ClassError,        /*      6 -  ClassError - Error in classification has been detected */
		e_InternalPortStatus_TestMode ,         /*      7 -  TestMode - Port needs to be turned on as Test Mode  Force Power */
		e_InternalPortStatus_ValidSig,          /*      8 -  ValidSig - A valid signature has been detected */
		e_InternalPortStatus_Disabled ,         /*      9 -  Disabled -  Port is disabled */
		e_InternalPortStatus_StartupOVL,        /*      10 -  StartupOVL - Overload during startup */
		e_InternalPortStatus_StartupUDL ,       /*      11 - StartupUDL - Underload during startup */
		e_InternalPortStatus_StartupShort,      /*      12 - StartupShort - Short during startup */
		e_InternalPortStatus_DvDtFail ,         /*      13 - DvDtFail - Failure in the Dv/Dt algorithm */
		e_InternalPortStatus_TestError ,        /*      14 - TestError - Port was turned on as Test Mode and has error */
		e_InternalPortStatus_OVL ,              /*      15 - OVL - Overload detected */
		e_InternalPortStatus_UDL ,              /*      16 - UDL - Underload detected */
		e_InternalPortStatus_ShortCircuit ,     /*      17 - ShortCircuit - Short circuit detected */
		e_InternalPortStatus_PM ,               /*      18 - PM  port was turned off due to PM */
		e_InternalPortStatus_SysDisabled,       /*      19 - SysDisabled - Chip level error */
		e_InternalPortStatus_Unknown,           /*      20 - Unknown - General chip error */
		e_InternalPortStatus_CustomerUnDefined
    }InternalPortStatus_e;


    typedef enum
    {
    	e_ExternalPortStatus_Disabled,                   /* 000 */
    	e_ExternalPortStatus_Searching,                  /* 001 */
    	e_ExternalPortStatus_DeliveringPower,            /* 010 */
    	e_ExternalPortStatus_TestMode,                   /* 011 */
    	e_ExternalPortStatus_TestError,                  /* 100 */
    	e_ExternalPortStatus_ImplementationSpecific      /* 101 */
    }ExternalPortStatus_e;


    /*=========================================================================
    / PROTOTYPES
    /========================================================================*/
    S32 mscc_AnalysisAndExecutionCommand(_IN U8 *pDataIn);
   	void mscc_SendReport(_IN U16 Status);

#endif  /* _MSCC_POE_COMM_PROTOCOL_H_ */
