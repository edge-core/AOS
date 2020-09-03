/* ------------------------------------------------------------------------
 * FILE NAME - lacp_muxm.c
 * ------------------------------------------------------------------------
 * Abstract: This file contains the multiplexer machine of LACP.
 * ------------------------------------------------------------------------
 *
 * Modification History:
 * Modifier             Date            Description
 * ------------------------------------------------------------------------
 * ckhsu                2001/12/10      Create for Mercury from MC2.
 * amytu                2002/06/24      V2.0  Design change for Single link to be able
 *                                      to form a trunk link
 * Allen Cheng          2002/12/05      Modify for trunk specification changed
 * ------------------------------------------------------------------------
 * Copyright(C)                         ACCTON Technology Corp., 2001, 2002
 * ------------------------------------------------------------------------
 * NOTE :
 *-------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lacp_type.h"
#include "lacp_util.h"
#include "lacp_om_private.h"

#include "stktplg_pom.h"
#include "swctrl.h"
#include "trk_pmgr.h"

#include "sys_time.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARARTIONS */

/* Type definitions  */

/* STATIC VARIABLE DECLARATIONS
 */

#if(SYS_CPNT_DEBUG == TRUE)
static char DEBUG_PageBuffer[1500];
static char DEBUG_LineBuffer[80];
#else
static char DEBUG_PageBuffer[1];
#endif/* #if(SYS_CPNT_DEBUG == TRUE) */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void   LACP_Update_Mux_State( LACP_PORT_T *pPort);
static void   LACP_Selection_Logic( LACP_PORT_T *pPort);
static void   LACP_ReEvaluate_All_Ports( void);
static BOOL_T LACP_Is_Same_Aggregator( LACP_PORT_T *pPort);
/* static void   LACP_Scan_Single_Aggregators( LACP_PORT_T *pPort); */
/*static void   LACP_Scan_True_Aggregators( LACP_PORT_T *pPort);*/
static void   LACP_Try_To_Attach( LACP_PORT_T *pPort);

static SELECTED_VALUE_E LACP_Select_Aggregator(LACP_PORT_T *pPort);

static void LACP_MuxMachine_Detached( LACP_PORT_T *pPort);
static void LACP_MuxMachine_Attached( LACP_PORT_T *pPort);
static void LACP_MuxMachine_Collecting_Distributing( LACP_PORT_T *pPort);

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Mux_Machine
 *------------------------------------------------------------------------
 * FUNCTION: This function is Mux Machine defined in 802.3ad.
 * INPUT   : pPort -- pointer to the port structure.
 *           event -- event to Mux machine.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : The basic idea here is trying to combine MuxMachine and
 *           SelectionLogic in the same place.
 *------------------------------------------------------------------------*/
void LACP_Mux_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event)
{

    switch(event)
    {
    case LACP_SYS_INIT:
    case LACP_RX_PORT_DOWN_EV:
        /* When the MuxMachine is initialized, it should set to the "DETACHED" state. */
        LACP_MuxMachine_Detached( pPort);
        return;
        break;
    case LACP_NEW_INFO_EV:
        break;
    case LACP_MUX_REEVALUATE_EV:
        LACP_ReEvaluate_All_Ports();
        return;
        break;
    case LACP_TIMER_EV:
        if(LACP_Is_Wait_While_Timer_Enabled( pPort))
        {
            /* Just decrease the timer here, because it might have a lot of situations */
            /* that our SELECTED has already changed.                                  */
            LACP_Decrease_Wait_While_Timer( pPort);
        }
        break;
    default:
        break;
    }

    LACP_Update_Mux_State(pPort);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_Mux_State
 *------------------------------------------------------------------------
 * FUNCTION: This function is Mux Machine defined in 802.3ad. When ever
 *           any information is updated, this will be call to check Mux
 *           machine state should transit to any other state.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Update_Mux_State( LACP_PORT_T *pPort)
{
    /* Where AS means Actor Operational State, PS means Partner Operational State. */
    /*LACP_PORT_STATE_U   *pAS = &((pPort->AggActorPort).state),*/
    LACP_PORT_STATE_T   *pAS = &((pPort->AggActorPort).state),
                        *pPS = &((pPort->AggPartnerPort).state);

    BOOL_T  bActorSync,
            bPartnerSync;

    LACP_SYSTEM_T       *lacp_system_ptr;

    /* First, we have to know whether this port is in sync or not and its partner's situation. */
    if( (pPort->Selected == SELECTED) &&
        (pPort->Attach) &&
        (pPort->Attached) )
    {
        bActorSync = TRUE;
    }
    else
    {
        bActorSync = FALSE;
    }

/*Lewis: Matched means (Same Partner or Individual) & One side is Actively */
/* Matched is used with received PDU's Actor's Sync to decide Part.Sync in Mux machine */
    if(   (pPort->Matched)
       && LACP_TYPE_GET_PORT_STATE_BIT(pPS->port_state, LACP_dot3adAggPortActorOperState_Synchronization)
          /*((pPS->lacp_port_state).Synchronization)*/
      )
    {
        bPartnerSync = TRUE;
    }
    else
    {
        bPartnerSync = FALSE;
    }

    /*if((pAS->lacp_port_state).Synchronization != bActorSync)*/
    if (LACP_TYPE_GET_PORT_STATE_BIT(pAS->port_state, LACP_dot3adAggPortActorOperState_Synchronization) != bActorSync)
    {
        /* record change time. */
        lacp_system_ptr = LACP_OM_GetSystemPtr();
        SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));

        /*(pAS->lacp_port_state).Synchronization = bActorSync;*/
        LACP_TYPE_SET_PORT_STATE_BIT(pAS->port_state, bActorSync, LACP_dot3adAggPortActorOperState_Synchronization);
        /* Send a Pdu. */
        pPort->NTT = TRUE;
    }

    /* Partner Sync is Partner Sync.                                                                */
    /* Our problem here are:                                                                        */
    /* 1) We have chip limitation here.                                                             */
    /* 2) How we can integrate VLAN and LACP here. When we create dynamic aggregation, we don't     */
    /*    care about VLAN attributes.                                                               */
    /* Here is the whole state machine of Mux machine, every time when we have Mux Machine event,   */
    /* we will always update the information, hence take care of the finite state transition.       */
    /* First checking should transit to Muxm_COLLECTING_DISTRIBUTING state                          */
    /* If Actor_Sync it means we are in ATTACHED state.                                             */
    if(   ((bPartnerSync) && (bActorSync))
       && (   ( !LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Collecting) )
           || ( !LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Distributing) )
          )
        /*(((((pPort->AggActorPort).state).lacp_port_state).Collecting == FALSE) || ((((pPort->AggActorPort).state).lacp_port_state).Distributing == FALSE)) )*/
      )
    {
        UI32_T trunk_id;
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\n((bPartnerSync) && (bActorSync)) &&");
            BACKDOOR_MGR_Printf("\nAggActorPort (Collecting||Distributing) == FALSE");
        }

        /*macauley: if the port will join the trunk port, wait later enter collection_distribution state before
          this port join the trunk port*/
        if(pPort->pAggregator!=NULL &&(FALSE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunk_id)))
        {
            return;
        }
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
        /* actor can't receive any packet and transfer to LACP_dot3adAggPortActorOperState_Defaulted state,   */
        /* because lacp handshake fail when partner lacp is disable;                                         */
        /* so can't call LACP_MuxMachine_Collecting_Distributing because actor is dormant trunk member         */
        if(pPort->Is_static_lacp == TRUE
        &&  LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state,  LACP_dot3adAggPortActorOperState_Defaulted)== TRUE
          && LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state,  LACP_dot3adAggPortActorOperState_Expired)== FALSE
        )
        {
            return;
        }
#endif

        /* We should transmit to either ATTACH or COLLECTING_DISTRIBUTING state. */
        LACP_MuxMachine_Collecting_Distributing( pPort);
        LACP_Mux_Machine( pPort, LACP_NULL_EVENT);
    }
    else if( ((!bPartnerSync) || (!bActorSync)) &&
           /*  (((((pPort->AggActorPort).state).lacp_port_state).Collecting == TRUE) || ((((pPort->AggActorPort).state).lacp_port_state).Distributing == TRUE)) )*/
             (   (LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Collecting))
              || (LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Distributing))
             )
           )
    {
        /* At this point, it means that we are now in COLLECTING_DISTRIBUTING state */
        /* but for some reason, the Partner is back to DETACHED or WAITING state.   */
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\n((!bPartnerSync) || (!bActorSync)) &&");
            BACKDOOR_MGR_Printf("\nAggActorPort (Collecting||Distributing) == TRUE");
        }

        if(bActorSync)
        {
            if(bLACP_DbgMsg)
            {
                BACKDOOR_MGR_Printf("\nbActorSync == TRUE");
            }

            LACP_MuxMachine_Attached( pPort);
        }
        else
        {
            if(bLACP_DbgMsg)
            {
                BACKDOOR_MGR_Printf("\nbActorSync == FALSE");
            }

            LACP_MuxMachine_Detached( pPort);
        }

        /* ckhsu Add for debug. */
        if(bLACP_DbgMsg)
        {
            if(bPartnerSync)
            {
                BACKDOOR_MGR_Printf("\nbPartnerSync == TRUE");
            }
            else
            {
                BACKDOOR_MGR_Printf("\nbPartnerSync == FALSE");
            }

            if(pPort->Matched)
            {
                BACKDOOR_MGR_Printf("\npPort->Matched == TRUE");
            }
            else
            {
                BACKDOOR_MGR_Printf("\npPort->Matched == FALSE");
            }

            /*if((pPS->lacp_port_state).Synchronization)*/
            if (LACP_TYPE_GET_PORT_STATE_BIT(pPS->port_state, LACP_dot3adAggPortActorOperState_Synchronization))
            {
                BACKDOOR_MGR_Printf("\npPS->Sync == TRUE");
            }
            else
            {
                BACKDOOR_MGR_Printf("\npPS->Sync == FALSE");
            }
        }

        LACP_Mux_Machine( pPort, LACP_NULL_EVENT);
    }
    else if( (pPort->Selected != SELECTED) &&
             ((pPort->AggPortDebug).MuxState != Muxm_DETACHED) )
    {
        /* If we are in any other state, but now it is force to transit */
        /* to DETACH state, then just transit it.                       */
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\n(pPort->Selected != SELECTED) &&");
            BACKDOOR_MGR_Printf("\n((pPort->AggPortDebug).MuxState != Muxm_DETACHED)");
        }

        if(bLACP_DbgTrace)
        {
            if (pPort->Selected == UNSELECTED)
                BACKDOOR_MGR_Printf("\npPort->Selected == UNSELECTED");
            if (pPort->Selected == STANDBY)
                BACKDOOR_MGR_Printf("\npPort->Selected == STANDBY");
            BACKDOOR_MGR_Printf("\nMuxState is %d", (pPort->AggPortDebug).MuxState);
        }

        LACP_MuxMachine_Detached( pPort);
        LACP_Mux_Machine( pPort, LACP_NULL_EVENT);
    }
    else if(pPort->Selected == UNSELECTED)
    {
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\npPort->Selected == UNSELECTED");
        }

        LACP_MuxMachine_Detached( pPort);
        LACP_Selection_Logic( pPort);
    }

    if( (pPort->Selected == SELECTED) &&
        (LACP_Is_Wait_While_Timer_Expired( pPort)) &&
        (!pPort->Attach) &&
        (!pPort->Attached) )
    {
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\n(pPort->Selected == SELECTED)");
            BACKDOOR_MGR_Printf("\nLACP_Is_Wait_While_Timer_Expired");
        }

        /* If this is true, then enter ATTATCH state, else it is already in ATTACH state. */
        LACP_Stop_Wait_While_Timer( pPort);
        LACP_MuxMachine_Attached( pPort);
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Selection_Logic
 *------------------------------------------------------------------------
 * FUNCTION: This function is recommend Selection Logic in 802.3ad. Due to
 *           we might have hardware limitation, every port will select the
 *           first port, and any port which select the same aggregator will
 *           be considered as STANDBY until we reevaluate it.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Selection_Logic( LACP_PORT_T *pPort)
{
    SELECTED_VALUE_E  Select_value = UNSELECTED;
    /* UI32_T            i            = 0; */

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nEnter Selection Logic");
    }

    /* If any of the following parameters of this port is not true, then we do  */
    /* not have to let this port select the AL.                                 */
    if( (pPort->LACP_OperStatus == FALSE) ||
        (pPort->Port_OperStatus == FALSE) )
    {
        return;
    }

#if 0   /* Allenc */
    /* The first step is to find an appropriate port to aggregate in the whole system. */
    pAggPort = LACP_Find_AggregatorPort( pPort);

    if(!pAggPort)
    {
        /* This means in the whole system, we have no appropriate aggregator(no ports in system). */
        printf("\nFile: %s Line: %d has unacceptable behavior...", __FILE__, __LINE__);
        printf("\nCan not find an appropiate aggregator port");
        return;
    }

    bSelect = TRUE;
    Select_value = LACP_Select_Aggregator( pAggPort, pPort);
#endif  /* Allenc */

    Select_value = LACP_Select_Aggregator(pPort);
    if(Select_value == STANDBY)
    {
        pPort->Selected = STANDBY;

        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\npPort->Selected = STANDBY");
        }
        DEBUG_STATE_MACHING("%sSlot:%lu Port:%lu Selected = STANDBY\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port);
    }
    /* 1. Find the Agg.
     * 2. If the partner port is the individule link, it means
     *    the actor port in default state or doesn't enable the lacp, set the port selected whether select the Agg or not.
     */
    else if((Select_value ==SELECTED) ||((pPort->AggPortDebug).RxState == Rxm_DEFAULTED))
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\npPort->Selected = SELECTED");
        }
        DEBUG_STATE_MACHING("%sSlot:%lu Port:%lu Selected = SELECTED\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port);
        pPort->Selected = SELECTED;
        /* Since this port is SELECTED, then we have to start the AGGREGATE_WAIT_TIMER   */
        /* Due to limitation of our firmware, we do not start it, and just always set it */
        /* to expired to let it attach immediately.                                      */
        LACP_Start_Wait_While_Timer( pPort);
    }
    else
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\npPort->Selected = UNSELECTED");
        }
        DEBUG_STATE_MACHING("%sSlot:%lu Port:%lu Selected = UNSELECTED\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port);
        pPort->Selected = UNSELECTED;
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Select_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to add a port to this aggregator.
 *           Except for the first port, we will always consider it as a
 *           STANDBY port to this aggregator.
 * INPUT   : pPort    -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static SELECTED_VALUE_E LACP_Select_Aggregator(LACP_PORT_T *pPort)
{
    LACP_AGGREGATOR_T *pAgg = NULL;
    UI32_T             nIdx = 0;

    if(!pPort)
    {
        printf("\nFile: %s Line: %d passed in NULL ptr...", __FILE__, __LINE__);
        return STANDBY;
    }

#if 0   /* Allenc */
    pAgg = LACP_FIND_AGGREGATOR( pAggPort->slot, pAggPort->port);
#endif  /* Allenc */
    pAgg    = LACP_UTIL_AllocateAggregator(pPort);
    if (pAgg)
    {
        nIdx = LACP_Add_To_Aggregator( pAgg, pPort);

        pAgg->NumOfPortsInAgg++;

        if(nIdx == 1)
        {
            /* Here may have the problem that how we can manage this number?    */
            /* Mark by ckhsu: No longer be used. */
            /* pAgg->NumOfSelectedPorts++; */
            return SELECTED;
        }
    }
    else
    {
        return UNSELECTED;
    }

    return STANDBY;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_ReEvaluate_All_Ports
 *------------------------------------------------------------------------
 * FUNCTION: This function is called every timer tick to reevaluate the
 *           ports to see whether it should attach to another aggregator
 *           or not, besides this, it also re-evaluate whether the port
 *           should aggregate or not.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_ReEvaluate_All_Ports( void)
{
    UI32_T slot, port;
    UI32_T al_id;

    BOOL_T bAggregatable;

    LACP_PORT_T       *pPort;
    LACP_AGGREGATOR_T *pAgg;

    /* 2001/12/04 ckhsu                                                                                */
    /* The current thought is very simple, if a port doesnot have ability to aggregate with any ports  */
    /* then just check whether it has attached to correct aggregator or not.                           */
    /* If it has, then we have to do the followings:                                                   */
    /* 1) First check whether it has more than 1 port in this aggregator. If yes, then we will try     */
    /*    to check whether we have an empty true aggregator to add these ports or this aggregator      */
    /*    has already map to the true aggregator. If it can then just add these ports else just leave  */
    /*    it alone.                                                                                    */
    /* 2) For a aggregator that maps to a true aggregator (a logical interface) then it have to check  */
    /*    the following:                                                                               */
    /*    1. Are ports in this aggregator less than 2 ports, if yes then we have to release the true   */
    /*       aggregator else just left.                                                                */

    for( slot = 1; slot <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; slot++)
    {
        if(STKTPLG_POM_UnitExist( slot) == FALSE)
        {
            continue;
        }

        for( port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            if(STKTPLG_POM_PortExist( slot, port) == FALSE)
            {
                /* We won't have the next port in this slot. */
                /* Allen Cheng, 12/06/2002
                break;
                */
                continue;
            }
            else
            {
                pPort = LACP_FIND_PORT( slot, port);
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT  :configure normal port to join dynamic port-channel
                               and all member will leave this dynamic port-channel ,so the Is_static_lacp should be reset to false*/
                if((FALSE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &al_id))
                    && (pPort->Is_static_lacp == TRUE))
                {
                    pPort->Is_static_lacp = FALSE;
                }
#endif
                if( (pPort->LACP_OperStatus == FALSE) ||
                    (pPort->Port_OperStatus == FALSE) )
                {
                    continue;
                }

                /*if( (((((pPort->AggActorPort).state).lacp_port_state).Aggregation) && ((((pPort->AggPartnerPort).state).lacp_port_state).Aggregation)) )*/
                if (   LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
                    && LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
                   )
                {
                    bAggregatable = TRUE;
                }
                else
                {
                    bAggregatable = FALSE;
                }

                if( (UNSELECTED != pPort->Selected) &&
                    (FALSE == LACP_Is_Same_Aggregator( pPort)) )
                {
                    if( (TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &al_id)) &&
                        (TRUE == TRK_PMGR_IsDynamicTrunkId( al_id)) )
                    {
                        /*EPR:ES3628BT-FLF-ZZ-00856
                         *Probelm: System:modify the lacp admin key cause the lacp cannot create.
                         *Root cause: 1:higher port (25,26) in trunk 1,lower port (6-10)  in trunk 2
                         *            2:change the port (25,26) admin-key to 0,and 25,26 will leave trunk 1
                                      3,port(6-10)will find trunk 1 is better than 2,and leave trunk2,but it will
                                        not notify swctrl to leave trunk2
                                      4 port (25,26) will attacth trunk 2 in lacp,but it will added failed in swctrl
                                        for port(6-10) still in .
                         *Solution: always sync lacp and swctrl to the same!
                         *Modify file: lacp_muxm.c
                         */
#if 0 /* DanXie, Sunday, March 01, 2009 10:25:54 */
                        if(FALSE == LACP_Migrate_Port( pPort))
                        {
	                     if (bLACP_DbgMsg)
                                BACKDOOR_MGR_Printf("\nLACP Migrate FAIL: TRUNK MEMBER of slot:%d port:%d", (int)pPort->slot, (int)pPort->port);
                            /* If migrate fail then we do the same thing. */
                            LACP_MuxMachine_Detached( pPort);
                            pPort->Selected = UNSELECTED;
                            /* We should also update aggregator's pointer list. */
                            LACP_Update_Mux_State( pPort);
                        }
                        else
                        {
                            if (bLACP_DbgMsg)
                                BACKDOOR_MGR_Printf("\nLACP Migrate TRUNK MEMBER of slot:%d port:%d", (int)pPort->slot, (int)pPort->port);
                        }
                        continue;
#else
                        /* Migrate the trunk. */
                        LACP_Migrate_Port( pPort);

                        LACP_MuxMachine_Detached( pPort);
                        pPort->Selected = UNSELECTED;
                        /* We should also update aggregator's pointer list. */
                        LACP_Update_Mux_State( pPort);
                        continue;
#endif /* #if 0 */
                    }


                    /* if(STANDBY == pPort->Selected) */
                    {
                        /* If this is a STANDBY port, then the data structure have to */
                        /* remove from the aggregator else it will cause a problem.   */
                        /* So what I think is simple, just let it re-initialize this  */
                        /* port.                                                      */
                        LACP_MuxMachine_Detached( pPort);
                    }

                    pPort->Selected = UNSELECTED;
                    /* We should also update aggregator's pointer list. */
                    LACP_Update_Mux_State( pPort);
                }
                else if( (UNSELECTED != pPort->Selected) &&
                         (TRUE == bAggregatable) )
                {
                    if(bLACP_DbgTrace)
                    {
                        BACKDOOR_MGR_Printf("\n(Selected != Unselected) && (bAggregatable = TRUE)");
                    }
                    /* Here we have to do:                                                             */
                    /* 1) check whether we should select the same aggregator or not,                   */
                    /* 2) If yes, then do what I said above                                            */
                    /* 3) If no, then just set it as UNSELECTED and it will reselect.                  */
                    /* First we have to check ports in this                                            */
                    /* This port has ability to aggregate with others, then just try to check whether  */
                    /* it should add to the true aggregator or not.                                    */
                    /* Besides this, we also have to check the port select status, if it is UNSELECTED */
                    /* then we don't need to waste time to check it.                                   */
                    pAgg = pPort->pAggregator;
                    if(!pAgg)
                    {
                        /* printf("\nFile: %s Line: %d Port has NULL aggregator...", __FILE__, __LINE__);
                        */
                        continue;
                    }
          /* [amytu: 6-24-2002] Destroy trunk for individual links */
#if 0
                    if((pAgg->NumOfPortsInAgg) == 1)
                    {
                        /* Check whether this port is in true aggregator or not. If yes, then just */
                        /* remove it from true aggregator else ignore it.                          */
                        /* 2001/12/10 ckhsu                                                        */
                        /* In Mercury, we have to use the following to know whether this port is a */
                        /* Trunk port or not.                                                      */
                        /* SWCTRL_UserPortToLogicalPort == SWCTRL_LPORT_TRUNK_PORT_MEMBER          */
                        /* then to get the index.                                                  */
                        /* Add by ckhsu to clarify somthing.   */
                        LACP_Recalculate_CollectingDistributing_Ports( pPort);
                        if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &al_id))
                        {
                            if(TRUE == TRK_PMGR_IsDynamicTrunkId( al_id))
                            {
                                UI32_T  port_ifindex;
                                /* We have to first remove this port from true aggregator. */
                                /* if (TRK_PMGR_DeleteTrunkMember( al_id, pPort->slot, pPort->port)) */
                                if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
                                {
                                    /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                                     * if (TRK_PMGR_DeleteDynamicTrunkMember( al_id, pPort->slot, pPort->port))
                                     */
                                    if (TRK_PMGR_DeleteDynamicTrunkMember( al_id, port_ifindex))
                                    {
                                        if(bLACP_DbgAthAL)
                                        {
                                            BACKDOOR_MGR_Printf("\nLACP_ReEvaluate_All_Ports Delete Trunk id:%d member slot:%d port:%d", (int)al_id, (int)pPort->slot, (int)pPort->port);
                                        }
                                        if (TRK_PMGR_GetTrunkMemberCounts(al_id)==0)
                                        {
                                            /* TRK_PMGR_DestroyTrunk(al_id); */ TRK_PMGR_FreeTrunkIdDestroyDynamic( al_id);
                                            if(bLACP_DbgAthAL)
                                            {
                                                BACKDOOR_MGR_Printf("\nLACP_ReEvaluate_All_Ports Delete Trunk id:%d", (int)al_id);
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if(bLACP_DbgAthAL)
                                    {
                                        BACKDOOR_MGR_Printf("\nLACP_ReEvaluate_All_Ports :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                                    }
                                } /* End of if (SWCTRL_UserPortToIfindex) */

                                if(SELECTED == pPort->Selected)
                                {
                                    /* Here we have to let this port to be selected and interface up. */
                                    /* In Mercury, it is controled by SWCTRL. So we just inform it.   */
                                    /* STA_SetInterface( pPort->slot, pPort->port, TRUE); */
                                    if(LACP_Is_CollectingDistributing( pPort))
                                    {
                                        LACP_SetInterface( pPort->slot, pPort->port, TRUE);
                                    }
                                    else
                                    {
                                        LACP_SetInterface( pPort->slot, pPort->port, FALSE);
                                    }

                                }
                                else
                                {
                                    /* Since this port is STANDBY, so just let it reselect its */
                                    /* aggregator.                                             */
                                    LACP_MuxMachine_Detached( pPort);

                                    pPort->Selected = UNSELECTED;
                                    LACP_Update_Mux_State( pPort);
                                }

                                lacp_system_ptr = LACP_OM_GetSystemPtr();
                                SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                            }
                        }

                    }

                    else if((pAgg->NumOfPortsInAgg) > 1)
                    {
                        /* Try to check whether this port should add to aggregator or remove it */
                        /* from current aggregator.                                             */
                        LACP_Try_To_Attach( pPort);
                    }
#endif
                    /* [amytu 6-20-2002]
                       Reason of Modification:  To support LACP capable of forming LAG with an
                       individual link.
                     */
                    if((pAgg->NumOfPortsInAgg) >= 1)
                    {
                        if(bLACP_DbgTrace)
                        {
                            BACKDOOR_MGR_Printf("\nCurrent Agg ID S%ld:", (long)pAgg->AggID);
                        }
                        /* Try to check whether this port should add to aggregator or remove it */
                        /* from current aggregator.                                             */
                        LACP_Try_To_Attach( pPort);
                    }
                    else
                    {
                        /* ... This case ??? */
                        if( (SELECTED == pPort->Selected) ||
                            (STANDBY == pPort->Selected) )
                        {
                            LACP_MuxMachine_Detached( pPort);

                            pPort->Selected = UNSELECTED;
                            LACP_Update_Mux_State( pPort);
                        }
                        else
                        {
                            printf("\nFile: %s Line: %d Port has non-desireable behavior...", __FILE__, __LINE__);
                        }

                    }
                }

            } /* Port Exists inside system. */

        } /* End of Port iteration. */

    } /* End of Slot iteration. */

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_MuxMachine_Detached
 *------------------------------------------------------------------------
 * FUNCTION: This function is Mux machine detach function.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_MuxMachine_Detached( LACP_PORT_T *pPort)
{
    LACP_SYSTEM_T       *lacp_system_ptr;

    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\n\tSlot:%2d Port:%2d enter DETACHED state", (int)pPort->slot, (int)pPort->port);
    }
    DEBUG_STATE_MACHING("%sSlot:%lu Port:%lu enter DETACHED state\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port);

    LACP_Detach_Mux_From_Aggregator( pPort);
    pPort->Attach   = FALSE;  /* Attach & Attached have the same meaning now */
    pPort->Attached = FALSE;

    /*(((pPort->AggActorPort).state).lacp_port_state).Synchronization = FALSE;*/
    /*(((pPort->AggActorPort).state).lacp_port_state).Collecting      = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Synchronization);
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Collecting);
    LACP_Disable_Collecting_Distributing( pPort);
    /*(((pPort->AggActorPort).state).lacp_port_state).Distributing    = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Distributing);

    /* restore LACP timeout to default */
    {
        UI32_T  port_ifindex;
        if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            LACP_UTIL_SetDot3AggPortActorLacp_Timeout(port_ifindex, LACP_DEFAULT_SYSTEM_TIMEOUT);
        }
    }

    if(pPort->LACP_OperStatus)
    {
        pPort->NTT = TRUE;
    }

    (pPort->AggPortDebug).MuxState = Muxm_DETACHED;

    LACP_Stop_Wait_While_Timer( pPort);

    /* Table changed, record time. */
    lacp_system_ptr = LACP_OM_GetSystemPtr();
    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_MuxMachine_Attached
 *------------------------------------------------------------------------
 * FUNCTION: This function is Mux machine attach function.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_MuxMachine_Attached( LACP_PORT_T *pPort)
{
    LACP_SYSTEM_T       *lacp_system_ptr;

    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\n\tSlot:%2d Port:%2d enter ATTACHED state", (int)pPort->slot, (int)pPort->port);
    }
    DEBUG_STATE_MACHING("%sUnit %lu Port %lu enter ATTACHED state\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port);

    pPort->Attach   = TRUE;
    LACP_Attach_Mux_To_Aggregator( pPort);
    pPort->Attached = TRUE;

    /*(((pPort->AggActorPort).state).lacp_port_state).Synchronization = TRUE;*/
    /*(((pPort->AggActorPort).state).lacp_port_state).Collecting      = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(TRUE)), LACP_dot3adAggPortActorOperState_Synchronization);
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Collecting);
    LACP_Disable_Collecting_Distributing( pPort);
    /*(((pPort->AggActorPort).state).lacp_port_state).Distributing    = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Distributing);

    /* inherit LACP timeout from aggregator to port */
    {
        UI32_T  port_ifindex;

        if (   (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            && (pPort->pAggregator != NULL)
           )
        {
                LACP_UTIL_SetDot3AggPortActorLacp_Timeout(port_ifindex, pPort->pAggregator->AggActorTimeout);
        }
    }

    if(pPort->LACP_OperStatus)
    {
        pPort->NTT = TRUE;
    }

    (pPort->AggPortDebug).MuxState = Muxm_ATTACHED;

    /* Table changed, record time. */
    lacp_system_ptr = LACP_OM_GetSystemPtr();
    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_MuxMachine_Collecting_Distributing
 *------------------------------------------------------------------------
 * FUNCTION: This function is Mux machine enable collecting/distributing
 *           function.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_MuxMachine_Collecting_Distributing( LACP_PORT_T *pPort)
{
    LACP_SYSTEM_T       *lacp_system_ptr;

    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\n\tSlot:%2d Port:%2d enter COLLECTING_DISTRIBUTING state", (int)pPort->slot, (int)pPort->port);
    }
    DEBUG_STATE_MACHING("%sEnable Collecting_Distributing on unit %lu port %lu\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port);

    /*(((pPort->AggActorPort).state).lacp_port_state).Distributing = TRUE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(TRUE)), LACP_dot3adAggPortActorOperState_Distributing);
    LACP_Enable_Collecting_Distributing( pPort);
    /*(((pPort->AggActorPort).state).lacp_port_state).Collecting   = TRUE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(TRUE)), LACP_dot3adAggPortActorOperState_Collecting);

    if(pPort->LACP_OperStatus)
    {
        pPort->NTT = TRUE;
    }

    (pPort->AggPortDebug).MuxState = Muxm_COLLECTING_DISTRIBUTING;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Same_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is to check whether the attached aggregator is
 *           the one we should attach to or not.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : TRUE  -- The select aggregator is same as current select one
 *           FALSE -- should select another aggregator.
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T LACP_Is_Same_Aggregator( LACP_PORT_T *pPort)
{
    /* Here we have to check whether we should still attach to same aggregator or not. */
    LACP_AGGREGATOR_T *pAgg;

#if 0   /* Allenc */
    pAggPort = LACP_Find_AggregatorPort( pPort);

    pAgg = LACP_FIND_AGGREGATOR( pAggPort->slot, pAggPort->port);
#endif  /* Allenc */
    pAgg    = LACP_UTIL_AllocateAggregator(pPort);


#if 0   /* Allenc */
    if(!pAgg)
    {
        /* Since this port is SELECTED but why it does not point to an aggregator? */
        printf("\nFile: %s Line: %d has unacceptable behavior...", __FILE__, __LINE__);
        printf("\nSlot:%2d Port:%2d is SELECTED but with no aggregator...", (int)pPort->slot, (int)pPort->port);
        return FALSE;
    }
#endif  /* Allenc */

    if(pPort->pAggregator != pAgg)
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Try_To_Attach
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to check whether the port can add to
 *           true aggregator in the trunk manager or not.
 * INPUT   : pPort -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Try_To_Attach( LACP_PORT_T *pPort)
{
    /* Try to check whether this port should add to aggregator or remove it */
    /* from current aggregator.                                             */
    UI32_T  port_index;
    UI32_T  Selected_AL_id;
    UI32_T  dbg_agg_id;
    UI32_T  dbg_trk_mgr_err_code;
    /*        current_check_AL_id; */
    /* UI32_T  slot, port; */
    /* UI32_T  port_status; */

    BOOL_T  bCollecting_Distributing_State = FALSE;

    LACP_AGGREGATOR_T *pAgg;
    LACP_SYSTEM_T     *lacp_system_ptr;

    /* We should judge this from current index or this port inside the aggregator.    */
    /* If this index is larger than SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK then it should */
    /* change to STANDBY state.                                                       */
    /* else it should switch to SELECTED state.                                       */
    /* But due to maintain the code integrity, I suggest we do this by following the  */
    /* structure of the standard not to add any special codes.                        */
    /*                                                                                */
    /* Algorithm change again, due to some cases, I'll change the algorithm to check  */
    /* Partner.Sync to decide whether to add to trunk or not. If we still have empty  */
    /* room for port, we will set Actor.Sync = TRUE, when the whole LAG has more than */
    /* two Partner.Sync = TRUE, we will start to add ports, once we found the trunk   */
    /* is occupied by others, and that is another LAG, then we will set SELECTED to   */
    /* STANDBY again to let Partner know that now we have no ability to aggregate     */
    /* with remote device.                                                            */
    if(!pPort)
    {
        return;
    }

    pAgg = pPort->pAggregator;

    if(!pAgg)
    {
        return;
    }

    /* Now recalcute the ports in Collecting_Distributing state. */
    LACP_Recalculate_CollectingDistributing_Ports( pPort);
    bCollecting_Distributing_State = LACP_Is_CollectingDistributing( pPort);

    /* [amytu 6-20-2002] to satisfy the ability to form LAG for
       individual link
     */
#if 0
    if(pAgg->NumOfPortsInAgg <= 1)
    {
        /* Basically this should not happen, but to prevent any accident, */
        /* just check it.                                                 */
        printf("\nFile: %s Line: %d is unpredictable...", __FILE__, __LINE__);
        return;
    }
#endif
    port_index = LACP_Get_Port_Index_In_Aggregator( pPort);

   if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nInside try_to_Attach");
        BACKDOOR_MGR_Printf("\nUnit %ld Port %ld", (long)pPort->slot, (long)pPort->port);
        BACKDOOR_MGR_Printf("\nIndex %ld in Agg ID %ld", (long)port_index, (long)pAgg->AggID);
    }

    if(!port_index)
    {
        printf("\nFile: %s Line: %d Slot: %d Port: %d Agg: %d  index: %d is unpredictable...", __FILE__, __LINE__, (int)pPort->slot, (int)pPort->port, (int)(pPort->pAggregator)->AggID, (int)port_index);
        return;
    }

    pPort->port_index = port_index;

    if(bLACP_DbgAttach)
    {
        BACKDOOR_MGR_Printf("\nSlot: %d Port: %d Agg:%d index:%d", (int)pPort->slot, (int)pPort->port, (int)pAgg->AggID, (int)port_index);
        BACKDOOR_MGR_Printf("\n# of ports in Agg:%d", (int)pAgg->NumOfPortsInAgg);
        BACKDOOR_MGR_Printf("\nNumOfCollectingDistributingPorts: %d", (int)pAgg->NumOfCollectingDistributingPorts);
        BACKDOOR_MGR_Printf("\nPort %s Collecting_Distributing", ((bCollecting_Distributing_State)?("is"):("isn't")));

        if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &dbg_agg_id))
        {
            BACKDOOR_MGR_Printf("\nPort is in trunk id: %d", (int)dbg_agg_id);
            BACKDOOR_MGR_Printf("\n");
        }
        else
        {
            BACKDOOR_MGR_Printf("\nPort is not a trunk member...");
            BACKDOOR_MGR_Printf("\n");
        }
    }

    {
        DEBUG_PageBuffer[0]=0;
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tSlot:%lu Port:%lu Agg:%lu index:%lu", (unsigned long)pPort->slot, (unsigned long)pPort->port, (long)pAgg->AggID, (long)port_index);
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\t# of ports in Agg:%lu", (unsigned long)pAgg->NumOfPortsInAgg);
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tNumOfCollectingDistributingPorts:%lu", (unsigned long)pAgg->NumOfCollectingDistributingPorts);
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPort %s Collecting_Distributing", ((bCollecting_Distributing_State)?("is"):("isn't")));

        if(TRUE == LACP_Is_Trunk_Member_Set(pPort->slot, pPort->port, &dbg_agg_id))
        {
            DEBUG_STRCAT(DEBUG_PageBuffer, "\r\n\tPort is in trunk id: %lu", (unsigned long)dbg_agg_id);
        }
        else
        {
            DEBUG_STRCAT(DEBUG_PageBuffer, "\r\n\tPort is not a trunk member...");
        }
        DEBUG_STRCAT(DEBUG_PageBuffer, "\r\n");
        DEBUG_STATE_MACHING("%stry to attach:%s", DEBUG_HEADER(), DEBUG_PageBuffer);
    }

    /* The first one is the case whether this port has more ports than aggregator can handle */
    /* so we have to remove one to let higher ports to be added into the true aggregator.    */
    if( (port_index > SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK) &&
        (SELECTED == pPort->Selected) )
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\n(port_index > %ld) &&", (long) SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK);
            BACKDOOR_MGR_Printf("\n(SELECTED == pPort->Selected");
        }
        DEBUG_STATE_MACHING("%s(port_index > %ld) && (SELECTED == pPort->Selected\r\n", DEBUG_HEADER(), (long)SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK);

        /* This port should set to STANDBY due to priority and id of this group. */
        LACP_MuxMachine_Detached( pPort);

        /* Originally, it should set to STANDBY, but since it is same as UNSELECTED. */
        /* To prevent any accident, I would rather it to RESELECT again.             */
        /* pPort->Selected = STANDBY; */
        pPort->Selected = UNSELECTED;

        LACP_Update_Mux_State( pPort);

        return;
    }

    /* The following are the cases we have to handle of the */
    else if(port_index <= SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK)
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\n(port_index <= %ld)", (long)SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK);
        }
        DEBUG_STATE_MACHING("%s(port_index <= %ld)\r\n", DEBUG_HEADER(), (long)SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK);

        /* Check above information to see how to port to Mercury. */
        if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &Selected_AL_id))
        {
            if(bLACP_DbgTrace)
            {
                BACKDOOR_MGR_Printf("\nUnit %ld Port %ld is in trunk id %ld", (long)pPort->slot, (long)pPort->port, (long)Selected_AL_id);
            }
            DEBUG_DATABASE("%sUnit %lu Port %lu is in trunk id %lu\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port, (long)Selected_AL_id);

            /* Don't care of this case because it has already became AL member. */
            /* Now we have to care about this since we have the mission to know */
            /* what is the exact situation of remote partner. If both of the    */
            /* machines between the link has the ability, then we will add it,  */
            /* else we will just ignore it. Remember to check whether we have   */
            /* more than two links in the LAG is in Partner.Sync = TRUE.        */

            /* First we check for the*/

            /* [amytu: 6-20-2002] */
#if 0
            if(pAgg->NumOfCollectingDistributingPorts <= 1)
#endif
            if(pAgg->NumOfCollectingDistributingPorts < 1)
            {
                /* We have to remove the trunk. */
                if(port_index == 1)
                {
                    if(bLACP_DbgTrace)
                    {
                        BACKDOOR_MGR_Printf("\nInside (port index == 1)");
                    }

                    /* This judgement should be changed in Mercury. */
                    if(TRUE == TRK_PMGR_IsDynamicTrunkId( Selected_AL_id))
                    {
                        UI32_T  port_ifindex;
                        /*if (TRK_PMGR_DeleteTrunkMember( Selected_AL_id, pPort->slot, pPort->port))*/
                        if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
                        {
                            /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                             * if (TRK_PMGR_DeleteDynamicTrunkMember( Selected_AL_id, pPort->slot, pPort->port))
                             */
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                            if(pPort->Is_static_lacp == FALSE)
                            {
                                if (TRK_PMGR_DeleteDynamicTrunkMember( Selected_AL_id, port_ifindex))
                                {
                                    if(bLACP_DbgAthAL)
                                    {
                                        BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d member slot:%d port:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port);
                                    }
                                    DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu member slot:%lu port:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port);

                                    if (TRK_PMGR_GetTrunkMemberCounts(Selected_AL_id)==0)
                                    {
                                        /* TRK_PMGR_DestroyTrunk(Selected_AL_id); */
                                        TRK_PMGR_FreeTrunkIdDestroyDynamic(Selected_AL_id);
                                        if(bLACP_DbgAthAL)
                                        {
                                            BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d", (int)Selected_AL_id);
                                        }
                                        DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id);
                                    }
                                }
                                else
                                {
                                    if(bLACP_DbgMsg)
                                    {
                                        BACKDOOR_MGR_Printf("\nFile: %s Line: %d fail to delete trunk member...", __FILE__, __LINE__);
                                    }

                                    return;
                                }
                            }
#else
                            if (TRK_PMGR_DeleteDynamicTrunkMember( Selected_AL_id, port_ifindex))
                            {
                                if(bLACP_DbgAthAL)
                                {
                                    BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d member slot:%d port:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port);
                                }
                                DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu member slot:%lu port:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port);

                                if (TRK_PMGR_GetTrunkMemberCounts(Selected_AL_id)==0)
                                {
                                    /* TRK_PMGR_DestroyTrunk(Selected_AL_id); */
                                    TRK_PMGR_FreeTrunkIdDestroyDynamic(Selected_AL_id);
                                    if(bLACP_DbgAthAL)
                                    {
                                        BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d", (int)Selected_AL_id);
                                    }
                                    DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id);
                                }
                            }
                            else
                            {
                                if(bLACP_DbgMsg)
                                {
                                    BACKDOOR_MGR_Printf("\nFile: %s Line: %d fail to delete trunk member...", __FILE__, __LINE__);
                                }

                                return;
                            }
#endif
                        }
                        else
                        {
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                            }
                            DEBUG_STATE_MACHING("%sLACP_Try_To_Attach :: Fail to Delete unknown Trunk member slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (long)pPort->port, (long)port_ifindex);
                        } /* End of if (SWCTRL_UserPortToIfindex) */

                        if(SELECTED == pPort->Selected)
                        {
                            if(bCollecting_Distributing_State)
                            {
                                LACP_SetInterface( pPort->slot, pPort->port, TRUE);
                            }
                            else
                            {
                                LACP_SetInterface( pPort->slot, pPort->port, FALSE);
                            }
                        }
                        else
                        {
                            /* Since this port is STANDBY, so just let it reselect its */
                            /* aggregator.                                             */
                            LACP_SetInterface( pPort->slot, pPort->port, FALSE);

                            LACP_MuxMachine_Detached( pPort);

                            /* Originally it should be STANDBY, but I would rather it to RESELECT */
                            /* again.                                                             */
                            pPort->Selected = UNSELECTED;

                            LACP_Update_Mux_State( pPort);
                        }

                        lacp_system_ptr = LACP_OM_GetSystemPtr();
                        SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                    }
                }
                else
                {
                    if(TRUE == TRK_PMGR_IsDynamicTrunkId( Selected_AL_id))
                    {
                        UI32_T  port_ifindex;
                        /* if (TRK_PMGR_DeleteTrunkMember( Selected_AL_id, pPort->slot, pPort->port)) */
                        if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
                        {
                            /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                             * if (TRK_PMGR_DeleteDynamicTrunkMember( Selected_AL_id, pPort->slot, pPort->port))
                             */
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                            if(pPort->Is_static_lacp == FALSE)
                            {
                                if (TRK_PMGR_DeleteDynamicTrunkMember( Selected_AL_id, port_ifindex))
                                {
                                    if(bLACP_DbgAthAL)
                                    {
                                        BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d member slot:%d port:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port);
                                    }
                                    DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu member slot:%lu port:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port);

                                    if (TRK_PMGR_GetTrunkMemberCounts(Selected_AL_id)==0)
                                    {
                                        /* TRK_PMGR_DestroyTrunk(Selected_AL_id); */
                                        TRK_PMGR_FreeTrunkIdDestroyDynamic( Selected_AL_id);
                                        if(bLACP_DbgAthAL)
                                        {
                                            BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d", (int)Selected_AL_id);
                                        }
                                        DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id);
                                    }
                                }
                            }
#else
                            if (TRK_PMGR_DeleteDynamicTrunkMember( Selected_AL_id, port_ifindex))
                            {
                                if(bLACP_DbgAthAL)
                                {
                                    BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d member slot:%d port:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port);
                                }
                                DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu member slot:%lu port:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port);

                                if (TRK_PMGR_GetTrunkMemberCounts(Selected_AL_id)==0)
                                {
                                    /* TRK_PMGR_DestroyTrunk(Selected_AL_id); */
                                    TRK_PMGR_FreeTrunkIdDestroyDynamic( Selected_AL_id);
                                    if(bLACP_DbgAthAL)
                                    {
                                        BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Delete Trunk id:%d", (int)Selected_AL_id);
                                    }
                                    DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Delete Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id);
                                }
                            }
#endif
                        }
                        else
                        {
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                            }
                            DEBUG_STATE_MACHING("%sLACP_Try_To_Attach :: Fail to Delete unknown Trunk member slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port, (unsigned long)port_ifindex);
                        } /* End of if (SWCTRL_UserPortToIfindex) */
                    }
                    /* Since this port is STANDBY, so just let it reselect its */
                    /* aggregator.                                             */
                    LACP_MuxMachine_Detached( pPort);

                    pPort->Selected = UNSELECTED;

                    LACP_Update_Mux_State( pPort);

                    lacp_system_ptr = LACP_OM_GetSystemPtr();
                    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                }
            }

#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
           /*when port is static lacp trunk member, it does not need add port to trunk but need revaule  the pPort->Selected after linkup*/
           if(pPort->Is_static_lacp == TRUE)
           {
               if (    (FALSE == TRK_PMGR_IsDynamicTrunkId( Selected_AL_id))
                    &&  (TRK_PMGR_GetTrunkMemberCounts(Selected_AL_id) != 0)
                   )
                {
                    /* This is a static trunk, just ignore it. */
                    /* Actually, this should not happen here.  */
                    if(bLACP_DbgMsg)
                    {
                        BACKDOOR_MGR_Printf("\n***** Should not come here ! Getting a static trunk\n");
                        BACKDOOR_MGR_Printf("\nFile: %s Line: %d is not dynamic trunk...", __FILE__, __LINE__);
                    }
                    return;
                }
                else
                {
                    /* Originally we have to check whether the port can be aggregated   */
                    /* together or not, but since this must be a dynamic trunk and one  */
                    /* of the port in aggregator is in this port, so we always should   */
                    /* aggregate to this trunk.                                         */

                    if(STANDBY == pPort->Selected)
                    {
                        pPort->Selected = SELECTED;
                        /* Now we have to start the timer. */
                        LACP_Start_Wait_While_Timer( pPort);

                        LACP_Update_Mux_State( pPort);

                        return;
                    }
                    else if(UNSELECTED == pPort->Selected)
                    {
                        /* Should not happen. */
                        if (bLACP_DbgMsg)
                        {
                            BACKDOOR_MGR_Printf("\nFile: %s Line: %d is strange...", __FILE__, __LINE__);
                        }

                        return;
                    }
                }

           }
#endif
            return;
        }
        else
        {
            if(bLACP_DbgTrace)
            {
                BACKDOOR_MGR_Printf("\nNot Trunk Member...Get a new available trunk id");
            }

            /* Now we have to check where to add this port. */
            /* Selected_AL_id = P_TRUNK_Get_Available_TrunkId( pPort->slot, pPort->port); */
            Selected_AL_id = LACP_Get_Available_TrunkId( pPort->slot, pPort->port);

            if( (Selected_AL_id == 0) ||
                (Selected_AL_id > MAX_LPORTS) )
            {
                return;
            }
            if(bLACP_DbgTrace)
            {
                BACKDOOR_MGR_Printf("\nAvailable Trunk_id got is %ld", (long)Selected_AL_id);
            }

            /* Now we try to get the first port in this trunk id.   */
            /* Originally in MC2/Foxfire, I just want to know whether this id is */
            /* a empty id or used by other ports then decide to compare it with  */
            /* the current ports. But now in Mercury, it is different and do not */
            /* has the same feature so I have to change the code here to achieve */
            /* the same thing as before.                                         */
            if(FALSE == LACP_Is_Empty_Trunk( Selected_AL_id))
            {
                /* This means this aggregator must belong to this trunk */
                /* First we have to check this trunk is dynamic trunk   */
                /* or any other kind of trunk. Since SWCTRL now is not  */
                /* Finish yet, so I just always consider it is a static */
                /* trunk.                                               */
                if (    (FALSE == TRK_PMGR_IsDynamicTrunkId( Selected_AL_id))
                    &&  (TRK_PMGR_GetTrunkMemberCounts(Selected_AL_id) != 0)
                   )
                {
                    /* This is a static trunk, just ignore it. */
                    /* Actually, this should not happen here.  */
                    if(bLACP_DbgMsg)
                    {
                        BACKDOOR_MGR_Printf("\n***** Should not come here ! Getting a static trunk\n");
                        BACKDOOR_MGR_Printf("\nFile: %s Line: %d is not dynamic trunk...", __FILE__, __LINE__);
                    }
                    return;
                }
                else
                {
                    /* Originally we have to check whether the port can be aggregated   */
                    /* together or not, but since this must be a dynamic trunk and one  */
                    /* of the port in aggregator is in this port, so we always should   */
                    /* aggregate to this trunk.                                         */

                    if(STANDBY == pPort->Selected)
                    {
                        pPort->Selected = SELECTED;
                        /* Now we have to start the timer. */
                        LACP_Start_Wait_While_Timer( pPort);

                        LACP_Update_Mux_State( pPort);

                        return;
                    }
                    else if(UNSELECTED == pPort->Selected)
                    {
                        /* Should not happen. */
                        if (bLACP_DbgMsg)
                        {
                            BACKDOOR_MGR_Printf("\nFile: %s Line: %d is strange...", __FILE__, __LINE__);
                        }

                        return;
                    }

                    if(bCollecting_Distributing_State)
                    {
                        UI32_T  port_ifindex;
                        /* Add to trunk when ever it transit to Collecting_Distributing state */
                        /* STA_SetDynTrunkPort( Selected_AL_id, pPort->slot, pPort->port, ROW_STATUS_CREATE_AND_GO); */
                        /* Since there has already at least one member in the trunk */
                        /* So we do not need to allocate or create trunk for it. */
                        /* TRK_PMGR_AddTrunkMember( Selected_AL_id, pPort->slot, pPort->port); */
                        /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                         * dbg_trk_mgr_err_code = TRK_PMGR_AddDynamicTrunkMember( Selected_AL_id, pPort->slot, pPort->port);
                         */
                        if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
                        {
                            dbg_trk_mgr_err_code    = TRK_MGR_ERROR_TRUNK;
                        }
                        else
                        {
                            dbg_trk_mgr_err_code    = TRK_PMGR_AddDynamicTrunkMember( Selected_AL_id, port_ifindex, TRUE);
                        }
                        if(dbg_trk_mgr_err_code != TRK_MGR_SUCCESS)
                        {
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nFail to TRK_PMGR_AddDynamicTrunkMember id=%d slot:%d port:%d ifindex:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
  				                LACP_Print_Fail_Message( dbg_trk_mgr_err_code);
                            }
                            DEBUG_STATE_MACHING("%sFail to TRK_MGR_AddDynamicTrunkMember id=%lu slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port, (unsigned long)port_ifindex);
                        }
                        else
                        {
                            lacp_system_ptr = LACP_OM_GetSystemPtr();
                            SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                            if(lacp_system_ptr->aggregator[Selected_AL_id - 1].AggActorAdminKey == pPort->AggActorAdminPort.key
                                && pPort->AggActorAdminPort.key !=0)
                            {
                                pPort->Is_static_lacp = TRUE;
                            }
#endif
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Add Trunk id:%d member slot:%d port:%d ifindex:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                            }
                            DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Add Trunk id:%lu member slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port, (unsigned long)port_ifindex);
                        }
                    }
                }
            }
            else
            {   /* This is an empty trunk id. */
                if(bLACP_DbgTrace)
                {
                    BACKDOOR_MGR_Printf("\nport index is %ld", (long)port_index);
                }

                if(port_index == 1)
                {
                    /* Only let the trunk is occupied when we have two or more ports is in */
                    /* Collecting_Distributing state.                                      */
#if 0
                    if(pAgg->NumOfCollectingDistributingPorts > 1)
#endif
                    if(pAgg->NumOfCollectingDistributingPorts >= 1)
                    {
                        /* STA_SetDynTrunkPort( Selected_AL_id, pPort->slot, pPort->port, ROW_STATUS_CREATE_AND_GO); */
                        /* TRK_PMGR_CreateTrunk( Selected_AL_id); */
                        /* if (TRK_PMGR_AllocateTrunkIdCreateDynamic( &Selected_AL_id)) */
                        /* if(TRK_PMGR_AllocateTrunkIdCreateDynamic( &Selected_AL_id))*/
                        BOOL_T  is_static;
                        Selected_AL_id  = (pPort->pAggregator)->AggID;
                        if (    TRK_PMGR_IsTrunkExist(Selected_AL_id, &is_static)
                            ||  TRK_PMGR_CreateDynamicTrunk(Selected_AL_id)
                           )
                        {
                            UI32_T  port_ifindex;
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Allocate Trunk id:%d", (int)Selected_AL_id);
                            }
                            DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Allocate Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id);

                            /* if (TRK_PMGR_AddTrunkMember( Selected_AL_id, pPort->slot, pPort->port) != TRK_MGR_SUCCESS) */
                            /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                             * dbg_trk_mgr_err_code = TRK_PMGR_AddDynamicTrunkMember( Selected_AL_id, pPort->slot, pPort->port);
                             */
                            if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
                            {
                                dbg_trk_mgr_err_code    = TRK_MGR_ERROR_TRUNK;
                            }
                            else
                            {
                                dbg_trk_mgr_err_code    = TRK_PMGR_AddDynamicTrunkMember(Selected_AL_id, port_ifindex, TRUE);
                            }
                            if(dbg_trk_mgr_err_code != TRK_MGR_SUCCESS)
                            {
                                if(bLACP_DbgAthAL)
                                {
                                    BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Fail to add trunk member Trunk id:%d slot:%d port:%d ifindex:%d", (int)Selected_AL_id, (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                                    LACP_Print_Fail_Message( dbg_trk_mgr_err_code);
                                }
                                DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Fail to add trunk member Trunk id:%lu slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (long)Selected_AL_id, (unsigned long)pPort->slot, (unsigned long)pPort->port, (unsigned long)port_ifindex);

                                /* TRK_PMGR_DestroyTrunk(Selected_AL_id); */
                                TRK_PMGR_FreeTrunkIdDestroyDynamic( Selected_AL_id);
                                if(bLACP_DbgAthAL)
                                {
                                    BACKDOOR_MGR_Printf("\nLACP_Try_To_Attach Free Trunk id:%d", (int)Selected_AL_id);
                                }
                                DEBUG_STATE_MACHING("%sLACP_Try_To_Attach Free Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)Selected_AL_id);
                            }
                            else
                            {
                                lacp_system_ptr = LACP_OM_GetSystemPtr();
                                SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                                if(lacp_system_ptr->aggregator[Selected_AL_id - 1].AggActorAdminKey == pPort->AggActorAdminPort.key
                                    && pPort->AggActorAdminPort.key !=0)
                                {
                                    pPort->Is_static_lacp = TRUE;
                                }
#endif
                            }

                        }
                        else
                        {
                            return;
                        }
                    }
                }

                /* Since we have an empty trunk index, before it is occupied, this port is */
                /* always aggregatable.                                                    */
                if(STANDBY == pPort->Selected)
                {
                    pPort->Selected = SELECTED;
                    /* Now we have to start the timer. */
                    LACP_Start_Wait_While_Timer( pPort);

                    LACP_Update_Mux_State( pPort);
                }
            }
        }
    }
}
