/* ------------------------------------------------------------------------+
 * FILE NAME - lacp_txm.c                                                  |
 * ------------------------------------------------------------------------+
 * Abstract: This file contains the transmit machine and periodic transmit |
 *           machine of LACP.                                              |
 * ------------------------------------------------------------------------+
 *       Project    : MC2                                                  |
 *       Written by : Chi-kuo Hsu                                          |
 *       Date       : 2001/09/21                                           |
 *                                                                         |
 * Modification History:                                                   |
 *   By        Date     Ver.   Modification Description                    |
 *   ------- --------   -----  --------------------------------------------|
 *                                                                         |
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
/* static LACP_PDU_U  LACP_TxPdu[MAX_PHYSICAL_PORTS]; */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Tx_Machine
 *------------------------------------------------------------------------
 * FUNCTION: This function is Transmit Machine defined in 802.3ad
 * INPUT   : pPort -- pointer to the port structure.
 *           event -- event to Transmit Machine.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Tx_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event)
{
    LACP_PDU_U *pPdu;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    switch(event)
    {
    case LACP_RX_PORT_UP_EV:
    case LACP_RX_PORT_DOWN_EV:
    case LACP_SYS_INIT:
        /* Here we only need to initialize ntt flag.    */
        pPort->NTT = FALSE;
        break;
    case LACP_TIMER_EV:
        /* Reinitialize variable.   */
        pPort->FramesInLastSec = 0;
        if(!pPort->NTT)
        {
            break;
        }
        /* This means we have frame need to transmit. Fall down...            */
    case LACP_TRANSMIT_EV:
        /* Before we transmit any PDU, we need to check whether how many PDUs */
        /* have already transmitted in this port in past one second.          */
        if(!pPort->LACPEnabled)
        {
            /* If LACP is DISABLED then we do not need to send any packets */
            pPort->NTT = FALSE;
            break;
        }

        if( (pPort->FramesInLastSec < LACP_MAX_FRAMES_IN_ONE_SECOND) &&
            (pPort->NTT) &&
            (pPort->Port_OperStatus) &&
            (pPort->LACP_OperStatus) )
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(LACP_PDU_U),
                                                  L_MM_USER_ID2(SYS_MODULE_LACP, LACP_TYPE_TRACE_ID_LACP_TX_MACHINE));
            pPdu = (LACP_PDU_U*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(pPdu==NULL)
            {
                printf("\r\nLACP_Tx_Machine::ERROR!! for L_MM_Mref_GetPdu return NULL");
                return;
            }
            mref_handle_p->current_usr_id = SYS_MODULE_LACP;

            memset( pPdu, 0x0, sizeof(LACP_PDU_U));

            LACP_Generate_PDU( pPort, pPdu, LACP_LAC_PDU);

            /* We transmit one more LACPDU. */
            (pPort->AggPortStats).LACPDUsTx++;
            pPort->FramesInLastSec++;
            pPort->NTT = FALSE;

            LACP_TX_SendPDU( mref_handle_p, pPort->slot, pPort->port);
        }
        break;
    default:
        break;
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Periodic_Machine
 *------------------------------------------------------------------------
 * FUNCTION: This function is Periodic Transmit Machine defined in 802.3ad
 * INPUT   : pPort -- pointer to the port structure.
 *           event -- event to Transmit Machine.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Periodic_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event)
{
    /* Actually, I do not think that we need to process the port_enable */
    /* events etc. In fact, I check it every time when receive tick.    */
    switch(event)
    {
    case LACP_RX_PORT_DOWN_EV:
    case LACP_PORT_DISABLED_EV:
    case LACP_SYS_INIT:
        /* Here we need to modify timer related parameter. */
        LACP_Stop_Periodic_Timer( pPort);
        break;
    case LACP_TIMER_EV:
        if( !( (pPort->LACP_OperStatus) &&
               (pPort->Port_OperStatus) &&
               /*((((pPort->AggActorPort).state).lacp_port_state).LACP_Activity || (((pPort->AggPartnerPort).state).lacp_port_state).LACP_Activity) ) )*/
               (   LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity)
                || LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity)
               )
             )
          )
        {
            LACP_Stop_Periodic_Timer( pPort);
            break;
        }

        /* Check periodic timer here. */
        if(LACP_Is_Periodic_Timer_Enabled( pPort) == TRUE)
        {
            /* If timer is enabled, then we process this.       */
            LACP_Decrease_Periodic_Timer( pPort);
            /* then we have to check timer is expired or not.   */
            if(LACP_Is_Periodic_Timer_Expired( pPort))
            {
                /* If timer is expired, then we have to set NTT to true. */
                /* do different things.                                  */
                pPort->NTT = TRUE;
                /*LACP_Start_Periodic_Timer( pPort, (((pPort->AggPartnerPort).state).lacp_port_state).LACP_Timeout);*/
                LACP_Start_Periodic_Timer(pPort, LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout));
            }
            else
            {
                /* We have to check the timer again here.                */
                /* If the timer parameter changed, then set NTT to TRUE. */
                /*if(((((pPort->AggPartnerPort).state).lacp_port_state).LACP_Timeout) == LACP_SHORT_TIMEOUT)*/
                if (LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout) == LACP_SHORT_TIMEOUT)
                {
                    pPort->NTT = TRUE;
                    /*LACP_Start_Periodic_Timer( pPort, (((pPort->AggPartnerPort).state).lacp_port_state).LACP_Timeout);*/
                    LACP_Start_Periodic_Timer(pPort, LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout));
                }
            }

        }
        else
        {
            /* If timer is not enabled, then e have to enabled it. */
            /*LACP_Start_Periodic_Timer( pPort, (((pPort->AggPartnerPort).state).lacp_port_state).LACP_Timeout);*/
            LACP_Start_Periodic_Timer(pPort, LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout));
        }
        break;
    default:
        break;
    }

}
