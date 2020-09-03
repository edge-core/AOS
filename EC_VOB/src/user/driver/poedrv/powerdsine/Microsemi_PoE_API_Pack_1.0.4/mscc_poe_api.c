/*************************************************************************
 *
 *            Copyright (c) 2008 by Microsemi Corp. Inc.
 *
 *  This software is copyrighted by, and is the sole property of Microsemi
 *  Corp. All rights, title, ownership, or other interests in the
 *  software remain the property of Microsemi Corp. This software
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
 *************************************************************************/


/*=========================================================================
/ INCLUDES
/========================================================================*/

#include "mscc_poe_comm_protocol.h"
#include "mscc_poe_ic_func.h"
#include "mscc_poe_host_communication.h"
#include "mscc_poe_util.h"
#include "mscc_poe_api.h"


/*=========================================================================
/ FUNCTIONS
/========================================================================*/


/*---------------------------------------------------------------------
 *    description:     This command inits PoE software - database and communication components.
 *
 *    input :   pMscc_InitInfo	  - pointer to struct InitInfo_t which contain data required for
 * 									PoE API software initialization.
 *	  output:	pDevice_error     - contain the error code of the host driver in case of errors
 * 																							in i2c read or i2c write operations.
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    < e_POE_STATUS_OK     - operation failed
 *---------------------------------------------------------------------*/
mscc_POE_STATUS_e MSCC_POE_Init(_IN mscc_InitInfo_t *pMscc_InitInfo,_OUT S32 *pDevice_error)
{
	S32 localResult = e_POE_STATUS_OK;

	/* try to take and lock the mutex */
	localResult = OS_mutex_lock();       /* down semaphore */
	if(localResult != e_POE_STATUS_OK)
		return localResult;

	/* START CRITICAL REGION */

	pDeviceErrorInternal = pDevice_error;
	*pDeviceErrorInternal = e_POE_STATUS_OK;

	tmpResult = mscc_InitSoftware(pMscc_InitInfo);  /* INIT POE SW  */
	localResult = mscc_POE_UTIL_GetPOE_STATUS_ERR_value(_IN tmpResult);

	/* END CRITICAL REGION */

	tmpResult = OS_mutex_unlock();       /* up semaphore */
	UPDATE_RESULT(localResult,tmpResult);

	return localResult;
}



/*---------------------------------------------------------------------
 *    description:     Write 15 bytes message from the Host to the PoE API software.
 *
 *    input :   pTxdata           - 15 bytes message data array
 * 				num_write_length  - number of bytes to write - message length - must be 15.
 *
 * 	  output:	pDevice_error     - contain the error code of the host driver in case of errors
 * 									in read or write operations.
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    < e_POE_STATUS_OK     - operation failed
 *---------------------------------------------------------------------*/
mscc_POE_STATUS_e MSCC_POE_Write(_IN U8* pTxdata,_IN U16 num_write_length,_OUT S32 *pDevice_error)
{
	S32 localResult = e_POE_STATUS_OK;

	if(num_write_length != PACKAGE_SIZE)
		return e_POE_STATUS_ERR_HOST_COMM_MSG_LENGTH_MISMATCH;

	/* try to take and lock the mutex */
	localResult = OS_mutex_lock();       /* down semaphore */
	if(localResult != e_POE_STATUS_OK)
		return localResult;

	/* START CRITICAL REGION */

	result = e_POE_STATUS_OK;
	pDeviceErrorInternal = pDevice_error;
	*pDeviceErrorInternal = e_POE_STATUS_OK;

	tmpResult = mscc_AnalysisAndExecutionCommand(pTxdata); /* decode 15 bytes Tx cmd ,access PoE IC's and place result in reply buffer.  */
	localResult = mscc_POE_UTIL_GetPOE_STATUS_ERR_value(_IN tmpResult);

	/* END CRITICAL REGION */

	tmpResult = OS_mutex_unlock();       /* up semaphore */
	UPDATE_RESULT(localResult,tmpResult);

	return localResult;
}


/*---------------------------------------------------------------------
 *    description:     read 15 bytes message from the PoE API software to the Host.
 *
 *    input :   num_read_length   - number of bytes to read - message length - must be 15.
 *
 *    output:   pRxdata           - 15 bytes message data array
 *				pDevice_error     - contain the error code of the host driver in case of errors
 * 									in read or write operations.
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    < e_POE_STATUS_OK     - operation failed
 *---------------------------------------------------------------------*/
mscc_POE_STATUS_e MSCC_POE_Read (_OUT U8* pRxdata,_IN U16 num_read_length,_OUT S32 *pDevice_error)
{
	S32  localResult = e_POE_STATUS_OK;

	if(num_read_length != PACKAGE_SIZE)
		localResult = e_POE_STATUS_ERR_HOST_COMM_MSG_LENGTH_MISMATCH;

	/* try to take and lock the mutex */
	localResult = OS_mutex_lock();       /* down semaphore */
	if(localResult != e_POE_STATUS_OK)
		return localResult;

	/* START CRITICAL REGION */

	result = e_POE_STATUS_OK;
	pDeviceErrorInternal = pDevice_error;
	*pDeviceErrorInternal = e_POE_STATUS_OK;

	tmpResult = mscc_POE_HOST_HostReadMsgFromTxBuffer(pRxdata);
	localResult = mscc_POE_UTIL_GetPOE_STATUS_ERR_value(_IN tmpResult);

	/* END CRITICAL REGION */

	tmpResult = OS_mutex_unlock();       /* up semaphore */
	UPDATE_RESULT(localResult,tmpResult);

	return localResult;
}



/*---------------------------------------------------------------------
 *    description:     This command Close the operation of the PoE software.
 * 					   the command is FFU.
 *
 *    input :   mscc_CloseInfo_t  - pointer to struct mscc_CloseInfo_t which contain data required for
 * 									closing the PoE API software.
 *	  output:	pDevice_error     - contain the error code of the host driver in case of errors
 * 									in read or write operations.
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    < e_POE_STATUS_OK    - operation failed
 *---------------------------------------------------------------------*/
mscc_POE_STATUS_e MSCC_POE_Exit(_IN mscc_CloseInfo_t *pMscc_CloseInfo,_OUT S32 *pDevice_error)
{
	pDeviceErrorInternal = pDevice_error;
	*pDeviceErrorInternal = e_POE_STATUS_OK;

	/* FFU */

	return e_POE_STATUS_OK;
}



/*---------------------------------------------------------------------
 *    description:     Timer function - one tick for a second.
 *
 *    input :   IntervalTime_Sec  - the interval of the timer - must be 1 second.
 *
 *    output:	pDevice_error     - contain the error code of the host driver in case of errors
 * 									in read or write operations.
 *
 *    return:   e_POE_STATUS_OK       - operation succeed
 * 			    < e_POE_STATUS_OK     - operation failed
 *---------------------------------------------------------------------*/
mscc_POE_STATUS_e MSCC_POE_Timer_Tick(_IN U8 IntervalTime_Sec,_OUT S32 *pDevice_error)
{
	S32  localResult = e_POE_STATUS_OK;

	if(IntervalTime_Sec != 1)
		localResult = e_POE_STATUS_ERR_TIMER_INTERVAL_ERROR;

	/* try to take and lock the mutex */
	localResult = OS_mutex_lock();       /* down semaphore */
	if(localResult != e_POE_STATUS_OK)
		return localResult;

	/* START CRITICAL REGION */

	result = e_POE_STATUS_OK;
	pDeviceErrorInternal = pDevice_error;
	*pDeviceErrorInternal = e_POE_STATUS_OK;

	tmpResult = mscc_POE_CMD_LLDP_AndLayer2PM();
	UPDATE_RESULT(localResult,tmpResult);

    if(SystemParams.bPM_PowerDisconnentProcess == POE_FALSE)
    {
    	tmpResult = mscc_POE_CMD_IgnoreHighPriority();
    	UPDATE_RESULT(localResult,tmpResult);
    }
    else
    	PriorityStep = 0;

    localResult = mscc_POE_UTIL_GetPOE_STATUS_ERR_value(_IN tmpResult);

	/* END CRITICAL REGION */

	tmpResult = OS_mutex_unlock();       /* up semaphore */
	UPDATE_RESULT(localResult,tmpResult);

	return localResult;
}
