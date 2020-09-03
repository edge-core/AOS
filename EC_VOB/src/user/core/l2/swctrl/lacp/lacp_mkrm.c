/* ------------------------------------------------------------------------+
 * FILE NAME - lacp_mkrm.c                                                 |
 * ------------------------------------------------------------------------+
 * Abstract: This file contains the Marker machine of LACP                 |
 * ------------------------------------------------------------------------+
 *       Project    : MC2                                                  |
 *       Written by : Chi-kuo Hsu                                          |
 *       Date       : 2001/10/04                                           |
 *                                                                         |
 * Modification History:                                                   |
 *   By      Date        Ver.   Modification Description                   |
 *   ------- ----------  -----  -------------------------------------------|
 *   ckhsu   2001/12/10         Create for Mercury from MC2.               |
 * ------------------------------------------------------------------------|
 * Copyright(C)                              ACCTON Technology Corp., 2001 |
 * ------------------------------------------------------------------------|
 * NOTE :                                                                  |
 *-------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_module.h"

#include "lacp_type.h"
#include "lacp_util.h"

#include "l_mm.h"

/* NAMING CONSTANT DECLARARTIONS */

/* Type definitions  */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Marker_Machine
 *------------------------------------------------------------------------
 * FUNCTION: This function is a machine map to Marker protocol.
 * INPUT   : pPort -- pointer to the port structure.
 *           event -- event to marker machine.
 *           pPdu  -- pointer to received Pdu.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Marker_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event, LACP_PDU_U *pPdu)
{
    LACP_PDU_U *reply_Pdu;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T      pdu_len;

    switch(event)
    {
    case LACP_RX_PDU_EV:
        /* Here we might receive 2 kinds of frames. One is Marker Information and */
        /* the other is Marker response.                                          */
        if(LACP_Check_PDU( pPdu, LACP_MARKER_INFO_PDU) == FALSE)
        {
            (pPort->AggPortStats).IllegalRx++;
            return;
        }

        /* We are not going to check anything since in RxMachine, we have already */
        /* did that.                                                              */
        switch((pPdu->marker).tlv_type_marker_infomation)
        {
        case LACP_TLV_TYPE_MARKER_INFORMATION:
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(LACP_PDU_U),
                                                  L_MM_USER_ID2(SYS_MODULE_LACP, LACP_TYPE_TRACE_ID_LACP_MARKER_MACHINE));
            reply_Pdu = (LACP_PDU_U*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(reply_Pdu==NULL)
            {
                printf("\nLACP_Marker_Machine::ERROR!! for L_MM_Mref_GetPdu return NULL");
                return;
            }
            mref_handle_p->current_usr_id = SYS_MODULE_LACP;

            /* If we receive a Marker Information PDU, then we have to response */
            /* it in the same port.                                             */
            memset( reply_Pdu, 0x0, sizeof(LACP_PDU_U));

            /* Set system Id */
            memcpy( (reply_Pdu->marker).request_system, (pPdu->marker).request_system, 0x6);
            /* Set system transaction number. */
            (reply_Pdu->marker).request_transaction_id = (pPdu->marker).request_transaction_id;

            LACP_Generate_PDU( pPort, reply_Pdu, LACP_MARKER_RESPONSE_PDU);

            /* Bug fix...................................
               Requestor port should be the same as Pdu.
               Add the following 3 lines because when generating Pdu, it should not modify the
               Pdu content.  Also remember to copy system id and trans id.
             */
            (reply_Pdu->marker).request_port = (pPdu->marker).request_port;
            memcpy( (reply_Pdu->marker).request_system, (pPdu->marker).request_system, 0x6);
            (reply_Pdu->marker).request_transaction_id = (pPdu->marker).request_transaction_id;

            (pPort->AggPortStats).MarkerPDUsRx++;
            (pPort->AggPortStats).MarkerResponsePDUsTx++;

            /* This also count as a PDU transmition. */
            pPort->FramesInLastSec++;

            LACP_TX_SendPDU( mref_handle_p, pPort->slot, pPort->port);

            break;
        case LACP_TLV_TYPE_MARKER_RESPONSE:
            /* We are not going to process marker response since we didnot use this */
            /* in this version of implementation.                                   */
            (pPort->AggPortStats).MarkerResponsePDUsRx++;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

