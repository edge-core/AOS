/* ------------------------------------------------------------------------+
 * FILE NAME - lacp_rxm.c                                                  |
 * ------------------------------------------------------------------------+
 * Abstract: This file contains the receive machine of LACP                |
 * ------------------------------------------------------------------------+ 
 *       Project    : MC2                                                  |
 *       Written by : Chi-kuo Hsu                                          |
 *       Date       : 2001/09/20                                           |
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

#include "lacp_type.h"
#include "lacp_util.h"

/* NAMING CONSTANT DECLARARTIONS */

/* Type definitions  */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void LACP_RxMachineUpdateInfo( LACP_PORT_T *pPort);

/* The following are finite state machine functions, we have to enter   */
/* it when we are insome situations.                                    */
static void LACP_RxMachine_Initialize( LACP_PORT_T *pPort);
static void LACP_RxMachine_PortDisabled( LACP_PORT_T *pPort);
static void LACP_RxMachine_LacpDisabled( LACP_PORT_T *pPort);
static void LACP_RxMachine_Expired( LACP_PORT_T *pPort);
static void LACP_RxMachine_Defaulted( LACP_PORT_T *pPort);
static void LACP_RxMachine_Current( LACP_PORT_T *pPort,  LACP_PDU_U *pPdu);

static RX_MACHINE_STATE_E LACP_Get_RxMachine_Current_State( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_Rx_Machine
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is Receive Machine defined in 802.3ad
 * INPUT   : pPort -- pointer to the port structure.
 *           event -- event to Transmit Machine.
 *           pPdu  -- pointer to Pdu received. If it is not receive evt,
 *                    then it will not be used.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Rx_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event, LACP_PDU_U *pPdu)
{
    switch(event)
    {
    case LACP_SYS_INIT:
    case LACP_RX_INIT_EV:
        /* No matter in what situation we get into here, we need to re-initialize */
        /* this port.                                                             */
        LACP_RxMachine_Initialize( pPort);
        /* Since some information is changed. So we have to inform other state machine */
        /* to do their necessary actions.                                              */
        if( (pPort->LACP_OperStatus) &&
            (pPort->Port_OperStatus) )
        {
            LACP_RxMachine_Expired( pPort);
        }
        break;
    case LACP_RX_PORT_UP_EV:
    	/* Since we have DuplexMode callback, so it may be unneccessary to check/deal with Duplex mode here
    	if(SWCTRL_GetPortInfo(LACP_LINEAR_ID( pPort->slot, pPort->port), &port_info) != FALSE)
 	{
 	
 	}*/
        /* Fall down ... */
    case LACP_PORT_ENABLED_EV:
        /* port enable */
        if(pPort->LACP_OperStatus)
        {
            /* Initialize LACP */
            LACP_Rx_Machine( pPort, LACP_RX_INIT_EV, NULL);
        }
        else
        {
            /* Goto LACP disabled state. */
            LACP_RxMachine_LacpDisabled( pPort);
        }
        break;
    case LACP_RX_PORT_DOWN_EV:
        /* Fall down... */
    case LACP_PORT_DISABLED_EV:
        /* port disabled is considered as need to initialization.                */
        /* When this happened, we have to stop the timer because this may happen */
        /* at every time.                                                        */
        LACP_RxMachine_Initialize  ( pPort);
        LACP_RxMachine_PortDisabled( pPort);
        break;
    case LACP_NEW_INFO_EV: /* won't happen */
        /* This should come from some signal, means something is changed. */
        LACP_RxMachineUpdateInfo( pPort);
        break;
    case LACP_TIMER_EV:
        /* Remember to decrease the timer. */
        if(LACP_Is_Current_While_Timer_Enabled( pPort) == TRUE)
        {
            /* If timer is enabled, then we process this. */
            LACP_Decrease_Current_While_Timer( pPort);
            /* then we have to check timer is expired or not. */
            if(LACP_Is_Current_While_Timer_Expired( pPort))
            {
                /* This is one time timer. */
                LACP_Stop_Current_While_Timer( pPort);

                /* If timer is expired, then we have to check we are in which state then */
                /* do different things.                                                  */
                /* Case 1, if we are in                                                  */
                if(LACP_Get_RxMachine_Current_State( pPort) == Rxm_CURRENT)
                {
                    /* Since we are in current state, then we have to switch to */
                    /* EXPIRED state.                                           */
                    LACP_RxMachine_Expired( pPort);
                }
                else if(LACP_Get_RxMachine_Current_State( pPort) == Rxm_EXPIRED)
                {
                    /* Enter DEFAULTED state */
                    LACP_RxMachine_Defaulted( pPort);
                }
                else
                {
                    /* Actually we should not here, at this situation, we just ignore it. */
                }
            }
        }
        break;
    case LACP_RX_PDU_EV:
        /* We are receiving PDU, but process that when LACP is enabled. */
        if( (pPort->LACP_OperStatus) &&
            (pPort->Port_OperStatus) )
        {
            /* First check type. */
            #if 0
            /* Currently not support in Mercury because when we receive this Pdu */
            /* we can not know its ethertype. This should support by LAN.        */
            if((pPdu->lacp).ethertype != LACPDU_Slow_Protocols_Type)
            {
                /* break this case since this is a unknown Rx event. */
                (pPort->AggPortStats).UnknownRx++;
                break;
            }
            #endif
            
            switch((pPdu->lacp).protocol_subtype)
            {
            case LACP_LACP:
                /* LACP frame. Now we need to check sub-type. */
                if(LACP_Check_PDU( pPdu, LACP_LAC_PDU) == FALSE)
                {
                    (pPort->AggPortStats).IllegalRx++;
                    return;
                }
                /* This is a correct LACPDU. */
                (pPort->AggPortStats).LACPDUsRx++;
                
                /* All Pdus are receiving here, so we have to check the state here. */
                switch(LACP_Get_RxMachine_Current_State( pPort))
                {
                /* We only process PDU when in CURRENT and DEFAULTED and Rxm_EXPIRED state. */
                case Rxm_CURRENT:
                case Rxm_DEFAULTED:
                case Rxm_EXPIRED:
                    /* Since this is a union, we can just cast it and then pass in. */
                    LACP_RxMachine_Current( pPort, pPdu);
                    break;
                default:
                    break;
                }
                break;
            case LACP_MARKER:
                LACP_Marker_Machine( pPort, LACP_RX_PDU_EV, pPdu);
                break;
            default:
                /* Illegal type. */
                (pPort->AggPortStats).IllegalRx++;
                break;
            }
        }
        break;
    default:
        break;
    }
    
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachineUpdateInfo
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is trying to update Receive Machine.
 *           Now it is not used but leave it alone.
 * INPUT   : pPort -- pointer to the port structure.
 *           event -- event to Transmit Machine.
 *           pPdu  -- pointer to Pdu received. If it is not receive evt,
 *                    then it will not be used.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachineUpdateInfo( LACP_PORT_T *pPort)
{
    switch((pPort->AggPortDebug).RxState)
    {
    case Rxm_INITIALIZE:
        LACP_RxMachine_PortDisabled( pPort);
        LACP_RxMachineUpdateInfo( pPort);
        break;
    case Rxm_PORT_DISABLED:
        if(pPort->PortEnabled)
        {
            if(pPort->LACPEnabled)
            {
                LACP_RxMachine_Expired( pPort);
            }
            else
            {
                LACP_RxMachine_LacpDisabled( pPort);
            }
        }
        else
        {
            LACP_RxMachine_PortDisabled( pPort);
        }
        break;
    case Rxm_LACP_DISABLED:
        if(pPort->PortEnabled)
        {
            if(pPort->LACPEnabled)
            {
                LACP_RxMachine_Expired( pPort);
            }
            else
            {
                LACP_RxMachine_LacpDisabled( pPort);
            }
        }
        else
        {
            LACP_RxMachine_PortDisabled( pPort);
        }
        break;
    case Rxm_CURRENT:
    case Rxm_EXPIRED:
        break;
    case Rxm_DEFAULTED:
        break;
    default:
        LACP_RxMachine_Initialize( pPort);
        break;
    }
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachine_Initialize
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is doing actions defined in 802.3ad when Rx
 *           Machine transit to Initialization state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachine_Initialize( LACP_PORT_T *pPort)
{
    LACP_Stop_Current_While_Timer( pPort);
    
    pPort->Selected = UNSELECTED;
    /* Although in 802.3ad we didnot have this step, but actually we need, else */
    /* how can we set the administrative value into the operation value.        */
    LACP_recordActorDefault( pPort);
    LACP_recordDefault( pPort);
    /*(((pPort->AggActorPort).state).lacp_port_state).Expired = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Expired);
    /* I am not sure that we need a port-moved variable here. */
    pPort->PortMoved = FALSE;
        
    (pPort->AggPortDebug).RxState = Rxm_INITIALIZE;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachine_PortDisabled
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is doing actions defined in 802.3ad when Rx
 *           Machine transit to PortDisabled state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachine_PortDisabled( LACP_PORT_T *pPort)
{
    pPort->Matched = FALSE;  /*Lewis: Matched means (Same Partner or Individual) & One side is Actively */
						    /* Matched is used with received PDU's Actor's Sync to decide Part.Sync in Mux machine */
    /*(((pPort->AggPartnerPort).state).lacp_port_state).Synchronization = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Synchronization);
    (pPort->AggPortDebug).RxState = Rxm_PORT_DISABLED;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachine_Expired
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is doing actions defined in 802.3ad when Rx
 *           Machine transit to Expired state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachine_Expired( LACP_PORT_T *pPort)
{
/* Lewis Mark out the following */
    pPort->Matched  = FALSE; /*Lewis: Matched means (Same Partner or Individual) & One side is Actively */
                                           /* Lewis changes it to TRUE from FALSE here, since it is used against part's sync and partner may be individual */
                                           /* Matched is used with received PDU's Actor's Sync to decide Part.Sync in Mux machine */
    /*(((pPort->AggPartnerPort).state).lacp_port_state).Synchronization = FALSE;*/
    /*(((pPort->AggPartnerPort).state).lacp_port_state).LACP_Timeout    = LACP_SHORT_TIMEOUT;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Synchronization);
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, ((BOOL_T)(LACP_SHORT_TIMEOUT)), LACP_dot3adAggPortActorOperState_LACP_Timeout);
    LACP_Start_Current_While_Timer( pPort, LACP_SHORT_TIMEOUT);
    /*(((pPort->AggActorPort).state).lacp_port_state).Expired           = TRUE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(TRUE)), LACP_dot3adAggPortActorOperState_Expired);
    (pPort->AggPortDebug).RxState = Rxm_EXPIRED;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachine_LacpDisabled
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is doing actions defined in 802.3ad when Rx
 *           Machine transit to LacpDisabled state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachine_LacpDisabled( LACP_PORT_T *pPort)
{
    pPort->Matched  = TRUE; /*Lewis: Matched means (Same Partner or Individual) & One side is Actively */
    							/* Lewis changes it to FALSE from TRUE, since it's unselected not in MUX attached and collecting state */
    							/* Matched is used with received PDU's Actor's Sync to decide Part.Sync in Mux machine */
    pPort->Selected = UNSELECTED;
    LACP_recordActorDefault( pPort);
    LACP_recordDefault( pPort);
    /*(((pPort->AggPartnerPort).state).lacp_port_state).Aggregation = FALSE;*/ /* Individual */
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Aggregation);
    
    (pPort->AggPortDebug).RxState = Rxm_LACP_DISABLED;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachine_Defaulted
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is doing actions defined in 802.3ad when Rx
 *           Machine transit to Defaulted state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachine_Defaulted( LACP_PORT_T *pPort)
{
/* Lewis Marked out the following */
    pPort->Matched  = TRUE; /*Lewis: Matched means (Same Partner or Individual) & One side is Actively */
						    /* Matched is used with received PDU's Actor's Sync to decide Part.Sync in Mux machine */
    LACP_Stop_Current_While_Timer( pPort);
    LACP_Update_Default_Selected( pPort);
    LACP_recordDefault( pPort);
    
    /*(((pPort->AggActorPort).state).lacp_port_state).Expired = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Expired);
    (pPort->AggPortDebug).RxState = Rxm_DEFAULTED;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_RxMachine_Current
 *------------------------------------------------------------------------ 
 * FUNCTION: This function is doing actions defined in 802.3ad when Rx
 *           Machine transit to Current state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_RxMachine_Current( LACP_PORT_T *pPort, LACP_PDU_U *pPdu)
{
    LACP_Update_Selected( pPort, pPdu);
    LACP_Update_NTT( pPort, pPdu);
    LACP_recordPDU( pPort, pPdu);
    
    /* Although this is not inside standard, but actually this is what the rest of */
    /* recordPDU.    
    */
    LACP_Choose_Matched( pPort, pPdu);
    
    /*LACP_Start_Current_While_Timer( pPort, (((pPort->AggActorPort).state).lacp_port_state).LACP_Timeout);*/
    LACP_Start_Current_While_Timer(pPort, LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout));
    
    /*(((pPort->AggActorPort).state).lacp_port_state).Expired = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Expired);
    (pPort->AggPortDebug).RxState = Rxm_CURRENT;
}

/*------------------------------------------------------------------------ 
 * ROUTINE NAME - LACP_Get_RxMachine_Current_State
 *------------------------------------------------------------------------ 
 * FUNCTION: This function get Rx Machine transit to Current state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : Rx Machine state.
 * NOTE    : None
 *------------------------------------------------------------------------*/
static RX_MACHINE_STATE_E LACP_Get_RxMachine_Current_State( LACP_PORT_T *pPort)
{
    return (pPort->AggPortDebug).RxState;
}

