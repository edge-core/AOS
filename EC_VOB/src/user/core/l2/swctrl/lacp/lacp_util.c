/* ------------------------------------------------------------------------
 * FILE NAME - lacp_util.c
 * ------------------------------------------------------------------------
 * Abstract: This file contains the utilities and functions defined in
 *           802.3ad.
 * ------------------------------------------------------------------------
 *
 * Modification History:
 * Modifier             Date            Description
 * ------------------------------------------------------------------------
 *  ckhsu               2001/12/10      Create for Mercury from MC2.
 *  amytu               2002/06/24      V1.0  Design change for Single link
 *                                      to be able to form a trunk link
 * Allen Cheng          2002/12/05      Modify for trunk specification changed
 * ------------------------------------------------------------------------
 * Copyright(C)                         ACCTON Technology Corp., 2001, 2002
 * ------------------------------------------------------------------------
 * NOTE :
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_module.h"
#include "sys_dflt.h"
#include "lacp_type.h"
#include "lacp_util.h"
#include "lacp_om.h"
#include "lacp_om_private.h"
#include "lacp_mgr.h"
#include "sysfun.h"

#include "stktplg_pom.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "trk_pmgr.h"
#include "lan.h"
#include "vlan_om.h"
#include "vlan_pmgr.h"

#include "sys_time.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

#define LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME()  SYS_TIME_GetSystemUpTimeByTick(&lacp_last_change_time)

/* STATIC VARIABLE DECLARATIONS
 */

BOOL_T bLACP_DbgMsg = FALSE;
BOOL_T bLACP_DbgPdu = FALSE;
BOOL_T bLACP_DbgAthAL = FALSE;
BOOL_T bLACP_DbgAttach= FALSE;
BOOL_T bLACP_DbgTrace= FALSE;
static UI32_T lacp_last_change_time;

#if(SYS_CPNT_DEBUG == TRUE)
static char DEBUG_PageBuffer[1500];
static char DEBUG_LineBuffer[80];
#else
static char DEBUG_PageBuffer[1];
#endif/* #if(SYS_CPNT_DEBUG == TRUE) */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void   copy_info(LAC_INFO_T *from, LAC_INFO_T *to);
static BOOL_T same_Partner( LAC_INFO_T *a, LAC_INFO_T *b);

static void   LACP_StopTimer( LACP_TIMER_T *pTimer);
static void   LACP_StartTimer( LACP_TIMER_T *pTimer, UI32_T tick);
static BOOL_T LACP_TimerIsEnabled( LACP_TIMER_T *pTimer);
static void   LACP_TimerDecrease( LACP_TIMER_T *pTimer);
static BOOL_T LACP_TimerExpired( LACP_TIMER_T *pTimer);

static void   LACP_Decode_PortStateInfo( LACP_PORT_STATE_T *p);
static void   LACP_Show_Statistics( LACP_PORT_STATISTICS_T *p);
static void   LACP_Show_Debug_State( LACP_PORT_DEBUG_INFO_T *p);
static void   LACP_Show_PortTimer( LACP_PORT_TIMER_T *p);
static void   LACP_ShowTimerInfo( LACP_TIMER_T *p);
static void   LACP_Show_Aggregator( LACP_AGGREGATOR *p);

#if 0   /* Not used currently */
static BOOL_T LACP_Check_Looped( LACP_PORT_T *pA, LACP_PORT_T *pPort);
#endif  /* Not used currently */

static LACP_PORT_T * LACP_Get_Appropriate_Physical_Port_From_ifIndex( UI32_T agg_index);
#if 0   /* Redesigned */
static LACP_PORT_T * LACP_Get_Appropriate_Port_From_ifIndex( UI32_T agg_index);
static BOOL_T        LACP_Get_PortListPorts( LACP_PORT_T *pPort, UI8_T *pList);
#endif

static  BOOL_T  LACP_UTIL_PortIsAggregatable(LACP_PORT_T *port_ptr);
static  BOOL_T  LACP_UTIL_IsTheSameAttribute(LACP_AGGREGATOR_T *agg_ptr, LACP_PORT_T *port_ptr);
static  void    LACP_UTIL_CopyTheAttributeFromPort(LACP_AGGREGATOR_T *agg_ptr, LACP_PORT_T *port_ptr);
static  BOOL_T  LACP_UTIL_GetAggMemberList( LACP_AGGREGATOR_T *agg_ptr, UI8_T *port_list);
#if 0   /* Not used currently */
static  BOOL_T  LACP_UTIL_PortsAreTheSameAttribute(LACP_PORT_T *dst_port, LACP_PORT_T *src_port);
#endif  /* Not used currently */


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_recordActorDefault
 *------------------------------------------------------------------------
 * FUNCTION: This function will record default actor information to
 *           related port.
 * INPUT   : port -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_recordActorDefault( LACP_PORT_T *port)
{
    copy_info(&(port->AggActorAdminPort), &(port->AggActorPort));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_recordDefault
 *------------------------------------------------------------------------
 * FUNCTION: This function will record default partner information to
 *           related port.
 * INPUT   : port -- pointer to the port structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_recordDefault( LACP_PORT_T *port)
{
    LACP_SYSTEM_T       *lacp_system_ptr;

    copy_info(&(port->AggPartnerAdminPort), &(port->AggPartnerPort));
    /*port->AggActorPort.state.lacp_port_state.Defaulted = TRUE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(port->AggActorPort.state.port_state, ((BOOL_T)(TRUE)), LACP_dot3adAggPortActorOperState_Defaulted);
    /* Update the system time of changing time of table */
    lacp_system_ptr = LACP_OM_GetSystemPtr();
    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - copy_info
 *------------------------------------------------------------------------
 * FUNCTION: This function will copy the information from info to info
 * INPUT   : from -- pointer to the LAC_INFO structure which will copy
 *                   from.
 *           to   -- pointer to the LAC_INFO structure which will copy
 *                   to.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void copy_info(LAC_INFO_T *from, LAC_INFO_T *to)
{
    memcpy( to, from, sizeof(LAC_INFO_T));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - same_Partner
 *------------------------------------------------------------------------
 * FUNCTION: This function will compare pass in a and b to check whether
 *           they has the same information or not to know this is the same
 *           Partner in LACP or not.
 * INPUT   : a -- pointer to the LAC_INFO structure which will be compared
 *           b -- pointer to the LAC_INFO structure which will be compared
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T same_Partner( LAC_INFO_T *a, LAC_INFO_T *b)
{
    return  ( (a->port_no == b->port_no) &&
              (a->port_priority == b->port_priority) &&
              (0 == memcmp( a->system_id, b->system_id, 0x6)) &&
              (a->system_priority == b->system_priority) &&
              (a->key == b->key) &&
              /*(((a->state).lacp_port_state).Aggregation == ((b->state).lacp_port_state).Aggregation) );*/
              (LACP_TYPE_GET_PORT_STATE_BIT((a->state).port_state, LACP_dot3adAggPortActorOperState_Aggregation) == LACP_TYPE_GET_PORT_STATE_BIT((b->state).port_state, LACP_dot3adAggPortActorOperState_Aggregation))
            );
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_StopTimer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the timer.
 * INPUT   : pTimer -- pointer to the LACP_TIMER_T structure which will we
 *                     want to stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_StopTimer( LACP_TIMER_T *pTimer)
{
    pTimer->enabled = FALSE;
    pTimer->tick    = 0;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_StartTimer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the timer with the tick value.
 * INPUT   : pTimer -- pointer to the LACP_TIMER_T structure which will we
 *                     want to start.
 *           tick   -- Expired tick (unit:second)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_StartTimer( LACP_TIMER_T *pTimer, UI32_T tick)
{
    pTimer->enabled = TRUE;
    pTimer->tick    = tick;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TimerIsEnabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the timer to see whether it is
 *           "Enabled" or "Disabled"
 * INPUT   : pTimer -- pointer to the LACP_TIMER_T structure which will we
 *                     want to check.
 * OUTPUT  : None
 * RETURN  : TRUE   -- Enabled
 *           FALSE  -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T LACP_TimerIsEnabled( LACP_TIMER_T *pTimer)
{
    return pTimer->enabled;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TimerDecrease
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the timer tick
 * INPUT   : pTimer -- pointer to the LACP_TIMER_T structure which will we
 *                     want to decrease.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only decrease when tick is not 0 and timer is "Enabled".
 *------------------------------------------------------------------------*/
static void LACP_TimerDecrease( LACP_TIMER_T *pTimer)
{
    /* ckhsu 2001/11/22                                                      */
    /* To prevent that if a timer is expired but we just forget to check it. */
    if( (pTimer->enabled) &&
        (pTimer->tick) )
    {
        pTimer->tick--;
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TimerExpired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the timer expired or not
 * INPUT   : pTimer -- pointer to the LACP_TIMER_T structure which will we
 *                     want to check expired.
 * OUTPUT  : None
 * RETURN  : TRUE   -- Expired
 *           FALSE  -- Not Expired
 * NOTE    : Only decrease when tick is not 0 and timer is "Enabled".
 *------------------------------------------------------------------------*/
static BOOL_T LACP_TimerExpired( LACP_TIMER_T *pTimer)
{
    if( (pTimer->enabled) &&
        (!(pTimer->tick)) )
    {
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Detach_Mux_From_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function will detach the port from the aggregator.
 * INPUT   : pPort -- pointer to port which want to be detached.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Detach_Mux_From_Aggregator( LACP_PORT_T *pPort)
{
    /*
     * Remember to set something, as detach it from this port.
     * Also remember to set it when STA is DISABLED because they will
     * have different hehavior.
     */
    UI32_T  idx;
    UI32_T  trunk_id;
    LACP_SYSTEM_T       *lacp_system_ptr;

    LACP_AGGREGATOR *pAgg = pPort->pAggregator;

    /* Now to see this is a single port or a true aggregator */
    if(!pAgg) /* NULL means port admin disabled */
    {
        return;
    }

    /* Remove from our data linked list structure */
    idx = LACP_Remove_From_Aggregator( pAgg, pPort);

    /* Normally the idx will be the index in the aggregator. */
    /* If it is 0, then it means we have some problems.      */
    if(!idx)
    {
        return;
    }

    /* The reason we have to check this just because we are not */
    /* going to add port when it select an aggregator. We will  */
    /* add it when we have an empty true aggregator when timer  */
    /* evaluate it.                                             */
    if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunk_id))
    {
        if(TRUE == TRK_PMGR_IsDynamicTrunkId( trunk_id))
        {
            UI32_T  port_ifindex;
            /* What should I do in Mercury? Actually, what has happen should always  */
            /* controled by SW_CTRL, don't you think so?                             */
            /* Here what we have to do are two things, one is let the interface down */
            /* the other is to remove the trunk if it is a trunk member.             */

            /* First we delete trunk members, about the interface status, that is    */
            /* taken cared by SWCTRL.                                                */
            /* if (TRK_PMGR_DeleteTrunkMember( trunk_id, pPort->slot, pPort->port)) */
            if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                if (TRK_PMGR_DeleteDynamicTrunkMember( trunk_id, pPort->slot, pPort->port))
                */
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                if(pPort->Is_static_lacp == FALSE)
                {
                    if (TRK_PMGR_DeleteDynamicTrunkMember( trunk_id, port_ifindex))
                    {
                        if(bLACP_DbgAthAL)
                        {
                            BACKDOOR_MGR_Printf("\nDetach_Mux_From_Aggregator Delete Trunk id:%d member slot:%d port:%d", (int)trunk_id, (int)pPort->slot, (int)pPort->port);
                        }
                        if (TRK_PMGR_GetTrunkMemberCounts(trunk_id)==0)
                        {
                            /* TRK_PMGR_DestroyTrunk(trunk_id); */ TRK_PMGR_FreeTrunkIdDestroyDynamic( trunk_id);
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nDetach_Mux_From_Aggregator Delete Trunk id:%d", (int)trunk_id);
                            }
                        }
                        lacp_system_ptr = LACP_OM_GetSystemPtr();
                        SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                    }
                }
#else
                if (TRK_PMGR_DeleteDynamicTrunkMember( trunk_id, port_ifindex))
                {
                    if(bLACP_DbgAthAL)
                    {
                        BACKDOOR_MGR_Printf("\nDetach_Mux_From_Aggregator Delete Trunk id:%d member slot:%d port:%d", (int)trunk_id, (int)pPort->slot, (int)pPort->port);
                    }
                    if (TRK_PMGR_GetTrunkMemberCounts(trunk_id)==0)
                    {
                        /* TRK_PMGR_DestroyTrunk(trunk_id); */ TRK_PMGR_FreeTrunkIdDestroyDynamic( trunk_id);
                        if(bLACP_DbgAthAL)
                        {
                            BACKDOOR_MGR_Printf("\nDetach_Mux_From_Aggregator Delete Trunk id:%d", (int)trunk_id);
                        }
                    }
                    lacp_system_ptr = LACP_OM_GetSystemPtr();
                    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                }
#endif
                /* The inter state is not set here.                                      */
            }
            else
            {
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nDetach_Mux_From_Aggregator :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                }
            } /* End of if (SWCTRL_UserPortToIfindex) */
        }
    }

    if(pAgg->NumOfPortsInAgg > 0)
    {
        pAgg->NumOfPortsInAgg--;
    }

    if(pAgg->AggID <= MAX_PHYSICAL_PORTS)
    {
        /* No matter this is a single port not not, we always let */
        /* the logical interface disappear from STP.              */

        /* In Mercury, the interface is controled by SW_CTRL, so  */
        /* we are not going to modify it anymore.                 */
        LACP_SetInterface( pPort->slot, pPort->port, FALSE);
    }

    pPort->pAggregator = NULL;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Disable_Collecting_Distributing
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable the collecting and distributing as
 *           describe in 802.3ad.
 * INPUT   : pPort -- pointer to port which want to be disabled.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Disable_Collecting_Distributing( LACP_PORT_T *pPort)
{
    UI32_T  trunkId;
    LACP_SYSTEM_T       *lacp_system_ptr;

    if(STKTPLG_POM_PortExist( pPort->slot, pPort->port) == FALSE)
    {
        return;
    }

    /* The other thing is when LACP is disabled in this port, then */
    /* we don't have to process this anymore                       */
    /*
     *EPR:NULL
     *Problem:    Lewis disable a code "if(pPort->LACPEnabled)"
                  if the code is disable ,it will cause the LACP send
                  lots of packet when enter transition mode
     *Solution:    Enable it,so when enter transition mode ,need not send                  LACP control packet
     *Approved by:Hardsun
     *Fixed by: DanXie
     */
    if(pPort->LACPEnabled) /* Lewis: maybe we need to remove this, so that APP set lacp admin to down, the state machine can still remove it from trunk */
    {
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\n\tLACP Disable_Collecting_Distributing on slot:%2d port:%2d", (int)pPort->slot, (int)pPort->port);
        }

        /* Here we have to decide whether we should let STP or LACP to take over the port state */
        /* When this port's collecting_distributing is disabled...                              */
        /* There is a chance that the remote partner just set its Sync to false  */
        /* and before that we are in COLLECTING_DISTRIBUTING state. Then we will */
        /* see the STP run into a wrong state.                                   */
        if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunkId))
        {
            /* Since this port is trunk member, so we have to remove this */
            /* port from trunk if it is a dynamic aggregation link.       */
            if(TRUE == TRK_PMGR_IsDynamicTrunkId( trunkId))
            {
                UI32_T  port_ifindex;
                /* if (TRK_PMGR_DeleteTrunkMember( trunkId, pPort->slot, pPort->port)) */
                if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                    if (TRK_PMGR_DeleteDynamicTrunkMember( trunkId, pPort->slot, pPort->port))
                    */
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
                    if(pPort->Is_static_lacp == FALSE)
                    {
                        if (TRK_PMGR_DeleteDynamicTrunkMember( trunkId, port_ifindex))
                        {
                            lacp_system_ptr = LACP_OM_GetSystemPtr();
                            SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));

                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nLACP_Disable_Collecting_Distributing Delete Trunk id:%d member slot:%d port:%d", (int)trunkId, (int)pPort->slot, (int)pPort->port);
                            }
                            if(TRK_PMGR_GetTrunkMemberCounts(trunkId)==0)
                            {
                                /* TRK_PMGR_DestroyTrunk(trunkId); */ TRK_PMGR_FreeTrunkIdDestroyDynamic( trunkId);
                                if(bLACP_DbgAthAL)
                                {
                                    BACKDOOR_MGR_Printf("\nLACP_Disable_Collecting_Distributing Delete Trunk id:%d", (int)trunkId);
                                }

                            }
                        }
                        else
                        {
                            if (bLACP_DbgMsg)
                            {
                                BACKDOOR_MGR_Printf("\nFile: %s Line: %d Slot: %d Port: %d fail to remove Dynamic-Trunk.", __FILE__, __LINE__, (int)pPort->slot,(int) pPort->port);
                            }

                        }
                    }
#else
                    if (TRK_PMGR_DeleteDynamicTrunkMember( trunkId, port_ifindex))
                    {
                        lacp_system_ptr = LACP_OM_GetSystemPtr();
                        SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));

                        if(bLACP_DbgAthAL)
                        {
                            BACKDOOR_MGR_Printf("\nLACP_Disable_Collecting_Distributing Delete Trunk id:%d member slot:%d port:%d", (int)trunkId, (int)pPort->slot, (int)pPort->port);
                        }
                        if(TRK_PMGR_GetTrunkMemberCounts(trunkId)==0)
                        {
                            /* TRK_PMGR_DestroyTrunk(trunkId); */ TRK_PMGR_FreeTrunkIdDestroyDynamic( trunkId);
                            if(bLACP_DbgAthAL)
                            {
                                BACKDOOR_MGR_Printf("\nLACP_Disable_Collecting_Distributing Delete Trunk id:%d", (int)trunkId);
                            }

                        }
                    }
                    else
                    {
                        if (bLACP_DbgMsg)
                        {
                            BACKDOOR_MGR_Printf("\nFile: %s Line: %d Slot: %d Port: %d fail to remove Dynamic-Trunk.", __FILE__, __LINE__, (int)pPort->slot,(int) pPort->port);
                        }

                    }
#endif
                }
                else
                {
                    if(bLACP_DbgAthAL)
                    {
                        BACKDOOR_MGR_Printf("\nLACP_Disable_Collecting_Distributing :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                    }
                } /* End of if (SWCTRL_UserPortToIfindex) */
            }
        }

        LACP_SetInterface( pPort->slot, pPort->port, FALSE);
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Attach_Mux_To_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function will attach port to aggregator.
 * INPUT   : pPort -- pointer to port which want to be attached.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Actually, this is a funny problem. Whether we should let the
 *           logical interface to be up at this time or when port get into
 *           ENABLE_COLLECTING_DISTRIBUTING state?
 *------------------------------------------------------------------------*/
void LACP_Attach_Mux_To_Aggregator( LACP_PORT_T *pPort)
{
    /* Here we should just attach it to aggregator.          */
    /* Note, the data structure is maintained when we select */
    /* the related aggregator so we won't do anymore here.   */
    /* Mark by ckhsu 2001/12/07                              */
    /* Now we want the STP run when port enter Collecting/   */
    /* Distributing state                                    */
    /*
    LACP_AGGREGATOR *pAgg = pPort->pAggregator;
    UI32_T           al_id;

    if(FALSE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &al_id))
    {
        STA_SetInterface( pPort->slot, pPort->port, TRUE);
    }
    */

    return;

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Collecting_Distributing
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable collecting and distributing of the
 *           port.
 * INPUT   : pPort -- pointer to port which want to be enabled.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Actually, what should we do here? From the standard, we should
 *           enable/disable the collecting/distributing here, but due to
 *           we don't have this function in chip, we can only do nothing
 *           here and let the STP to handle all things. This is the same
 *           as HP's implementation.
 *------------------------------------------------------------------------*/
void LACP_Enable_Collecting_Distributing( LACP_PORT_T *pPort)
{
    if(STKTPLG_POM_PortExist( pPort->slot, pPort->port) == FALSE)
    {
        return;
    }

    /* if(pPort->LACPEnabled) */
    if(pPort->LACP_OperStatus)
    {
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\n\tLACP Enable_Collecting_Distributing on slot:%2d port:%2d", (int)pPort->slot, (int)pPort->port);
        }

        /* A funny thinking is how should we handle the case when STP is disabled? */
        /* In several cases, it will cause the looping in network topology.        */
        /* How to do that in STP is disabled? .........                            */
        /* We shall set enable_collecting_distributing to SWCTRL even if the specified port is a trunk member.*/
        /* if(FALSE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &al_id)) */
        {
            /* Wait until I know how to do this in Mercury. */
            #if 0
            STA_SetInterface( pPort->slot, pPort->port, TRUE);
            #endif
            LACP_SetInterface( pPort->slot, pPort->port, TRUE);
        }
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_EnablePort
 *------------------------------------------------------------------------
 * FUNCTION: This function should be called when outside function try to
 *           enable/disable of a port.
 * INPUT   : pPort  -- pointer to port which want to be enabled/disabled.
 *           enable -- TRUE/FALSE for enable/disable a port.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Actually, we have to think in MC2 and Foxfire about this case.
 *           When port is enabled/disabled, the port should try to take
 *           care of a logical interface since it is mixed in MC2/foxfire.
 *------------------------------------------------------------------------*/
void LACP_EnablePort( LACP_PORT_T *pPort, BOOL_T enable)
{
    pPort->PortEnabled = enable;

    if( (pPort->LinkUp) &&
        (pPort->PortEnabled) )
    {
        pPort->Port_OperStatus = TRUE;
    }
    else
    {
        pPort->Port_OperStatus = FALSE;
    }

    if(enable)
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
    }
    else
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Current_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the current_while_timer
 * INPUT   : pPort   -- pointer to port which want to be enabled/disabled.
 *           timeout -- timer tick type (long/short)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Current_While_Timer( LACP_PORT_T *pPort, LACP_TIMEOUT_E timeout)
{
    switch(timeout)
    {
    case LACP_LONG_TIMEOUT:
        LACP_StartTimer( &((pPort->port_timer).current_while_timer), LACP_LONG_TIMEOUT_TIME);
        break;
    case LACP_SHORT_TIMEOUT:
        LACP_StartTimer( &((pPort->port_timer).current_while_timer), LACP_SHORT_TIMEOUT_TIME);
        break;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Current_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the current_while_timer
 * INPUT   : pPort   -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Current_While_Timer( LACP_PORT_T *pPort)
{
    LACP_StopTimer( &((pPort->port_timer).current_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Current_While_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the current_while_timer status
 * INPUT   : pPort -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Current_While_Timer_Enabled( LACP_PORT_T *pPort)
{
    return LACP_TimerIsEnabled( &((pPort->port_timer).current_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Current_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the current_while_timer tick
 * INPUT   : pPort -- pointer to port which want to be decreased.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Current_While_Timer( LACP_PORT_T *pPort)
{
    LACP_TimerDecrease( &((pPort->port_timer).current_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Current_While_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the current_while_timer expired
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Current_While_Timer_Expired( LACP_PORT_T *pPort)
{
    return LACP_TimerExpired( &((pPort->port_timer).current_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Periodic_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the periodic timer of a port.
 * INPUT   : pPort   -- pointer to port which want to be started.
 *           timeout -- timeout type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Periodic_Timer( LACP_PORT_T *pPort, LACP_TIMEOUT_E timeout)
{
    switch(timeout)
    {
    case LACP_LONG_TIMEOUT:
        LACP_StartTimer( &((pPort->port_timer).periodic_timer), LACP_SLOW_PERIODIC_TIME);
        break;
    case LACP_SHORT_TIMEOUT:
        LACP_StartTimer( &((pPort->port_timer).periodic_timer), LACP_FAST_PERIODIC_TIME);
        break;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Periodic_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the periodic timer of a port.
 * INPUT   : pPort   -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Periodic_Timer( LACP_PORT_T *pPort)
{
    LACP_StopTimer( &((pPort->port_timer).periodic_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Periodic_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the periodic timer of a port is
 *           enabled or not.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Periodic_Timer_Enabled( LACP_PORT_T *pPort)
{
    return LACP_TimerIsEnabled( &((pPort->port_timer).periodic_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Periodic_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the periodic timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Periodic_Timer( LACP_PORT_T *pPort)
{
    LACP_TimerDecrease( &((pPort->port_timer).periodic_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Periodic_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the periodic timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Expired
 *           FALSE -- Not expired.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Periodic_Timer_Expired( LACP_PORT_T *pPort)
{
    return LACP_TimerExpired( &((pPort->port_timer).periodic_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Wait_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be start.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Wait_While_Timer( LACP_PORT_T *pPort)
{
    /* LACP_StartTimer( &((pPort->port_timer).wait_while_timer), LACP_AGGREGATE_WAIT_TIME); */
    /* Due to our limitation, we want it take place immediately. */

    /* Allenc, compliant to the standard
    LACP_StartTimer( &((pPort->port_timer).wait_while_timer), LACP_AGGREGATE_NOWAIT_TIME);
    */
    LACP_StartTimer( &((pPort->port_timer).wait_while_timer), LACP_AGGREGATE_WAIT_TIME);

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Wait_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Wait_While_Timer( LACP_PORT_T *pPort)
{
    LACP_StopTimer( &((pPort->port_timer).wait_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Wait_While_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Wait_While_Timer_Enabled( LACP_PORT_T *pPort)
{
    return LACP_TimerIsEnabled( &((pPort->port_timer).wait_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Wait_While_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Wait_While_Timer( LACP_PORT_T *pPort)
{
    LACP_TimerDecrease( &((pPort->port_timer).wait_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Wait_While_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the wait_while timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Expired
 *           FALSE -- Not expired
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Wait_While_Timer_Expired( LACP_PORT_T *pPort)
{
    return LACP_TimerExpired( &((pPort->port_timer).wait_while_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Actor_Churn_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the actor_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be start.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Actor_Churn_Timer( LACP_PORT_T *pPort)
{
    LACP_StartTimer( &((pPort->port_timer).actor_churn_timer), LACP_CHURN_DETECTION_TIME);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Actor_Churn_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the actor_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Actor_Churn_Timer( LACP_PORT_T *pPort)
{
    LACP_StopTimer( &((pPort->port_timer).actor_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Actor_Churn_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the actor_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Actor_Churn_Timer_Enabled( LACP_PORT_T *pPort)
{
    return LACP_TimerIsEnabled( &((pPort->port_timer).actor_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Actor_Churn_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the actor_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Actor_Churn_Timer( LACP_PORT_T *pPort)
{
    LACP_TimerDecrease( &((pPort->port_timer).actor_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Actor_Churn_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the actor_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Expired
 *           FALSE -- Not expired
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Actor_Churn_Timer_Expired( LACP_PORT_T *pPort)
{
    return LACP_TimerExpired( &((pPort->port_timer).actor_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Start_Partner_Churn_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will start the partner_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be start.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Start_Partner_Churn_Timer( LACP_PORT_T *pPort)
{
    LACP_StartTimer( &((pPort->port_timer).partner_churn_timer), LACP_CHURN_DETECTION_TIME);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Stop_Partner_Churn_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will stop the partner_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be stop.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Stop_Partner_Churn_Timer( LACP_PORT_T *pPort)
{
    LACP_StopTimer( &((pPort->port_timer).partner_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Partner_Churn_Timer_Enabled
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the partner_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Enabled
 *           FALSE -- Disabled
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Partner_Churn_Timer_Enabled( LACP_PORT_T *pPort)
{
    return LACP_TimerIsEnabled( &((pPort->port_timer).partner_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decrease_Partner_Churn_Timer
 *------------------------------------------------------------------------
 * FUNCTION: This function will decrease the partner_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Decrease_Partner_Churn_Timer( LACP_PORT_T *pPort)
{
    LACP_TimerDecrease( &((pPort->port_timer).partner_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Partner_Churn_Timer_Expired
 *------------------------------------------------------------------------
 * FUNCTION: This function will check the partner_churn timer of a port.
 * INPUT   : pPort -- pointer to port which want to be checked.
 * OUTPUT  : None
 * RETURN  : TRUE  -- Expired
 *           FALSE -- Not expired
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Partner_Churn_Timer_Expired( LACP_PORT_T *pPort)
{
    return LACP_TimerExpired( &((pPort->port_timer).partner_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_SetLACPState
 *------------------------------------------------------------------------
 * FUNCTION: This function will set Enable/Disable LACP of a port.
 * INPUT   : slot   -- slot number of the port
 *           port   -- port number of the port
 *           enable -- TRUE/FALSE for Enable/Disable
 * OUTPUT  : None
 * RETURN  : Error Status of setting value.
 * NOTE    : None
 *------------------------------------------------------------------------*/
#if 0
LACP_RETURN_VALUE LACP_SetLACPState( UI32_T slot, UI32_T port, BOOL_T enable)
{
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( slot, port) == FALSE)
    {
        return LACP_RETURN_PORT_NOT_EXIST;
    }

    pPort = LACP_FIND_PORT( slot, port);

    if(pPort->LACPEnabled == enable)
    {
        return LACP_RETURN_SUCCESS;
    }

    pPort->LACPEnabled = enable;

    if( (pPort->LACPEnabled == LACP_ADMIN_ON) &&
        (pPort->FullDuplexMode) )
    {
        pPort->LACP_OperStatus = TRUE;
    }
    else
    {
        pPort->LACP_OperStatus = FALSE;
    }

    /* What should we do here?                               */
    /* In fact, either LACP or physical port is disabled, we */
    /* should just trigger the related event to let LACP     */
    /* finite state machine to do the related job.           */
    if(enable)
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
    }
    else
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);
    }

    return LACP_RETURN_SUCCESS;
}
#endif
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetLACPState
 *------------------------------------------------------------------------
 * FUNCTION: This function will get Enable/Disable LACP of a port.
 * INPUT   : slot    -- slot number of the port
 *           port    -- port number of the port
 * OUTPUT  : pEnable -- pointer to variable which will take this value
 *                      where TRUE/FALSE for Enable/Disable.
 * RETURN  : Error Status of getting value.
 * NOTE    : None
 *------------------------------------------------------------------------*/
#if 0
LACP_RETURN_VALUE LACP_GetLACPState( UI32_T slot, UI32_T port, BOOL_T *pEnable)
{
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( slot, port) == FALSE)
    {
        return LACP_RETURN_PORT_NOT_EXIST;
    }

    pPort = LACP_FIND_PORT( slot, port);

    *pEnable = pPort->LACPEnabled;

    return LACP_RETURN_SUCCESS;
}
#endif
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Generate_PDU
 *------------------------------------------------------------------------
 * FUNCTION: This function will generate a 802.3ad PDU according to type
 * INPUT   : pPort -- pointer to the port which want to generate PDU.
 *           type  -- Pdu type to be generated
 * OUTPUT  : pPdu  -- pointer to variable which will take Pdu
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Generate_PDU( LACP_PORT_T *pPort, LACP_PDU_U  *pPdu, LACP_PDU_TYPE_E type)
{
    /* I won't check passin parameter, caller should make sure passin parameter */
    /* are always correct.                                                      */
    /* Besides this, we are not going to set SA, DA, and ethertype that will be */
    /* set in LACP_TX_SendPDU.                                                  */
    /* Remember user should clear memory by himself.                            */
    if(type == LACP_LAC_PDU)
    {
        (pPdu->lacp).protocol_subtype = LACP_LACP;
    }
    else
    {
        /* It must be a Marker PDU or Marker Response PDU */
        (pPdu->marker).protocol_subtype = LACP_MARKER;
    }

    /* We only support version 1 of LACP and Marker protocol. */
    /* From program view, that data structure are the same.   */
    (pPdu->lacp).protocol_version = 0x01;

    switch(type)
    {
    case LACP_LAC_PDU:
        (pPdu->lacp).tlv_type_actor         = LACP_TLV_TYPE_ACTOR_INFORMATION;
        (pPdu->lacp).actor_info_length      = LACP_ACTOR_INFORMATION_LENGTH;
        /*copy_info( &(pPort->AggActorPort), &((pPdu->lacp).actor));*/
        LACP_TYPE_LAC_INFO_HtoNCpy( (pPdu->lacp).actor, pPort->AggActorPort );

        (pPdu->lacp).tlv_type_partner       = LACP_TLV_TYPE_PARTNER_INFORMATION;
        (pPdu->lacp).partner_info_length    = LACP_PARTNER_INFORMATION_LENGTH;
        /*copy_info( &(pPort->AggPartnerPort), &((pPdu->lacp).partner));*/
        LACP_TYPE_LAC_INFO_HtoNCpy( (pPdu->lacp).partner, pPort->AggPartnerPort );

        (pPdu->lacp).tlv_type_collector     = LACP_TLV_TYPE_COLLECTOR_INFORMATION;
        (pPdu->lacp).collector_info_length  = LACP_COLLECTOR_INFORMATION_LENGTH;
        /*(pPdu->lacp).collector_max_delay    = LACP_COLLECTOR_MAX_DELAY;*/
        (pPdu->lacp).collector_max_delay    = L_STDLIB_Hton16( LACP_COLLECTOR_MAX_DELAY );

        (pPdu->lacp).tlv_type_terminator    = LACP_TLV_TYPE_TERMINATOR_INFORMATION;
        (pPdu->lacp).terminator_info_length = LACP_TERMINATOR_INFORMATION_LENGTH;
        break;
    case LACP_MARKER_INFO_PDU:
    case LACP_MARKER_RESPONSE_PDU:
        /* Currently we are not going to use Marker information. */
        if(type == LACP_MARKER_INFO_PDU)
        {
            (pPdu->marker).tlv_type_marker_infomation = LACP_TLV_TYPE_MARKER_INFORMATION;
        }
        else
        {
            (pPdu->marker).tlv_type_marker_infomation = LACP_TLV_TYPE_MARKER_RESPONSE;
        }
        (pPdu->marker).marker_info_length     = LACP_MARKER_INFORMATION_LENGTH;
        /* request port should be this port. */
        /*(pPdu->marker).request_port           = (pPort->AggActorPort).port_no;*/
        (pPdu->marker).request_port           = L_STDLIB_Hton16( (pPort->AggActorPort).port_no );
        /* request system and transaction id should be filled in by requester. */
        (pPdu->marker).tlv_type_terminator    = LACP_TLV_TYPE_TERMINATOR_INFORMATION;
        (pPdu->marker).terminator_info_length = LACP_TERMINATOR_INFORMATION_LENGTH;
        break;
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Check_PDU
 *------------------------------------------------------------------------
 * FUNCTION: This function will check a 802.3ad PDU according to type
 * INPUT   : pPdu -- pointer to the Pdu which want to be checked.
 *           type -- Pdu type to be generated
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE for a correct/incorrect type of Pdu.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Check_PDU( LACP_PDU_U  *pPdu, LACP_PDU_TYPE_E type)
{
    switch(type)
    {
    case LACP_LAC_PDU:
        #if 0
        if( ((pPdu->lacp).ethertype != LACPDU_Slow_Protocols_Type) ||
        #endif
        if(
            ((pPdu->lacp).protocol_subtype != LACP_LACP) ||
            /* In RxMachine section, we don't need to validate the following information field. */
            /* ((pPdu->lacp).tlv_type_actor != LACP_TLV_TYPE_ACTOR_INFORMATION) || */
            ((pPdu->lacp).actor_info_length != LACP_ACTOR_INFORMATION_LENGTH) ||
            /* ((pPdu->lacp).tlv_type_partner != LACP_TLV_TYPE_PARTNER_INFORMATION) || */
            ((pPdu->lacp).partner_info_length != LACP_PARTNER_INFORMATION_LENGTH) ||
            /* ((pPdu->lacp).tlv_type_collector != LACP_TLV_TYPE_COLLECTOR_INFORMATION) || */
            ((pPdu->lacp).collector_info_length != LACP_COLLECTOR_INFORMATION_LENGTH) ||
            /* ((pPdu->lacp).tlv_type_terminator != LACP_TLV_TYPE_TERMINATOR_INFORMATION) || */
            ((pPdu->lacp).terminator_info_length != LACP_TERMINATOR_INFORMATION_LENGTH) )
        {
            goto ERRONEOUS_PDU;
        }
        break;
    case LACP_MARKER_INFO_PDU:
    case LACP_MARKER_RESPONSE_PDU:
        /* I check these two type in the same place. */
        #if 0
        if( ((pPdu->marker).ethertype != LACPDU_Slow_Protocols_Type) ||
        #endif
        if(
            ((pPdu->marker).protocol_subtype != LACP_MARKER) ||
            /* Here maybe be Info or Response */
            /* (((pPdu->marker).tlv_type_marker_infomation != LACP_TLV_TYPE_MARKER_INFORMATION) && ((pPdu->marker).tlv_type_marker_infomation != LACP_TLV_TYPE_MARKER_RESPONSE)) || */
            ((pPdu->marker).marker_info_length != LACP_MARKER_INFORMATION_LENGTH)
            /* ((pPdu->marker).tlv_type_terminator != LACP_TLV_TYPE_TERMINATOR_INFORMATION) || */
            /* ((pPdu->marker).terminator_info_length != LACP_TERMINATOR_INFORMATION_LENGTH)*/ )
        {
            goto ERRONEOUS_PDU;
        }
        break;
    }
    return TRUE;

ERRONEOUS_PDU:
    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_Default_Selected
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will update the default
 *           SELECTED value of a port.
 * INPUT   : pPort -- pointer to the port which want to update.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Update_Default_Selected( LACP_PORT_T *pPort)
{
    /* Compare Partner admin value with operational value, if any different */
    /* then its Selected should change to UNSELECTED.                       */
    if(!same_Partner( &(pPort->AggPartnerPort), &(pPort->AggPartnerAdminPort)))
    {
        pPort->Selected = UNSELECTED;

        /*(((pPort->AggActorPort).state).lacp_port_state).Synchronization = FALSE;*/
        LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Synchronization);
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_Selected
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will update the
 *           SELECTED value of a port according to receive Pdu.
 * INPUT   : pPort -- pointer to the port which want to update.
 *           pPdu  -- pointer to the Pdu which will be used to compare.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Update_Selected( LACP_PORT_T *pPort, LACP_PDU_U *pPdu)
{
    LAC_INFO_T after_convert;

    /* Convert actor information in PDU from network before comparsion */
    LACP_TYPE_LAC_INFO_NtoHCpy(after_convert, (pPdu->lacp).actor);

    /* Compare actors in PDU with current partner's information. */
    /*if(!same_Partner( &((pPdu->lacp).actor), &(pPort->AggPartnerPort)))*/
    if(!same_Partner(&after_convert, &(pPort->AggPartnerPort)))
    {
        pPort->Selected = UNSELECTED;
        /* Although in standard it didnot describe that if it is not the same */
        /* then the Synch should change, but actually, it need to change.     */
        /*(((pPort->AggActorPort).state).lacp_port_state).Synchronization = FALSE;*/
        LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Synchronization);
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Update_NTT
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will update the
 *           NTT value of a port according to receive Pdu.
 * INPUT   : pPort -- pointer to the port which want to update.
 *           pPdu  -- pointer to the Pdu which will be used to compare.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Update_NTT( LACP_PORT_T *pPort, LACP_PDU_U *pPdu)
{
    LAC_INFO_T after_convert;

    /* Convert partner information in PDU from network before comparsion */
    LACP_TYPE_LAC_INFO_NtoHCpy(after_convert, (pPdu->lacp).partner);

    /* I don't think we need to compare the Aggregation again since we */
    /* will compare it inside same_Partner. Besides this, now we don't */
    /* need to compare the collecting in 2000 standard.                */
    /*if( (!same_Partner( &((pPdu->lacp).partner), &(pPort->AggActorPort))) ||*/
    if( (!same_Partner( &after_convert, &(pPort->AggActorPort))) ||
        (LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).partner).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity) != LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity)) ||
        (LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).partner).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout) != LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout)) ||
        (LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).partner).state).port_state, LACP_dot3adAggPortActorOperState_Synchronization) != LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Synchronization))
        /*(((((pPdu->lacp).partner).state).lacp_port_state).LACP_Activity != (((pPort->AggActorPort).state).lacp_port_state).LACP_Activity) ||*/
        /*(((((pPdu->lacp).partner).state).lacp_port_state).LACP_Timeout != (((pPort->AggActorPort).state).lacp_port_state).LACP_Timeout) ||*/
        /*(((((pPdu->lacp).partner).state).lacp_port_state).Synchronization != (((pPort->AggActorPort).state).lacp_port_state).Synchronization) )*/
      )
    {
        pPort->NTT = TRUE;
    }

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_recordPDU
 *------------------------------------------------------------------------
 * FUNCTION: This function is as describe in 802.3ad will record the
 *           Pdu content to a port according to receive Pdu.
 * INPUT   : pPort -- pointer to the port which want to set.
 *           pPdu  -- pointer to the Pdu which will be used to record.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_recordPDU( LACP_PORT_T *pPort, LACP_PDU_U *pPdu)
{
    /* First we copy the information from PDU. */
        /*copy_info( &((pPdu->lacp).actor), &(pPort->AggPartnerPort));*/
        LACP_TYPE_LAC_INFO_NtoHCpy( pPort->AggPartnerPort, (pPdu->lacp).actor );
    /* Then we check for the necessary info from Partner and Actor to set */
    /* Synchronization. First we decide whether Partner are in            */
    /* Synchronization or not.      */


    /* Set Defaulted to false. */
    /*(((pPort->AggActorPort).state).lacp_port_state).Defaulted = FALSE;*/
    LACP_TYPE_SET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, ((BOOL_T)(FALSE)), LACP_dot3adAggPortActorOperState_Defaulted);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Choose_Matched
 *------------------------------------------------------------------------
 * FUNCTION: This function is not mentioned in 802.3ad but it is necessary.
 *           Because this is the residue to LACP_recordPDU.
 * INPUT   : pPort -- pointer to the port which want to set.
 *           pPdu  -- pointer to the Pdu which will be used to check.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Choose_Matched( LACP_PORT_T *pPort, LACP_PDU_U *pPdu)
{
    LAC_INFO_T after_convert;

    /* Convert partner information in PDU from network before comparsion */
    LACP_TYPE_LAC_INFO_NtoHCpy(after_convert, (pPdu->lacp).partner);

#if 0
    if(  (same_Partner( &((pPdu->lacp).partner), &(pPort->AggActorPort)) ||
          (!(((((pPdu->lacp).actor).state).lacp_port_state).Aggregation))  )
     && (  (((((pPdu->lacp).actor).state).lacp_port_state).LACP_Activity) || (   (((pPort->AggActorPort).state).lacp_port_state).LACP_Activity &&  ((((pPdu->lacp).partner).state).lacp_port_state).LACP_Activity)  )  )
#endif
    /*if( ( (same_Partner( &((pPdu->lacp).partner), &(pPort->AggActorPort)) && ( ((((pPdu->lacp).partner).state).lacp_port_state).Aggregation == (((pPort->AggActorPort).state).lacp_port_state).Aggregation)) ||*/
    /*      (!(((((pPdu->lacp).actor).state).lacp_port_state).Aggregation))  )*/
    /* && (  (((((pPdu->lacp).actor).state).lacp_port_state).LACP_Activity) || (   (((pPort->AggActorPort).state).lacp_port_state).LACP_Activity &&  ((((pPdu->lacp).partner).state).lacp_port_state).LACP_Activity)  )  )*/
    if(  (   (   same_Partner(&after_convert, &(pPort->AggActorPort))
             && (LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).partner).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation) == LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation))
            )
          ||(!LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).actor).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation))
         )
      && (   LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).actor).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity)
          || (   LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity)
              && LACP_TYPE_GET_PORT_STATE_BIT((((pPdu->lacp).partner).state).port_state, LACP_dot3adAggPortActorOperState_LACP_Activity)
            )
         )
     )
    {
        pPort->Matched = TRUE;  /*Lewis: Matched means (Same Partner or Individual) & One side is Actively */
                              /* Matched is used with received PDU's Actor's Sync to decide Part.Sync in Mux machine */
    }
    else
    {
        pPort->Matched = FALSE;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_HandlePdu
 *------------------------------------------------------------------------
 * FUNCTION: This function will handle the received Pdu from NIC.
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_HandlePdu(UI32_T unit, UI32_T port,  L_MM_Mref_Handle_T *mref_handle_p)
{
    LACP_PDU_U  *pPdu;
    LACP_PORT_T *pPort;
    UI32_T       pdu_len;

    pPdu = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if(pPdu==NULL)
        return;

    /* In 802.3ad, the frame format is always the same, so I just set it. */
    /* but remember to check frame length.                                */
    if(pdu_len != sizeof(LACP_PDU_U)) /* Lewis: Not including DA,SA, Type, CRC in pdu_len -> 128-14-4=110. */
    {
        /* Data length is invalid, discard it, this will not count into */
        /* MIB.                                                         */
        if (bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\nLACP_HandlePdu(): invalid mem_ref->pdu_len = %d\n", (int)pdu_len);
        }

        L_MM_Mref_Release(&mref_handle_p);
        return;
    }


    {
        /* Send to LACP RxMachine. At RxMachine, it will dispatch to MarkerMachine */
        /* if that is a Marker type frame.                                         */
        pPort = LACP_FIND_PORT( unit, port);

       if(bLACP_DbgPdu)
        {
            BACKDOOR_MGR_Printf("\n----------------------------RX LACP -----");
            BACKDOOR_MGR_Printf("\n\nRx PDU at Unit:%2d Port:%2d\n", (int)unit, (int)port);
            LACP_Print_PDU_Info( pPdu);
        }

        LACP_Rx_Machine( pPort, LACP_RX_PDU_EV, pPdu);
        LACP_Mux_Machine( pPort, LACP_NEW_INFO_EV);
        /* Then we have to check port moved or not. */
    }
    L_MM_Mref_Release(&mref_handle_p);

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Information
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : slot -- slot
 *           port -- port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_Information( UI32_T slot, UI32_T port)
{
    LACP_PORT_T *pPort;

    if(!STKTPLG_POM_PortExist( slot, port))
    {
        if(bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\nSTK_TPLG said Slot:%d Port:%d is not in system.\n", (int)slot, (int)port);
        }

        return;
    }

    pPort = LACP_FIND_PORT( slot, port);

    if(!pPort)
    {
        if (bLACP_DbgMsg)
        {
            BACKDOOR_MGR_Printf("\nFail to locate LACP port data");
        }

        return;
    }

    BACKDOOR_MGR_Printf("\nSlot:%2d Port:%2d", (int)pPort->slot, (int)pPort->port);
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\nActor Operational Values:");
    LACP_Show_Info( &(pPort->AggActorPort));
    BACKDOOR_MGR_Printf("\nActor Admin Values:");
    LACP_Show_Info( &(pPort->AggActorAdminPort));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\nPartner Operational Values:");
    LACP_Show_Info( &(pPort->AggPartnerPort));
    BACKDOOR_MGR_Printf("\nPartner Admin Values:");
    LACP_Show_Info( &(pPort->AggPartnerAdminPort));
    BACKDOOR_MGR_Printf("\nStatistics:");
    LACP_Show_Statistics( &(pPort->AggPortStats));
    BACKDOOR_MGR_Printf("\nDebug State:");
    LACP_Show_Debug_State( &(pPort->AggPortDebug));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\nAggregateOrIndividual:             %d", (int)pPort->AggPortAggregateOrIndividual);
    BACKDOOR_MGR_Printf("\nPortOperStatus(admin&linkup):      %s", (pPort->Port_OperStatus)?("True"):("False"));
    BACKDOOR_MGR_Printf("\nLACPOperStatus(admin&Fullduplex):  %s", (pPort->LACP_OperStatus)?("True"):("False"));
    BACKDOOR_MGR_Printf("\nPortEnabled(admin,0/1):            %d", (int)pPort->PortEnabled);
    BACKDOOR_MGR_Printf("\nLACPEnabled(admin,0/1):            %d", (int)pPort->LACPEnabled);
    BACKDOOR_MGR_Printf("\nLinkUp:                   %d", (int)pPort->LinkUp);
    BACKDOOR_MGR_Printf("\nNTT:                      %d", (int)pPort->NTT);
    BACKDOOR_MGR_Printf("\nPort Speed                %d", (int)pPort->PortSpeed);
    BACKDOOR_MGR_Printf("\nFull Duplex                %s", (pPort->FullDuplexMode)?("Full Duplex"):("Half Duplex"));
    BACKDOOR_MGR_Printf("\nSelected                  ");
    switch(pPort->Selected)
    {
    case UNSELECTED:
        BACKDOOR_MGR_Printf("UNSELECTED");
        break;
    case STANDBY:
        BACKDOOR_MGR_Printf("STANDBY");
        break;
    case SELECTED:
        BACKDOOR_MGR_Printf("SELECTED");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }
    BACKDOOR_MGR_Printf("\nMatched:                  %d", (int)pPort->Matched);
    BACKDOOR_MGR_Printf("\nActor Churn:              %d", (int)pPort->ActorChurn);
    BACKDOOR_MGR_Printf("\nPartner Churn:            %d", (int)pPort->PartnerChurn);
    BACKDOOR_MGR_Printf("\nFrames In Last Second:    %d", (int)pPort->FramesInLastSec);
    BACKDOOR_MGR_Printf("\nLogicalTrunkMemberFlag:   %d", (int)pPort->LogicalTrunkMemberFlag);
    BACKDOOR_MGR_Printf("\nDisconDelayedFlag:        %d", (int)pPort->DisconDelayedFlag);
    BACKDOOR_MGR_Printf("\nPortBlockedFlag:          %d", (int)pPort->PortBlockedFlag);
    BACKDOOR_MGR_Printf("\nInitialPDUTimer:          %d", (int)pPort->InitialPDUTimer);
    BACKDOOR_MGR_Printf("\nPortMoved:                %d", (int)pPort->PortMoved);

    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\nLACP Timer Information:");
    LACP_Show_PortTimer( &(pPort->port_timer));

    BACKDOOR_MGR_Printf("\n");
    LACP_Show_Aggregator( pPort->pAggregator);

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Info
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : info -- pointer to a LAC_INFO_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_Info( LAC_INFO_T *info)
{
    if(!info)
    {
        return;
    }

    BACKDOOR_MGR_Printf("\n\tSystem Priority:   0x%04x", info->system_priority);
    BACKDOOR_MGR_Printf("\n\tSystem Identifier: %02x-%02x-%02x-%02x-%02x-%02x",info->system_id[0],
                                                            info->system_id[1],
                                                            info->system_id[2],
                                                            info->system_id[3],
                                                            info->system_id[4],
                                                            info->system_id[5]);
    BACKDOOR_MGR_Printf("\n\tPort Key:          0x%04x", info->key);
    BACKDOOR_MGR_Printf("\n\tPort Priority:     0x%04x", info->port_priority);
    BACKDOOR_MGR_Printf("\n\tPort Number:       %4d", info->port_no);

    LACP_Decode_PortStateInfo( &(info->state));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Decode_PortStateInfo
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : p -- pointer to a LACP_PORT_STATE_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Decode_PortStateInfo(LACP_PORT_STATE_T *p)
{
    BACKDOOR_MGR_Printf("\n\tPort State:        0x%1x", p->port_state);
    BACKDOOR_MGR_Printf("\n\t\tLACP Activity:     %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_LACP_Activity))?("Active"):("Passive"));
    BACKDOOR_MGR_Printf("\n\t\tLACP Timeout:      %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_LACP_Timeout))?("Short Timeout"):("Long Timeout"));
    BACKDOOR_MGR_Printf("\n\t\tAggregation:       %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_Aggregation))?("Aggregatable"):("Individual"));
    BACKDOOR_MGR_Printf("\n\t\tSynchronization:   %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_Synchronization))?("IN_SYNC"):("OUT_OF_SYNC"));
    BACKDOOR_MGR_Printf("\n\t\tCollecting:        %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_Collecting))?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n\t\tDistributing:      %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_Distributing))?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n\t\tDefaulted:         %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_Defaulted))?("Defaulted"):("LACPDU"));
    BACKDOOR_MGR_Printf("\n\t\tExpired:           %s", (LACP_TYPE_GET_PORT_STATE_BIT(p->port_state, LACP_dot3adAggPortActorOperState_Expired))?("EXPIRED"):("not EXPIRED"));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Statistics
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : p -- pointer to a LACP_PORT_STATISTICS_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Show_Statistics( LACP_PORT_STATISTICS_T *p)
{
    BACKDOOR_MGR_Printf("\n\tReceive LACPDUs:               %8d", (int)p->LACPDUsRx);
    BACKDOOR_MGR_Printf("\n\tReceive MarkerPDUs:            %8d", (int)p->MarkerPDUsRx);
    BACKDOOR_MGR_Printf("\n\tReceive Marker Response PDUs:  %8d", (int)p->MarkerResponsePDUsRx);
    BACKDOOR_MGR_Printf("\n\tReceive Unknown PDUs:          %8d", (int)p->UnknownRx);
    BACKDOOR_MGR_Printf("\n\tReceive Illegal PDUs:          %8d", (int)p->IllegalRx);
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n\tTransmit LACPDUs:              %8d", (int)p->LACPDUsTx);
    BACKDOOR_MGR_Printf("\n\tTransmit MarkerPDUs:           %8d", (int)p->MarkerPDUsTx);
    BACKDOOR_MGR_Printf("\n\tTransmit Marker Response PDUs: %8d", (int)p->MarkerResponsePDUsTx);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Debug_State
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : p -- pointer to a LACP_PORT_DEBUG_INFO_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Show_Debug_State( LACP_PORT_DEBUG_INFO_T *p)
{
    BACKDOOR_MGR_Printf("\n\tRx State:               ");
    switch(p->RxState)
    {
    case Rxm_BEGIN:
        BACKDOOR_MGR_Printf("BEGIN");
        break;
    case Rxm_CURRENT:
        BACKDOOR_MGR_Printf("CURRENT");
        break;
    case Rxm_EXPIRED:
        BACKDOOR_MGR_Printf("EXPIRED");
        break;
    case Rxm_DEFAULTED:
        BACKDOOR_MGR_Printf("DEFAULTED");
        break;
    case Rxm_INITIALIZE:
        BACKDOOR_MGR_Printf("INITIALIZE");
        break;
    case Rxm_LACP_DISABLED:
        BACKDOOR_MGR_Printf("LACP_DISABLED");
        break;
    case Rxm_PORT_DISABLED:
        BACKDOOR_MGR_Printf("PORT_DISABLED");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }

    BACKDOOR_MGR_Printf("\n\tLast Rx Time:           %8d", (int)p->LastRxTime);
    BACKDOOR_MGR_Printf("\n\tMatch State:            %8d", (int)p->MatchState);

    BACKDOOR_MGR_Printf("\n\tMux State:              ");
    switch(p->MuxState)
    {
    case Muxm_BEGIN:
        BACKDOOR_MGR_Printf("BEGIN");
        break;
    case Muxm_DETACHED:
        BACKDOOR_MGR_Printf("DETACHED");
        break;
    case Muxm_WAITING:
        BACKDOOR_MGR_Printf("WAITING");
        break;
    case Muxm_ATTACHED:
        BACKDOOR_MGR_Printf("ATTACHED");
        break;
    case Muxm_COLLECTING:
        BACKDOOR_MGR_Printf("COLLECTING");
        break;
    case Muxm_DISTRIBUTING:
        BACKDOOR_MGR_Printf("DISTRIBUTING");
        break;
    case Muxm_COLLECTING_DISTRIBUTING:
        BACKDOOR_MGR_Printf("COLLECTING_DISTRIBUTING");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }
    #if 0
    BACKDOOR_MGR_Printf("\n\tMux Reason:             %s", p->MuxReason);
    #endif
    BACKDOOR_MGR_Printf("\n\tActor Churn State:      %8d", (int)p->ActorChurnState);
    BACKDOOR_MGR_Printf("\n\tPartner Churn State:    %8d", (int)p->PartnerChurnState);
    BACKDOOR_MGR_Printf("\n\tActor Churn Count:      %8d", (int)p->ActorChurnCount);
    BACKDOOR_MGR_Printf("\n\tPartner Churn Count:    %8d", (int)p->PartnerChurnCount);
    BACKDOOR_MGR_Printf("\n\tActor Sync Count:       %8d", (int)p->ActorSyncTransitionCount);
    BACKDOOR_MGR_Printf("\n\tPartner Sync Count:     %8d", (int)p->PartnerSyncTransitionCount);
    BACKDOOR_MGR_Printf("\n\tActor Change Count:     %8d", (int)p->ActorChangeCount);
    BACKDOOR_MGR_Printf("\n\tPartner Change Count:   %8d", (int)p->PartnerChangeCount);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_PortTimer
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : p -- pointer to a LACP_PORT_TIMER_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Show_PortTimer( LACP_PORT_TIMER_T *p)
{
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n\tCurrentWhile Timer:");
    LACP_ShowTimerInfo( &(p->current_while_timer));
    BACKDOOR_MGR_Printf("\n\tPeriodic Timer:");
    LACP_ShowTimerInfo( &(p->periodic_timer));
    BACKDOOR_MGR_Printf("\n\tWaitWhile Timer:");
    LACP_ShowTimerInfo( &(p->wait_while_timer));
    BACKDOOR_MGR_Printf("\n\tActorChurn Timer:");
    LACP_ShowTimerInfo( &(p->actor_churn_timer));
    BACKDOOR_MGR_Printf("\n\tPartnerChurn Timer:");
    LACP_ShowTimerInfo( &(p->partner_churn_timer));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_ShowTimerInfo
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : p -- pointer to a LACP_TIMER_T to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_ShowTimerInfo( LACP_TIMER_T *p)
{
    BACKDOOR_MGR_Printf("\n\t\t%s", (p->enabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n\t\t%d tick(s)", (int)p->tick);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used in backdoor to show information about
 *           a port.
 * INPUT   : p -- pointer to a LACP_AGGREGATOR to be printed.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_Show_Aggregator( LACP_AGGREGATOR *p)
{
    if(!p)
    {
        BACKDOOR_MGR_Printf("\nPort does not attach to any aggregator...");
        return;
    }

    BACKDOOR_MGR_Printf("\nAggregator Information:");
    BACKDOOR_MGR_Printf("\n\tIdentifier:               %3d", (int)p->AggID);
    #if 0
    BACKDOOR_MGR_Printf("\n\tDescription:              %s", p->AggDescription);
    #endif
    BACKDOOR_MGR_Printf("\n\tAggAggregateOrIndividual: %3d", (int)p->AggAggregateOrIndividual);
    BACKDOOR_MGR_Printf("\n\tMACAddress:               %02x-%02x-%02x-%02x-%02x-%02x",   p->AggMACAddress[0],
                                                                            p->AggMACAddress[1],
                                                                            p->AggMACAddress[2],
                                                                            p->AggMACAddress[3],
                                                                            p->AggMACAddress[4],
                                                                            p->AggMACAddress[5]);
    BACKDOOR_MGR_Printf("\n\tActor SyetmId:            %02x-%02x-%02x-%02x-%02x-%02x",   p->AggActorSystemID[0],
                                                                            p->AggActorSystemID[1],
                                                                            p->AggActorSystemID[2],
                                                                            p->AggActorSystemID[3],
                                                                            p->AggActorSystemID[4],
                                                                            p->AggActorSystemID[5]);
    BACKDOOR_MGR_Printf("\n\tActor System Priority:    0x%04x", (int)p->AggActorSystemPriority);
    BACKDOOR_MGR_Printf("\n\tActor Operational Key:    0x%08x", (int)p->AggActorOperKey);
    BACKDOOR_MGR_Printf("\n\tActor Admin Key:          0x%08x", (int)p->AggActorAdminKey);

    BACKDOOR_MGR_Printf("\n\tPartner SyetmId:          %02x-%02x-%02x-%02x-%02x-%02x",   p->AggPartnerSystemID[0],
                                                                            p->AggPartnerSystemID[1],
                                                                            p->AggPartnerSystemID[2],
                                                                            p->AggPartnerSystemID[3],
                                                                            p->AggPartnerSystemID[4],
                                                                            p->AggPartnerSystemID[5]);
    BACKDOOR_MGR_Printf("\n\tPartner System Priority:  0x%04x", (int)p->AggPartnerSystemPriority);
    BACKDOOR_MGR_Printf("\n\tPartner Operational Key:  0x%08x", (int)p->AggPartnerOperKey);

    BACKDOOR_MGR_Printf("\n\tAdmin State:              0x%02x", (int)p->AggAdminState);
    BACKDOOR_MGR_Printf("\n\tOperational State:        0x%02x", (int)p->AggOperState);
    BACKDOOR_MGR_Printf("\n\tTime of Last Oper. Change:%8d", (int)p->AggTimeOfLastOperChange);
    BACKDOOR_MGR_Printf("\n\tLinkUp/Down Enable Notify:0x%02x", (int)p->AggLinkUpDownNotificationEnable);
    BACKDOOR_MGR_Printf("\n\tLinkUp Notification:      %d", (int)p->AggLinkUpNotification);
    BACKDOOR_MGR_Printf("\n\tLinkDown Notification:    %d", (int)p->AggLinkDownNotification);
    BACKDOOR_MGR_Printf("\n\tCollectorMaxDelay:        %d", (int)p->AggCollectorMaxDelay);
    BACKDOOR_MGR_Printf("\n\tTimeout:                  %d", (int)p->AggActorTimeout);
    BACKDOOR_MGR_Printf("\n\tNumOfPortsInAgg:          %d", (int)p->NumOfPortsInAgg);
    BACKDOOR_MGR_Printf("\n\tNumOfSelectedPorts:       %d", (int)p->NumOfSelectedPorts);
    BACKDOOR_MGR_Printf("\n\tNumOfCollectingDistributingPorts:       %d", (int)p->NumOfCollectingDistributingPorts);
}

#if 0   /* Allenc */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Find_AggregatorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to find an appropiate port to aggregate
 *           with.
 * INPUT   : pPort -- pointer to a LACP_PORT_T to search.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_Find_AggregatorPort( LACP_PORT_T *pPort)
{
    /* First, we just consider the best aggregator is the original single */
    /* aggregator. Where pBest is the single aggregator map to this port. */
    LACP_PORT_T *pBest    = pPort;
    /* pNext is the next aggregator to check select                       */
    LACP_PORT_T *pNext = LACP_GetFirst_AggregatorPort();

    while(pNext)
    {
        /* Check for appropiate information for this port.                                        */
        /* The port which we want to aggregate together should have the following                 */
        /* informations are the same:                                                             */
        /* 1) Actor System Priority                                                               */
        /* 2) Actor System Identifier                                                             */
        /* 3) Actor Operational Key                                                               */
        /* 4) Partner System Priority                                                             */
        /* 5) Partner System Identifier                                                           */
        /* 6) Partner Operational Key                                                             */
        /* Besides these, if either the port or its link partner is an individual port,           */
        /* then it should always select itself, since we have already select itself in the        */
        /* first, now, we have to check itself or its link partner is aggregatable.               */
        /* If it is aggregatable then we can select this one, else we should always               */
        /* select a single aggregator.                                                            */
        /* 7) port's aggregate ability                                                            */
        /* 8) aggregate ability of selected port.                                                 */
        /* And remember to select a port that has lower priority and linear port number           */
        /* These 2 conditions shall be true at the same time                                      */
        /* 9) port's priority is better than or equal to best port                                */
        /* 10) linear port id is smaller than best port                                           */
        /* Finally, remember to check whether this is a looped port or not.                       */
        /* If yes, then we should not let the logic select this port.                             */
        /* 2001/11/22 ckhsu                                                                       */
        /* Now I have an idea, we only need to find out a port that can aggregate with this port. */
        /* That is enough information and did not disobey standard since we link it to the        */
        /* the logical aggregator that associate to P_TRUNK not just the same aggregator as       */
        /* standard. So, we don't need to check the port no and priority. But note, we should     */
        /* decide which port to attach at selection logic...                                      */
        if( (((pNext->AggActorPort).system_priority) == ((pPort->AggActorPort).system_priority)) &&
            (memcmp( ((pNext->AggActorPort).system_id), ((pPort->AggActorPort).system_id), 0x6) == 0) &&
            (((pNext->AggActorPort).key) == ((pPort->AggActorPort).key)) &&
            (((pNext->AggPartnerPort).system_priority) == ((pPort->AggPartnerPort).system_priority)) &&
            (memcmp( &((pNext->AggPartnerPort).system_id), &((pPort->AggPartnerPort).system_id), 0x6) == 0) &&
            (((pNext->AggPartnerPort).key) == ((pPort->AggPartnerPort).key)) &&
            /* Because we only want to aggregate with the same speed. */
            (pNext->PortSpeed == pPort->PortSpeed) &&
            /*(((((pNext->AggActorPort).state).lacp_port_state).Aggregation) && ((((pNext->AggPartnerPort).state).lacp_port_state).Aggregation)) &&*/
            /*(((((pPort->AggActorPort).state).lacp_port_state).Aggregation) && ((((pPort->AggPartnerPort).state).lacp_port_state).Aggregation)) &&*/
               (   LACP_TYPE_GET_PORT_STATE_BIT(((pNext->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
                && LACP_TYPE_GET_PORT_STATE_BIT(((pNext->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
               )
            && (   LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
                && LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
               )
            &&
            ( ((pNext->AggActorPort).port_priority < (pBest->AggActorPort).port_priority) ||
              ( ((pNext->AggActorPort).port_priority == (pBest->AggActorPort).port_priority) &&
                (LACP_LINEAR_ID(pNext->slot,pNext->port) < LACP_LINEAR_ID(pBest->slot,pBest->port)) ) )  &&
            (!LACP_Check_Looped( pNext, pPort)))
        {
            /* This port is the port we will aggregate together. */
            /* Then we have to do our chip limitation checking.  */
            if( (pNext->LACP_OperStatus == TRUE) &&
                (pNext->Port_OperStatus == TRUE) )
            {
                /* Okay, now one funny problem:                                       */
                /* If we let the user to change Default parameters, what will happen? */
                /* Will we find a wrong value?                                        */
                /* return pNext; */
                pBest = pNext;
            }

        }

        pNext = LACP_GetNext_AggregatorPort( pNext);
    }

    return (LACP_PORT_T *)pBest;
}
#endif  /* Allenc */

/* Allenc */
/* ===================================================================== */
/* ===================================================================== */
/* ===================================================================== */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_AllocateAggregator
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns a pointer of available aggregator, or
 *            NULL if no aggregator is available.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : pointer to the aggregator
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
LACP_AGGREGATOR_T*  LACP_UTIL_AllocateAggregator(LACP_PORT_T *port_ptr)
{
    LACP_PORT_T         *lacp_primary_port;
    LACP_AGGREGATOR_T   *allocated_aggregator;
    LACP_AGGREGATOR_T   *aggregator_ptr;
    LACP_SYSTEM_T       *lacp_system_ptr;
    UI32_T              agg_index;
    UI32_T              count = 0;

    allocated_aggregator    = NULL;
    lacp_system_ptr = LACP_OM_GetSystemPtr();
    if (LACP_UTIL_PortIsAggregatable(port_ptr) )
    {
        /* Phase I: Select an aggregator which has the same attribute as port_ptr */
        for (agg_index = 0; agg_index < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; agg_index++)
        {
            BOOL_T  static_trunk_used   = FALSE;
            BOOL_T  is_static           = FALSE;
            UI8_T   trunk_member_count  = 0;
            static_trunk_used   =   (TRK_PMGR_IsTrunkExist((agg_index+1), &is_static)
                                  &&is_static);
            trunk_member_count  = TRK_PMGR_GetTrunkMemberCounts(agg_index+1);
            aggregator_ptr      = &(lacp_system_ptr->aggregator[agg_index]);
            lacp_primary_port   = aggregator_ptr->pLACP_Ports;
            if (    (!static_trunk_used)
                ||  (trunk_member_count == 0)
               )
            {
                if (    (   (lacp_primary_port != NULL)
                         && LACP_UTIL_IsTheSameAttribute(aggregator_ptr, port_ptr)
                        )
                     || (   (lacp_primary_port == NULL)
                         && (   (aggregator_ptr->AggActorOperKey)
                             == (port_ptr->AggActorPort).key
                            )
                        )
                   )
                {
                    count++;
                    if (1 == count)
                    {
                        allocated_aggregator    = aggregator_ptr;
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == FALSE)
                        break;
                    }
#else
                    }
                    if (aggregator_ptr->static_admin_key == TRUE)
                    {
                        allocated_aggregator = aggregator_ptr;
                        break;
                    }
#endif
                }
            }
        } /* End of for (_Phase_I_) */

        /* Phase II: Select an aggregator which has the NULL attribute */
        for (agg_index = 0; ((allocated_aggregator == NULL) && (agg_index < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)); agg_index++)
        {
            BOOL_T  static_trunk_used   = FALSE;
            BOOL_T  is_static           = FALSE;
            UI8_T   trunk_member_count  = 0;
            static_trunk_used   =   (TRK_PMGR_IsTrunkExist((agg_index+1), &is_static)
                                  &&is_static);
            trunk_member_count  = TRK_PMGR_GetTrunkMemberCounts(agg_index+1);
            aggregator_ptr      = &(lacp_system_ptr->aggregator[agg_index]);
            lacp_primary_port   = aggregator_ptr->pLACP_Ports;
            if (    (   (!static_trunk_used)
                     || (trunk_member_count == 0)
                    )
                 && (lacp_primary_port == (LACP_PORT_T*)NULL)
                 && (   (aggregator_ptr->AggActorOperKey)
                     == SYS_DFLT_LACP_KEY_NULL
                    )
               )
            {
                /* Record the port as the lacp_primary_port of the aggregator */
                /*
                aggregator_ptr->lacp_primary_port   = port_ptr;
                */
                allocated_aggregator                = aggregator_ptr;
            }
        } /* End of for (_Phase_II_) */
    } /* End of if (_port_is_aggregatable_) */

    return allocated_aggregator;
} /* End of LACP_UTIL_AllocateAggregator */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SortAggregatorMembers
 * ------------------------------------------------------------------------
 * FUNCTION : This function will sort the members attached to the specified
 *            aggregator
 * INPUT    : agg_ptr   -- aggregator
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : prev_port this_port               test_port
 *                  |   |                       |
 *                  V   V                       V
 *          O-->O-->O-->@-->O-->O--> ....... -->O-->#-->O--> ... -->O-->|//|
 *            @ is compared with #. If # is prior to @ then # is moved to be
 *            the predecessor.
 * ------------------------------------------------------------------------
 */
void    LACP_UTIL_SortAggregatorMembers(LACP_AGGREGATOR_T *agg_ptr)
{
    BOOL_T      actor_key_major;
    LACP_PORT_T *prev_port;
    LACP_PORT_T *this_port;
    LACP_PORT_T *test_port;
    LACP_PORT_T *selected;
    UI32_T      key_size;
    UI32_T      member_sequence;
    int         cmp_result_1, cmp_result_2;

    if (agg_ptr)
    {
        /* Determine the sorting key */
        /* Compare {system_priority, system_id} */
        if (    (agg_ptr->AggActorSystemPriority > agg_ptr->AggPartnerSystemPriority)
             || (   (agg_ptr->AggActorSystemPriority == agg_ptr->AggPartnerSystemPriority)
                 && (   memcmp(agg_ptr->AggActorSystemID, agg_ptr->AggPartnerSystemID, 6)
                      > 0
                    )
                )
           )
        {
            actor_key_major = FALSE;
        }
        else
        {
            actor_key_major = TRUE;
        }
        key_size            = sizeof(SYSTEM_PRIORITY_T)+6+sizeof(LACP_KEY_T)+sizeof(PORT_PRIORITY_T)+sizeof(PPORT_T);
        this_port           = agg_ptr->pLACP_Ports;
        prev_port           = NULL;
        test_port           = NULL;
        member_sequence     = 0;
        if (this_port)
        {
            member_sequence++;
            while (this_port->pNext)
            {
                test_port   = this_port;
                while (test_port->pNext)
                {
                    /* compare the keys of this_port and test_port->pNext */
                    LACP_TYPE_CMP_LAC_INFO( cmp_result_1, this_port->AggActorPort, (test_port->pNext)->AggActorPort );
                    LACP_TYPE_CMP_LAC_INFO( cmp_result_2, this_port->AggPartnerPort, (test_port->pNext)->AggPartnerPort );
                    if (    (   (actor_key_major)
                             /*&& (memcmp(&(this_port->AggActorPort),
                                        &((test_port->pNext)->AggActorPort),
                                        key_size
                                       )>0
                                )*/
                             && (cmp_result_1 > 0)
                            )
                         || (   (!actor_key_major)
                             /*&& (memcmp(&(this_port->AggPartnerPort),
                                        &((test_port->pNext)->AggPartnerPort),
                                        key_size
                                       )>0
                                )*/
                             && (cmp_result_2 > 0)
                            )
                       )
                    {
                        selected                    = test_port->pNext;
                        test_port->pNext            = selected->pNext;
                        if (prev_port)
                        {
                            selected->pNext         = this_port;
                            prev_port->pNext        = selected;
                        }
                        else
                        {
                            selected->pNext         = this_port;
                            agg_ptr->pLACP_Ports    = selected;

                        }
                        this_port   = selected;
                    } /* End of if (_rearrange_sequence_) */
                    if (test_port->pNext)
                    {
                        test_port   = test_port->pNext;
                    }
                } /* End of while (test_port->pNext) */
                this_port->port_index   = member_sequence;

                prev_port   = this_port;
                this_port   = this_port->pNext;
                member_sequence++;
            } /* End of while (this_port->pNext) */
            this_port->port_index   = member_sequence;
        } /* End of if (prev_port) */


    } /* End of if (agg_ptr) */

    return;
} /* End of LACP_UTIL_SortAggregatorMembers */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_GetAggregator
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns TRUE if the aggregator of the specified
 *            port is existing, or FALSE if not available.
 * INPUT    : port_ptr  -- port
 * OUTPUT   : agg_ptr   -- aggregator
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_GetAggregator(LACP_PORT_T *port_ptr, LACP_AGGREGATOR_T *agg_ptr)
{
    agg_ptr = port_ptr->pAggregator;
    return (agg_ptr != NULL);
} /* End of LACP_UTIL_GetAggregator */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_RefreshOperStatusAndAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  : Refresh the operation status and the LACP admin key for the
 *            specified port.
 * INPUT    : port_ptr  -- port pointer
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    LACP_UTIL_RefreshOperStatusAndAdminKey(LACP_PORT_T *port_ptr)
{
    UI32_T      trunk_id;
    BOOL_T      is_static_trunk_member  = FALSE;

    if  (   (SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_UserPortToTrunkPort( port_ptr->slot, port_ptr->port, &trunk_id) )
         && (FALSE == TRK_PMGR_IsDynamicTrunkId( trunk_id))
        )
    {
        is_static_trunk_member  = TRUE;
    }

    /* Port OperStatus: Link Up/Down && Port Enabled/Disabled */
    if (    (port_ptr->LinkUp)
        &&  (port_ptr->PortEnabled)
       )
    {
        port_ptr->Port_OperStatus   = TRUE;
    }
    else
    {
        port_ptr->Port_OperStatus   = FALSE;
    } /* End of if (_link_up_ && _port_enabled_) */

    /* LACP OperStatus: LACP Enabled/Disabled && Full/Half Duplex */
    if (    (port_ptr->LACPEnabled)
        &&  (port_ptr->FullDuplexMode)
        &&  (!is_static_trunk_member)
       )
    {
        port_ptr->LACP_OperStatus   = TRUE;
    }
    else
    {
        port_ptr->LACP_OperStatus   = FALSE;
    } /* End of if (_lacp_enabled_ && _full_duplex_mode_) */

    /* LACP Default Admin Key */
    if (port_ptr->static_admin_key)
    {
        if (bLACP_DbgMsg)
            BACKDOOR_MGR_Printf("\r\n (unit:%d, port:%d) -- static_admin_key is set", (int)(port_ptr->slot), (int)(port_ptr->port));
    }
    else
    {
        LACP_KEY_T  lacp_key    = SYS_DFLT_LACP_KEY_DEFAULT;
        switch (port_ptr->PortSpeed)
        {
            case 10:
                lacp_key    = SYS_DFLT_LACP_KEY_10FULL;
                break;
            case 100:
                lacp_key    = SYS_DFLT_LACP_KEY_100FULL;
                break;
            case 1000:
                lacp_key    = SYS_DFLT_LACP_KEY_1000FULL;
                break;
            case 10000:
                lacp_key    = SYS_DFLT_LACP_KEY_10GFULL;
                break;
            case 40000:
                lacp_key    = SYS_DFLT_LACP_KEY_40GFULL;
                break;
            case 100000:
                lacp_key    = SYS_DFLT_LACP_KEY_100GFULL;
                break;
#ifdef SYS_DFLT_LACP_KEY_25GFULL
            case 25000:
                lacp_key    = SYS_DFLT_LACP_KEY_25GFULL;
                break;
#endif
        } /* End of switch (_port_speed_) */
        (port_ptr->AggActorAdminPort).key   = lacp_key;
    } /* End of if (_static_admin_key_) */
    return;
} /* End of LACP_UTIL_RefreshOperStatusAndAdminKey */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_RefreshPortSpeedDuplex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to refresh the speed duplex mode for
 *           a port.
 * INPUT   : pPort -- pointer to the port we want to refresh.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void    LACP_UTIL_RefreshPortSpeedDuplex(LACP_PORT_T *port_ptr)
{
    Port_Info_T         port_info;
    UI32_T              ifindex;
    UI32_T              port_speed  = 10;
    BOOL_T              full_duplex = FALSE;

    if (    (SWCTRL_UserPortToIfindex(port_ptr->slot, port_ptr->port, &ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
        &&  (SWCTRL_GetPortInfo(ifindex, &port_info) )
       )
    {
        switch (port_info.speed_duplex_oper)
        {
            case VAL_portSpeedDpxStatus_fullDuplex10:
                full_duplex = TRUE;
            case VAL_portSpeedDpxStatus_halfDuplex10:
                port_speed  = 10;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex100:
                full_duplex = TRUE;
            case VAL_portSpeedDpxStatus_halfDuplex100:
                port_speed  = 100;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex1000:
                full_duplex = TRUE;
            case VAL_portSpeedDpxStatus_halfDuplex1000:
                port_speed  = 1000;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex10g:
                full_duplex = TRUE;
            case VAL_portSpeedDpxStatus_halfDuplex10g:
                port_speed  = 10000;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex40g:
                full_duplex = TRUE;
            case VAL_portSpeedDpxStatus_halfDuplex40g:
                port_speed  = 40000;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex100g:
                full_duplex = TRUE;
            case VAL_portSpeedDpxStatus_halfDuplex100g:
                port_speed  = 100000;
                break;
#ifdef VAL_portSpeedDpxStatus_fullDuplex25g
            case VAL_portSpeedDpxStatus_fullDuplex25g:
                full_duplex = TRUE;
                port_speed  = 25000;
                break;
#endif
            default:
                break;
        } /* End of switch */
        port_ptr->FullDuplexMode    = full_duplex;
        port_ptr->PortSpeed         = port_speed;
    } /* End of if */
    return;
} /* End of LACP_UTIL_RefreshPortSpeedDuplex */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_PortIsAggregatable
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns true if the specified port is aggregatable,
 *            else false is returned.
 * INPUT    : port_ptr  -- port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_UTIL_PortIsAggregatable(LACP_PORT_T *port_ptr)
{
    return
    (
    /* Check both the actor port and the partner port of these two ports are
     * all aggregatable.
     * aggregation: 1 bit of the port_state
     */
        (port_ptr->LACP_OperStatus)
    &&  (port_ptr->Port_OperStatus)
    &&  LACP_TYPE_GET_PORT_STATE_BIT(((port_ptr->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
    &&  LACP_TYPE_GET_PORT_STATE_BIT(((port_ptr->AggPartnerPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation)
    /*&&  ( ( ( (port_ptr->AggActorPort  ).state).lacp_port_state).Aggregation)*/
    /*&&  ( ( ( (port_ptr->AggPartnerPort).state).lacp_port_state).Aggregation)*/
    );

} /* End of LACP_UTIL_PortIsAggregatable */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_IsTheSameAttribute
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns true if the specified port has the same
 *            attribute as the specified aggregator, else false is returned.
 * INPUT    : dst_port  -- destination port
 *            src_port  -- source port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_UTIL_IsTheSameAttribute(LACP_AGGREGATOR_T *agg_ptr, LACP_PORT_T *port_ptr)
{
    return
    (
        (   (   agg_ptr->AggActorSystemPriority)
         == ( (port_ptr->AggActorPort).system_priority)
        )
    &&  (memcmp( (  agg_ptr->AggActorSystemID),
                 ((port_ptr->AggActorPort).system_id),
                 0x6
               ) == 0
        )
    &&  (   (   agg_ptr->AggActorOperKey)
         == ( (port_ptr->AggActorPort).key)
        )
    &&  (   (   agg_ptr->AggPartnerSystemPriority)
         == ( (port_ptr->AggPartnerPort).system_priority)
        )
    &&  (memcmp( &(  agg_ptr->AggPartnerSystemID),
                 &((port_ptr->AggPartnerPort).system_id),
                 0x6) == 0
        )
    &&  (   (   agg_ptr->AggPartnerOperKey)
         == ( (port_ptr->AggPartnerPort).key)
        )
    );
} /* End of LACP_UTIL_IsTheSameAttribute */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CopyTheAttributeFromPort
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns true if the specified port has the same
 *            attribute as the specified aggregator, else false is returned.
 * INPUT    : dst_port  -- destination port
 *            src_port  -- source port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  void    LACP_UTIL_CopyTheAttributeFromPort(LACP_AGGREGATOR_T *agg_ptr, LACP_PORT_T *port_ptr)
{
    agg_ptr->AggActorSystemPriority     = (port_ptr->AggActorPort).system_priority;

    memcpy( (  agg_ptr->AggActorSystemID),
            ((port_ptr->AggActorPort).system_id),
            0x6
          );

    agg_ptr->AggActorOperKey            = (port_ptr->AggActorPort).key;

    agg_ptr->AggPartnerSystemPriority   = (port_ptr->AggPartnerPort).system_priority;

    memcpy( &(  agg_ptr->AggPartnerSystemID),
            &((port_ptr->AggPartnerPort).system_id),
            0x6
          );

    agg_ptr->AggPartnerOperKey          = (port_ptr->AggPartnerPort).key;

    return;
} /* End of LACP_UTIL_CopyTheAttributeFromPort */

#if 0   /* Not used currently */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_PortsAreTheSameAttribute
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns true if the specified ports have the same
 *            attribute, else false is returned.
 * INPUT    : dst_port  -- destination port
 *            src_port  -- source port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_UTIL_PortsAreTheSameAttribute(LACP_PORT_T *dst_port, LACP_PORT_T *src_port)
{
    return
    (
        (   ( (dst_port->AggActorPort).system_priority)
         == ( (src_port->AggActorPort).system_priority)
        )
    &&  (memcmp( ((dst_port->AggActorPort).system_id),
                 ((src_port->AggActorPort).system_id),
                 0x6
               ) == 0
        )
    &&  (   ( (dst_port->AggActorPort).key)
         == ( (src_port->AggActorPort).key)
        )
    &&  (   ( (dst_port->AggPartnerPort).system_priority)
         == ( (src_port->AggPartnerPort).system_priority)
        )
    &&  (memcmp( &((dst_port->AggPartnerPort).system_id),
                 &((src_port->AggPartnerPort).system_id),
                 0x6) == 0
        )
    &&  (   ( (dst_port->AggPartnerPort).key)
         == ( (src_port->AggPartnerPort).key)
        )
    );
} /* End of LACP_UTIL_PortsAreTheSameAttribute */
#endif  /* Not used currently */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_GetAggMemberList
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a port list related to the
 *           aggregator.
 * INPUT   : agg_ptr    -- pointer to aggregator
 * OUTPUT  : port_list  -- port list bit array.
 * RETURN  : TRUE  -- agg_ptr is valid
 *           FALSE -- agg_ptr does not exist
 * NOTE    : None
 *------------------------------------------------------------------------*/
static  BOOL_T  LACP_UTIL_GetAggMemberList( LACP_AGGREGATOR_T *agg_ptr, UI8_T *port_list)
{
    LACP_PORT_T         *member_port;
    UI32_T              linear_num;
    UI32_T              octet;
    UI32_T              bit;

    memset( port_list, 0x0, LACP_PORT_LIST_OCTETS);

    if(agg_ptr)
    {
        /* Now find all ports in the aggregator. */
        member_port = agg_ptr->pLACP_Ports;

        while(member_port)
        {
            linear_num = LACP_LINEAR_ID( member_port->slot, member_port->port);

            octet = (linear_num-1)/8;
            bit   = (linear_num-1)%8;

            port_list[octet] |= (UI8_T)(1<<(7-bit));
            member_port = member_port->pNext;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* End of LACP_UTIL_GetAggMemberList */


/* ===================================================================== */
/* ===================================================================== */
/* ===================================================================== */
/* Allenc */



/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetFirst_AggregatorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get first aggregator port in system.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_GetFirst_AggregatorPort(void)
{
    /* This function will try to get the first aggregator in system. */
    UI32_T slot,port;
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
                continue;
            }

            return LACP_FIND_PORT( slot, port);
        }
    }

    return (LACP_PORT_T *)NULL;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetNext_AggregatorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get next aggregator port in system.
 * INPUT   : pPrev -- previous port
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_GetNext_AggregatorPort( LACP_PORT_T *pPrev)
{
    /* First we get aggregator id. */
    UI32_T  slot,port;

    if(!pPrev)
    {
        /* If pass in parameter is NULL, then we just return the first one of the aggregator. */
        return LACP_GetFirst_AggregatorPort();
    }

    slot = pPrev->slot;
    port = pPrev->port;
    port++;

    /* Here, the starting value has already assigned by last function call. */
    /* First, we try to find the aggregator exist in this slot.             */
    for( ; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
    {
        if(STKTPLG_POM_PortExist( slot, port) == FALSE)
        {
            /* We won't have the next port in this slot. */
            continue;
        }

        return LACP_FIND_PORT( slot, port);
    }

    /* If the previous loop did not find any aggregator, then it means      */
    /* in that stack, we have no more aggregator inside it. Then we have to */
    /* search from the next slot, so increment the slot, but port is always */
    /* starting from 1.                                                     */
    slot++;
    for( ; slot<= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; slot++)
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
                continue;
            }

            return LACP_FIND_PORT( slot, port);
        }
    }


    return (LACP_PORT_T *)NULL;
}

#if 0   /* Not used currently */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Check_Looped
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to check loop in ports of LAG.
 * INPUT   : pA    -- select aggregator port
 *           pPort -- port want to check
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T LACP_Check_Looped( LACP_PORT_T *pA, LACP_PORT_T *pPort)
{
    if( (((pA->AggActorPort).system_priority) == ((pPort->AggPartnerPort).system_priority)) &&
        (memcmp( ((pA->AggActorPort).system_id), ((pPort->AggPartnerPort).system_id), 0x6) == 0) &&
        (((pA->AggActorPort).key) == ((pPort->AggPartnerPort).key)) &&
        ( ((pA->AggActorPort).port_priority < (pPort->AggActorPort).port_priority) ||
          ( ((pA->AggActorPort).port_priority == (pPort->AggActorPort).port_priority) &&
            (LACP_LINEAR_ID(pA->slot,pA->port) < LACP_LINEAR_ID(pPort->slot,pPort->port)) ) ) )
    {
        return TRUE;
    }

    return FALSE;
}
#endif  /* Not used currently */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message in system.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Message( BOOL_T enable)
{
    if(enable)
    {
        bLACP_DbgMsg = TRUE;
    }
    else
    {
        bLACP_DbgMsg = FALSE;
    }

    BACKDOOR_MGR_Printf("\nLACP detailed Debug message %s be displayed on screen...", (bLACP_DbgMsg)?("will"):("won't"));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_TRK_MGR_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message in debug calling
 *           trunk system.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_TRK_MGR_Message( BOOL_T enable)
{
    if(enable)
    {
        bLACP_DbgAthAL = TRUE;
    }
    else
    {
        bLACP_DbgAthAL = FALSE;
    }

    BACKDOOR_MGR_Printf("\nLACP detailed TRK_MGR message %s be displayed on screen...", (bLACP_DbgAthAL)?("will"):("won't"));
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Trace_Route_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message to trace the
 *           calling route.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Trace_Route_Message( BOOL_T enable)
{
    if(enable)
    {
        bLACP_DbgTrace = TRUE;
    }
    else
    {
        bLACP_DbgTrace = FALSE;
    }

    BACKDOOR_MGR_Printf("\nLACP Trace %s be displayed on screen...", (bLACP_DbgAttach)?("will"):("won't"));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Try_To_Attach_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug message in debug calling
 *           Try_To_Attach.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Try_To_Attach_Message( BOOL_T enable)
{
    if(enable)
    {
        bLACP_DbgAttach = TRUE;
    }
    else
    {
        bLACP_DbgAttach = FALSE;
    }

    BACKDOOR_MGR_Printf("\nLACP Try_To_Attach %s be displayed on screen...", (bLACP_DbgAttach)?("will"):("won't"));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Enable_Debug_Pdu
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to enable debug Pdu message in system.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Enable_Debug_Pdu( BOOL_T enable)
{
    if(enable)
    {
        bLACP_DbgPdu = TRUE;
    }
    else
    {
        bLACP_DbgPdu = FALSE;
    }

    BACKDOOR_MGR_Printf("\nReceived LACPDU packets %s be displayed on screen...", (bLACP_DbgPdu)?("will"):("won't"));
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Should_Select_Same_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to check ports should select the same
 *           aggregator.
 * INPUT   : enable -- TRUE/FALSE for Enable/Disable.
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Should_Select_Same_Aggregator( LACP_PORT_T *pA, LACP_PORT_T *pB)
{
    if( (((pA->AggActorPort).system_priority) == ((pB->AggActorPort).system_priority)) &&
        (memcmp( ((pA->AggActorPort).system_id), ((pB->AggActorPort).system_id), 0x6) == 0) &&
        (((pA->AggActorPort).key) == ((pB->AggActorPort).key)) &&
        (((pA->AggPartnerPort).system_priority) == ((pB->AggPartnerPort).system_priority)) &&
        (memcmp( &((pA->AggPartnerPort).system_id), &((pB->AggPartnerPort).system_id), 0x6) == 0) &&
        (((pA->AggPartnerPort).key) == ((pB->AggPartnerPort).key)) &&
        /* We also want to check the speed. */
        (pA->PortSpeed == pB->PortSpeed) )
    {
        if( (pB->LACP_OperStatus) &&
            (pB->Port_OperStatus) )
        {
            return TRUE;
        }
    }


    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Add_To_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to add port to an aggregator
 *           aggregator.
 * INPUT   : pAgg  -- Aggregator to add port
 *           pPort -- port to be removed
 * OUTPUT  : None
 * RETURN  : Index of port sequence number in aggregator, or zero if an
 *           error occurs.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Add_To_Aggregator( LACP_AGGREGATOR *pAgg, LACP_PORT_T *pPort)
{
    /* In this function, we will try to add a port to this aggregator.  */
    /* We want to insert the port and sorting by its system and port    */
    /* priority etc..                                                   */
    LACP_PORT_T *pPrev    = NULL;
    LACP_PORT_T *pCurrent = NULL;

    BOOL_T      bActorHasHighPriority;
    UI32_T      nCompareSize;
    UI32_T      nPortSequenceIndex = 1;
    int         cmp_result;

    if( (!pAgg) ||
        (!pPort) )
    {
        /* Exception. */
        BACKDOOR_MGR_Printf("\nFile: %s Line: %d Add_To_Aggregator has Exception...", __FILE__, __LINE__);
        return 0;
    }

    nCompareSize = sizeof(SYSTEM_PRIORITY_T)+6;

    /* Since this port want to add to this system, we have to know which system */
    /* has higher priority. We compare the system priority and MAC(6 bytes).    */
    LACP_TYPE_CMP_LAC_SYSTEM( cmp_result, pPort->AggActorPort, pPort->AggPartnerPort );
    /*if( memcmp( &(pPort->AggActorPort), &(pPort->AggPartnerPort), nCompareSize) >= 0 )*/
    if ( cmp_result >= 0 )
    {
        bActorHasHighPriority = FALSE;
    }
    else
    {
        bActorHasHighPriority = TRUE;
    }

    /* Now the comapre size will change to include port priority and port number. */
    nCompareSize = sizeof(SYSTEM_PRIORITY_T)+6+sizeof(LACP_KEY_T)+sizeof(PORT_PRIORITY_T)+sizeof(PPORT_T);

    /* Consider the aggregator always has ports attached to it. */
    /* pCurrent = pAgg->pLACP_Ports[0]; */
    pCurrent = pAgg->pLACP_Ports;

    if(!pCurrent)
    {
        /* No ports in this Aggregator. */
        /* pAgg->pLACP_Ports[0] = pPort; */
        pAgg->pLACP_Ports = pPort;
        pPort->pNext = NULL;

        pPort->pAggregator = pAgg;

        /* This is a NULL aggregator: shall inherit the attribute from the port */
        LACP_UTIL_CopyTheAttributeFromPort(pAgg, pPort);

        return 1;
    }
    else
    {
        while(pCurrent)
        {
            if(pCurrent == pPort)
            {
                /* Tracing bug, normally it should not happen. */
                BACKDOOR_MGR_Printf("\nList Looping in Aggregator %3d of Slot:%2d Port:%2d", (int)pAgg->AggID, (int)pPort->slot, (int)pPort->port);
                pPort->pAggregator = pAgg;
                return nPortSequenceIndex;
            }

            if(bActorHasHighPriority)
            {
                /* Since Actor has higher priority, we sort by Actor.   */
                LACP_TYPE_CMP_LAC_INFO( cmp_result, pPort->AggActorPort, pCurrent->AggActorPort );
                /*if(memcmp( &(pPort->AggActorPort), &(pCurrent->AggActorPort), nCompareSize) >= 0)*/
                if ( cmp_result >= 0 )
                {
                    pPrev    = pCurrent;
                    pCurrent = pCurrent->pNext;
                }
                else
                {
                    break;
                }
            }
            else
            {
                /* Since Partner has higher priority, we sort by Partner. */
                LACP_TYPE_CMP_LAC_INFO( cmp_result, pPort->AggPartnerPort, pCurrent->AggPartnerPort );
                /*if(memcmp( &(pPort->AggPartnerPort), &(pCurrent->AggPartnerPort), nCompareSize) >= 0)*/
                if ( cmp_result >= 0 )
                {
                    pPrev    = pCurrent;
                    pCurrent = pCurrent->pNext;
                }
                else
                {
                    break;
                }
            }
            nPortSequenceIndex++;
        }

        /* When it break or end of compare, it means that pPrev point to the last one who */
        /* is smaller than this value.                                                    */
        if(pPrev)
        {
            pPort->pNext = pPrev->pNext;
            pPrev->pNext = pPort;
        }
        else
        {
            pPort->pNext = pCurrent;
            /* pAgg->pLACP_Ports[0] = pPort; */
            pAgg->pLACP_Ports = pPort;
        }

    }

    pPort->pAggregator = pAgg;

    return nPortSequenceIndex;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Remove_From_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to remove port to an aggregator
 *           aggregator.
 * INPUT   : pAgg  -- Aggregator to remove port
 *           pPort -- port to be removed
 * OUTPUT  : None
 * RETURN  : Index of port sequence number in aggregator.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Remove_From_Aggregator( LACP_AGGREGATOR *pAgg, LACP_PORT_T *pPort)
{
    LACP_PORT_T *pPrev    = NULL;
    LACP_PORT_T *pCurrent = NULL;
    LACP_SYSTEM_T*  lacp_system_ptr;

    UI32_T      nPortSequenceIndex = 1;

    if( (!pAgg) ||
        (!pPort) )
    {
        /* Exception. */
        BACKDOOR_MGR_Printf("\nFile: %s Line: %d Remove_From_Aggregator has Exception...", __FILE__, __LINE__);
        return 0;
    }

    lacp_system_ptr = LACP_OM_GetSystemPtr();

    /* pCurrent = pAgg->pLACP_Ports[0]; */
    pCurrent = pAgg->pLACP_Ports;

    if(pPort == pCurrent)
    {
        /* pAgg->pLACP_Ports[0] = pPort->pNext; */
        pAgg->pLACP_Ports = pPort->pNext;
        if (pAgg->pLACP_Ports == NULL)
        {
            pAgg->AggActorSystemPriority    = lacp_system_ptr->SystemPriority;
            pAgg->AggPartnerSystemPriority  = 0;
            pAgg->AggPartnerOperKey         = SYS_DFLT_LACP_KEY_NULL;
            /*
            memset( (  pAgg->AggActorSystemID), 0, 0x6);
            */
            memset( &(  pAgg->AggPartnerSystemID), 0, 0x6);
            if (!pAgg->static_admin_key)
            {
                pAgg->AggActorOperKey   = pAgg->AggActorAdminKey
                                        = SYS_DFLT_LACP_KEY_NULL;
            }
        }

        pPort->pAggregator = NULL;
        pPort->pNext       = NULL;

        return 1;
    }
    else
    {
        while(pCurrent)
        {
            if(pCurrent == pPort)
            {
                if(pPrev)
                {
                    pPrev->pNext = pPort->pNext;
                }
                else
                {
                    /* pAgg->pLACP_Ports[0] = pPort->pNext; */
                    pAgg->pLACP_Ports = pPort->pNext;
                }

                if (pAgg->pLACP_Ports == NULL)
                {
                    pAgg->AggActorSystemPriority    = lacp_system_ptr->SystemPriority;
                    pAgg->AggPartnerSystemPriority  = 0;
                    pAgg->AggPartnerOperKey         = SYS_DFLT_LACP_KEY_NULL;
                    /*
                    memset( (  pAgg->AggActorSystemID), 0, 0x6);
                    */
                    memset( &(  pAgg->AggPartnerSystemID), 0, 0x6);
                    if (!pAgg->static_admin_key)
                    {
                        pAgg->AggActorOperKey   = pAgg->AggActorAdminKey
                                                = SYS_DFLT_LACP_KEY_NULL;
                    }
                }

                pPort->pAggregator = NULL;
                pPort->pNext       = NULL;

                return nPortSequenceIndex;
            }
            else
            {
                pPrev    = pCurrent;
                pCurrent = pCurrent->pNext;
            }
            nPortSequenceIndex++;
        }
    }

    BACKDOOR_MGR_Printf("\nFile: %s Line: %d Rmv_Frm_Aggtr can not find port in this Aggtr...", __FILE__, __LINE__);

    return 0;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_First_Port_In_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get first port in aggregator
 * INPUT   : pPort -- port to get
 * OUTPUT  : None
 * RETURN  : First port in the aggregator pointed by pass in port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_Get_First_Port_In_Aggregator( LACP_PORT_T *pPort)
{
    LACP_PORT_T       *p;
    LACP_AGGREGATOR_T *pAgg;

    if(!pPort)
    {
        return NULL;
    }

    pAgg = pPort->pAggregator;

    if(!pAgg)
    {
        return NULL;
    }

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nGet First Port In Agg ID: %d",(int)pAgg->AggID);
    }

    /* p = pAgg->pLACP_Ports[0]; */
    p = pAgg->pLACP_Ports;

    return p;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Next_Port_In_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get next port in aggregator
 * INPUT   : pPort -- port to get
 * OUTPUT  : None
 * RETURN  : Next port in the aggregator pointed by pass in port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
LACP_PORT_T * LACP_Get_Next_Port_In_Aggregator( LACP_PORT_T *pPort)
{
    /* The pass in parameter is the port which we want to search for. */
    /* LACP_PORT_T       *p; */
    LACP_AGGREGATOR_T *pAgg;

    if(!pPort)
    {
        return NULL;
    }

    pAgg = pPort->pAggregator;

    if(!pAgg)
    {
        return NULL;
    }
    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nGet Next Port In Agg ID: %d",(int)pAgg->AggID);
    }

    return pPort->pNext;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Port_Index_In_Aggregator
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get next port in aggregator
 * INPUT   : pPort -- port to get
 * OUTPUT  : None
 * RETURN  : 0     -- Not found.
 *           other -- index of port in aggregator
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_Get_Port_Index_In_Aggregator( LACP_PORT_T *pPort)
{
    LACP_PORT_T       *p;
    /* LACP_AGGREGATOR_T *pAgg; */

    UI32_T             idx = 1;

    p = LACP_Get_First_Port_In_Aggregator( pPort);

    while(p)
    {
        if(p == pPort)
        {
            return idx;
        }
        else
        {
            p = LACP_Get_Next_Port_In_Aggregator( p);
            idx++;
        }
    }

    return 0;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Recalculate_CollectingDistributing_Ports
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to recalculate the ports in Collecting/
 *           Distributing state of the aggregator which this port belong
 *           to.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Recalculate_CollectingDistributing_Ports( LACP_PORT_T *pPort)
{
    LACP_PORT_T       *p;
    LACP_AGGREGATOR_T *pAgg;

    UI32_T  num_of_ports;

    if(!pPort)
    {
        return;
    }

    pAgg = pPort->pAggregator;

    if(!pAgg)
    {
        return;
    }

    num_of_ports = 0;

    p = pAgg->pLACP_Ports;

    while(p)
    {
        if(TRUE == LACP_Is_CollectingDistributing( p))
        {
            num_of_ports++;
        }

        p = p->pNext;
    }

    pAgg->NumOfCollectingDistributingPorts = num_of_ports;

    /* Then we get the ports of sync */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_CollectingDistributing
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to check port is in Collecting/
 *           Distributing state or not.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_CollectingDistributing( LACP_PORT_T *pPort)
{
    LACP_PORT_STATE_T *pPS;

    BOOL_T  bActorSync,bPartnerSync;

    pPS = &((pPort->AggPartnerPort).state);

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
        /*&& ((pPS->lacp_port_state).Synchronization) )*/
      )
    {
        bPartnerSync = TRUE;
    }
    else
    {
        bPartnerSync = FALSE;
    }

    if(bActorSync && bPartnerSync)
    {
        return TRUE;
    }

    return FALSE;
}

/* --------------------------------------------------------------------------- */
/* The codes below are for the compatible of old Foxfire platform and Mercury. */
/* --------------------------------------------------------------------------- */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Trunk_Member_Set
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to be compatible with Foxfire/MC2 to
 *           know whether this port is a trunk member port or not.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Is a trunk port member.
 *           FALSE -- Is not a trunk port member.
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Trunk_Member_Set( UI32_T slot, UI32_T port, UI32_T *pTrkId)
{
    /* UI32_T ifIndex; */

    /* SWCTRL_UserPortToLogicalPort( slot, port, &ifIndex); */

    if(SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_UserPortToTrunkPort( slot, port, pTrkId))
    {
        return TRUE;
    }

    return FALSE;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TX_SendPDU
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to send packet out from port.
 * INPUT   : mref -- LReference to data structure.
 *           unit -- unit number.
 *           port -- port number.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_TX_SendPDU( L_MM_Mref_Handle_T *mref_handle_p, UI32_T unit, UI32_T port)
{
    UI8_T   my_mac_addr[6];
    UI8_T   bridge_group_addr[6];
    UI16_T  taginfo = 0x2000;   /* { priority(3 bits)-CFI(1 bit)-vlan_id(12 bits) } */
    VLAN_OM_Dot1qPortVlanEntry_T    vlan_port_entry;
    UI32_T  ifindex;
    UI32_T  pvid;

    if(bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP_TX_SendPDU slot:%d port:%d", (int)unit, (int)port);
    }

    DEBUG_PACKET("%stx LACPDU to unit %lu port %lu", DEBUG_HEADER(), (unsigned long)unit, (unsigned long)port);
    {
        LACP_PDU_U *pdu;
        UI32_T     pdu_len;

        pdu = (LACP_PDU_U*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (pdu == NULL)
        {
            return;
        }

        SPRINT_MEMORY(DEBUG_PageBuffer, pdu, pdu_len);
        DEBUG_PACKET_DETAIL("\r\n%s", DEBUG_PageBuffer);
    }

    if (!SWCTRL_GetPortMac(LACP_LINEAR_ID(unit, port), my_mac_addr))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* Need to know the pvid of this port */
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pvid    = 1;
    }
    else
    {
        VLAN_OM_GetDot1qPortVlanEntry(ifindex, &vlan_port_entry);
        pvid    = vlan_port_entry.dot1q_pvid_index;
    }
    taginfo |= (UI16_T)pvid;

    LACP_OM_GetSlowProtocolMCAddr(bridge_group_addr);
    mref_handle_p->next_usr_id = SYS_MODULE_LAN;

    LAN_SendPacket( mref_handle_p,               /* Reference to memory */
                    bridge_group_addr,
                    my_mac_addr,
                    LACPDU_Slow_Protocols_Type,
                    taginfo,
                    sizeof(LACP_PDU_U),
                    unit,
                    port,
                    0,  /* untagged */
                    3); /* highest priority */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Available_TrunkId
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get a available trunk id, it maybe
 *           either used by the aggregator or an empty id.
 * INPUT   : mref -- LReference to data structure.
 *           slot -- slot number.
 *           port -- port number.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
#if 0   /* Allenc: Redesigned */
UI32_T LACP_Get_Available_TrunkId( UI32_T slot, UI32_T port)
{
    /* ckhsu 2001/12/12 comment for Mercury.                                */
    /* Here the code is totally different in Foxfire platform and Mercury.  */
    /* In Foxfire platform, the similar code should return the trunk that   */
    /* in that chip, but since in broadcom it can contain several trunk in  */
    /* a single chip, so we have to search a ports reside the aggregator.   */
    /* Once we found any port is a trunk port, then the trunk id is what we */
    /* want here, if no ports are trunk ports, then we have to allocate or  */
    /* get a trunk id. According to ArthurWu's word, once we allocate a     */
    /* trunk id, then it can not be used by any other type of trunk, so I   */
    /* think here we only try to check the trunk id and whether we can or   */
    /* can not add to it. when Collecting/Distributing Enabled, then we try */
    /* to find a true trunk and then add it at that time.                   */
    LACP_PORT_T *pPort,
                *memberPort,
                *pNext;

    TRK_MGR_TrunkEntry_T trunk_entry;

    UI32_T       al_id, port_num;
    /*
    UI32_T       num_of_trunks = 0;
    */
    SWCTRL_Lport_Type_T     port_type;
    UI32_T       member_unit,member_port, trunk_id;

    /* BODY */
    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nLACP_Get_Available_TrunkId");
    }
    if(STKTPLG_POM_PortExist( slot, port) == FALSE)
    {
        return 0;
    }

    member_unit = member_port = trunk_id = 0;
    memset(&trunk_entry, 0 , sizeof(TRK_MGR_TrunkEntry_T));
    pPort = LACP_FIND_PORT( slot, port);

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nFind Port Unit %d, Slot %d", (int)pPort->slot, (int)pPort->port);
    }


    /* 9-27-2002
        This section is to prevent LACP to form a trunk for single link
        when there looping exist. (port 1 connects to port 10 )
     */

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nActor system priority %d",(pPort->AggActorPort).system_priority);
        BACKDOOR_MGR_Printf("\nActor system priority %d",(pPort->AggPartnerPort).system_priority);

        BACKDOOR_MGR_Printf("\n\tActor SyetmId: %02x-%02x-%02x-%02x-%02x-%02x",   (int)(pPort->AggActorPort).system_id[0],
                                                                     (int)(pPort->AggActorPort).system_id[1],
                                                                     (int)(pPort->AggActorPort).system_id[2],
                                                                     (int)(pPort->AggActorPort).system_id[3],
                                                                     (int)(pPort->AggActorPort).system_id[4],
                                                                     (int)(pPort->AggActorPort).system_id[5]);

        BACKDOOR_MGR_Printf("\n\nPartner SyetmId: %02x-%02x-%02x-%02x-%02x-%02x", (int)(pPort->AggPartnerPort).system_id[0],
                                                                     (int)(pPort->AggPartnerPort).system_id[1],
                                                                     (int)(pPort->AggPartnerPort).system_id[2],
                                                                     (int)(pPort->AggPartnerPort).system_id[3],
                                                                     (int)(pPort->AggPartnerPort).system_id[4],
                                                                     (int)(pPort->AggPartnerPort).system_id[5]);
    }

    if (((pPort->AggActorPort).system_priority == (pPort->AggPartnerPort).system_priority) &&
        (0 == memcmp((pPort->AggActorPort).system_id,(pPort->AggPartnerPort).system_id, 0x6)))
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\nERROR!!  This link is a Loop");
        }

        return 0;
    }
    /* End of Modification
     */

    pNext = LACP_Get_First_Port_In_Aggregator(pPort);

    if(bLACP_DbgTrace)
    {
        if (!pNext)
            BACKDOOR_MGR_Printf("\npNext is empty");
    }

    while(pNext)
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\nPort in agg Unit %d, Slot %d", (int)pNext->slot, (int)pNext->port);
        }

        if(TRUE == LACP_Is_Trunk_Member_Set( pNext->slot, pNext->port, &al_id))
        {
            if(bLACP_DbgTrace)
            {
                BACKDOOR_MGR_Printf("\nIs Trunk Member pNext Port Unit %d, Slot %d", (int)pNext->slot, (int)pNext->port);
                BACKDOOR_MGR_Printf("\nReturn trunk_id from lacp %d", (int)al_id);
            }
            return al_id;
        }

        pNext = LACP_Get_Next_Port_In_Aggregator( pNext);
    }

    /* If here, it means we have to find whether we can have a empty trunk id or not. */
    /* First we get how many trunks is being used. */
    /* Modified by Allen: for a Null Trunk, we have to check the number of the trunk member.
       Though a null trunk is existing, it is still available.
    while(TRUE == TRK_PMGR_GetNextTrunkEntry( &trunk_entry))
    {
        num_of_trunks++;
    }
    */

    /* [amytu: 6-24-2002] Add this for-loop to search for an existing dynamic trunk id.

     */
    for(al_id = 1; al_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; al_id++)
    {
        trunk_entry.trunk_index = al_id;

        if (TRUE == TRK_PMGR_IsDynamicTrunkId(al_id))
        {
            if(bLACP_DbgTrace)
            {
                BACKDOOR_MGR_Printf("\nFind Dynamic trunk_id %d", (int)al_id);
            }

            if(TRUE == TRK_PMGR_GetTrunkEntry( &trunk_entry))
            {
                for (port_num=1; port_num <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST * 8); port_num++)
                {
                    if (trunk_entry.trunk_ports[((port_num - 1) / 8)] & ((0x01) << ( 7 -((port_num - 1) % 8))))
                    {
                        if ((port_type = SWCTRL_LogicalPortToUserPort(port_num, &member_unit, &member_port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                        {
                            if(bLACP_DbgTrace)
                            {
                                BACKDOOR_MGR_Printf("\nFind port: unit %d port %d in trunk_id %d", (int)member_unit, (int)member_port, (int)trunk_id);
                            }

                            memberPort = LACP_FIND_PORT( member_unit, member_port);
                            if (!memberPort)
                            {
                                if(bLACP_DbgTrace)
                                {
                                    BACKDOOR_MGR_Printf("\nMember port is empty");
                                }
                                continue;
                            }
                            if(bLACP_DbgTrace)
                            {
                                BACKDOOR_MGR_Printf("\nPrint LAG ID in Get Available trunk id()");
                                BACKDOOR_MGR_Printf("\n--- UNIT %d  PORT %d---", (int)memberPort->slot, (int)memberPort->port);
                                BACKDOOR_MGR_Printf("\nActor system priority %d",(int)(memberPort->AggActorPort).system_priority);
                                BACKDOOR_MGR_Printf("\n\tActor SyetmId: %02x-%02x-%02x-%02x-%02x-%02x",(int)(memberPort->AggActorPort).system_id[0],
                                                                     (int)(memberPort->AggActorPort).system_id[1],
                                                                     (int)(memberPort->AggActorPort).system_id[2],
                                                                     (int)(memberPort->AggActorPort).system_id[3],
                                                                     (int)(memberPort->AggActorPort).system_id[4],
                                                                     (int)(memberPort->AggActorPort).system_id[5]);
                                BACKDOOR_MGR_Printf("\nActor KEY %d",(int)(memberPort->AggActorPort).key);
                                BACKDOOR_MGR_Printf("\nPartner system priority %d",(int)(memberPort->AggPartnerPort).system_priority);
                                BACKDOOR_MGR_Printf("\n\tActor SyetmId: %02x-%02x-%02x-%02x-%02x-%02x",(int)(memberPort->AggPartnerPort).system_id[0],
                                                                     (int)(memberPort->AggPartnerPort).system_id[1],
                                                                     (int)(memberPort->AggPartnerPort).system_id[2],
                                                                     (int)(memberPort->AggPartnerPort).system_id[3],
                                                                     (int)(memberPort->AggPartnerPort).system_id[4],
                                                                     (int)(memberPort->AggPartnerPort).system_id[5]);
                                BACKDOOR_MGR_Printf("\nPartner KEY %d",(int)(memberPort->AggPartnerPort).key);
                                BACKDOOR_MGR_Printf("\n");
                                BACKDOOR_MGR_Printf("\n--- UNIT %d  PORT %d---", (int)pPort->slot, (int)pPort->port);
                                BACKDOOR_MGR_Printf("\nActor system priority %d",(int)(pPort->AggActorPort).system_priority);
                                BACKDOOR_MGR_Printf("\n\tActor SyetmId: %02x-%02x-%02x-%02x-%02x-%02x",(int)(pPort->AggActorPort).system_id[0],
                                                                     (int)(pPort->AggActorPort).system_id[1],
                                                                     (int)(pPort->AggActorPort).system_id[2],
                                                                     (int)(pPort->AggActorPort).system_id[3],
                                                                     (int)(pPort->AggActorPort).system_id[4],
                                                                     (int)(pPort->AggActorPort).system_id[5]);
                                BACKDOOR_MGR_Printf("\nActor KEY %d",(int)(pPort->AggActorPort).key);
                                BACKDOOR_MGR_Printf("\nPartner system priority %d",(int)(pPort->AggPartnerPort).system_priority);
                                BACKDOOR_MGR_Printf("\n\tActor SyetmId: %02x-%02x-%02x-%02x-%02x-%02x",(int)(pPort->AggPartnerPort).system_id[0],
                                                                     (int)(pPort->AggPartnerPort).system_id[1],
                                                                     (int)(pPort->AggPartnerPort).system_id[2],
                                                                     (int)(pPort->AggPartnerPort).system_id[3],
                                                                     (int)(pPort->AggPartnerPort).system_id[4],
                                                                     (int)(pPort->AggPartnerPort).system_id[5]);
                                BACKDOOR_MGR_Printf("\nPartner KEY %d",(int)(pPort->AggPartnerPort).key);
                            }



                            if (((memberPort->AggActorPort).system_priority == (pPort->AggActorPort).system_priority) &&
                                (0 == memcmp((memberPort->AggActorPort).system_id,(pPort->AggActorPort).system_id, 0x6)) &&
                                ((memberPort->AggActorPort).key == (pPort->AggActorPort).key) &&
                                ((memberPort->AggPartnerPort).system_priority == (pPort->AggPartnerPort).system_priority)&&
                                (0 == memcmp((memberPort->AggPartnerPort).system_id,(pPort->AggPartnerPort).system_id, 0x6)) &&
                                ((memberPort->AggPartnerPort).key == (pPort->AggPartnerPort).key) &&
                                (memberPort->PortSpeed == pPort->PortSpeed))
                            {
                                if(bLACP_DbgTrace)
                                {
                                    BACKDOOR_MGR_Printf("\nReturn trunk_id after compare LAG ID %d", (int)al_id);
                                }
                                return al_id;
                            }
                            else
                                continue;
                        }
                        else
                        {
                            if(bLACP_DbgTrace)
                            {
                                BACKDOOR_MGR_Printf("\n[unit] %d port[%d] not trunk port member ", (int)member_unit, (int)member_port);
                                if (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                                    BACKDOOR_MGR_Printf("\nPort type is SWCTRL_LPORT_TRUNK_PORT_MEMBER");
                                else if (port_type == SWCTRL_LPORT_NORMAL_PORT)
                                    BACKDOOR_MGR_Printf("\nPort type is SWCTRL_LPORT_NORMAL_PORT");
                                else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                                    BACKDOOR_MGR_Printf("\nPort type is SWCTRL_LPORT_TRUNK_PORT");
                                else
                                    BACKDOOR_MGR_Printf("\nPort type is unknown");
                            }
                        }

                    }
                }
            }
            else
            {
                if(bLACP_DbgTrace)
                {
                    BACKDOOR_MGR_Printf("\nFail to get dynamic trunk id %d", (int)al_id);
                }
            }

        } /* end of if */
    }

    if(num_of_trunks < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
    {
        /* We still have space to add trunk. */
        for(al_id = 1; al_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; al_id++)
        {
            trunk_entry.trunk_index = al_id;

            if(FALSE == TRK_PMGR_GetTrunkEntry( &trunk_entry))
            {
                return al_id;
            }
        }
    }

    return 0; /* No more trunk is available. */
}
#endif
UI32_T LACP_Get_Available_TrunkId( UI32_T slot, UI32_T port)
{
    LACP_PORT_T *port_ptr;

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nLACP_Get_Available_TrunkId");
    }
    if(STKTPLG_POM_PortExist( slot, port) == FALSE)
    {
        return 0;
    }

    port_ptr    = LACP_FIND_PORT( slot, port);

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nFind Port Unit %d, Slot %d", (int)port_ptr->slot, (int)port_ptr->port);
    }

    /*  This section is to prevent LACP to form a trunk for single link
        when there looping exist. (port 1 connects to port 10 )
     */

    if(bLACP_DbgTrace)
    {
        BACKDOOR_MGR_Printf("\nActor system priority %d",(port_ptr->AggActorPort).system_priority);
        BACKDOOR_MGR_Printf("\nActor system priority %d",(port_ptr->AggPartnerPort).system_priority);

        BACKDOOR_MGR_Printf("\n\tActor SyetmId: %02x-%02x-%02x-%02x-%02x-%02x",   (int)(port_ptr->AggActorPort).system_id[0],
                                                                     (int)(port_ptr->AggActorPort).system_id[1],
                                                                     (int)(port_ptr->AggActorPort).system_id[2],
                                                                     (int)(port_ptr->AggActorPort).system_id[3],
                                                                     (int)(port_ptr->AggActorPort).system_id[4],
                                                                     (int)(port_ptr->AggActorPort).system_id[5]);

        BACKDOOR_MGR_Printf("\n\nPartner SyetmId: %02x-%02x-%02x-%02x-%02x-%02x", (int)(port_ptr->AggPartnerPort).system_id[0],
                                                                     (int)(port_ptr->AggPartnerPort).system_id[1],
                                                                     (int)(port_ptr->AggPartnerPort).system_id[2],
                                                                     (int)(port_ptr->AggPartnerPort).system_id[3],
                                                                     (int)(port_ptr->AggPartnerPort).system_id[4],
                                                                     (int)(port_ptr->AggPartnerPort).system_id[5]);
    }

    if (    (   (port_ptr->AggActorPort).system_priority == (port_ptr->AggPartnerPort).system_priority)
         && (   memcmp((port_ptr->AggActorPort).system_id,(port_ptr->AggPartnerPort).system_id, 0x6)
                == 0
            )
       )
    {
        if(bLACP_DbgTrace)
        {
            BACKDOOR_MGR_Printf("\nERROR!!  This link is a Loop");
        }

        return 0;
    }
    else
    {
        return (port_ptr->pAggregator->AggID);
    }

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_RX_HandlePacket
 *-------------------------------------------------------------------------
 * FUNCTION : This function really handles the incoming LACP packets.
 * INPUT    : LACP_MSG_T *lacp_msg_ptr: the pointer points to
 *                              a message containing packet information.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE :
 *      It allocate a memory buffer via standard C malloc() function and
 *      copy the LACP packet payload to the new buffer. It then free the
 *      packet buffer by calling the free function pointed by the
 *      pktFrtn.free_func function pointer.
 *      Finally it packages these information (packet content buffer,
 *      port number) into a message and sends it to the LACP input queue.
 *      When the LACP received the message from the input queue, it processes
 *      the packet content and modifies its protocol state if necessary.
 *-------------------------------------------------------------------------
 */
void  LACP_RX_HandlePacket(LACP_MSG_T *lacp_msg_ptr)
{
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI32_T              unit_no, port_no, pdu_len;
    LACP_PDU_U          *lacp_buffer;
    LACP_PORT_T *pPort;
    LACP_SYSTEM_T       *lacp_system_ptr;

    mref_handle_p = lacp_msg_ptr->mref_handle_p;
    unit_no = lacp_msg_ptr->unit_no;
    port_no = lacp_msg_ptr->port_no;

    if(bLACP_DbgPdu)
    {
        BACKDOOR_MGR_Printf("\r\n --RX a LACP packet on %d/%d --\n",(int)unit_no,(int)port_no);
    }

    if(bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP_RX_HandlePacket slot:%d port:%d", (int)unit_no,(int)port_no);
    }

    pPort = LACP_FIND_PORT( unit_no, port_no);
    if (pPort == NULL)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    if (    (lacp_system_ptr->lacp_oper == LACP_ADMIN_ON)
        &&  pPort->LACP_OperStatus
        &&  pPort->Port_OperStatus
       )
    {
        lacp_buffer = (LACP_PDU_U*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(lacp_buffer==NULL)
            return;

        if (lacp_buffer->lacp.protocol_subtype == LACP_LACP)
        {
            if (    (   (pPort->AggActorPort).system_priority == (lacp_buffer->lacp).actor.system_priority)
                 && (   memcmp((pPort->AggActorPort).system_id,(lacp_buffer->lacp).actor.system_id, 0x6)
                        == 0
                    )
               )
            {
                if(bLACP_DbgPdu)
                {
                    BACKDOOR_MGR_Printf("\nDiscard PDU!!  This link is a Loop!\n");
                }

                L_MM_Mref_Release(&mref_handle_p);
                return;
            }
        }

        if ( lacp_buffer->lacp.protocol_subtype == LACP_LACP || lacp_buffer->lacp.protocol_subtype == LACP_MARKER)
        {

            DEBUG_PACKET("%srx LACPDU from unit %u port %u; SA: %02x-%02x-%02x-%02x-%02x-%02x", DEBUG_HEADER(),
                lacp_msg_ptr->unit_no, lacp_msg_ptr->port_no, lacp_msg_ptr->saddr[0], lacp_msg_ptr->saddr[1],
                lacp_msg_ptr->saddr[2], lacp_msg_ptr->saddr[3], lacp_msg_ptr->saddr[4], lacp_msg_ptr->saddr[5]);

            SPRINT_MEMORY(DEBUG_PageBuffer, lacp_buffer, pdu_len);
            DEBUG_PACKET_DETAIL("\r\n%s", DEBUG_PageBuffer);

            LACP_HandlePdu(unit_no, port_no, mref_handle_p);
        }
        else
        {
            if(bLACP_DbgPdu)
            {
                BACKDOOR_MGR_Printf("\nprotocol subtype error!\n");
            }

            L_MM_Mref_Release(&mref_handle_p);
        }
    }
    else
    {
        if(bLACP_DbgPdu)
        {
            BACKDOOR_MGR_Printf("\nSystem LACP_ADMIN_OFF or Port's LACP_OperStatus(lacp_admin && FullDupex)==0 or Port's Port_OperStatus(port_admin && linkup)==0 \n");
        }

        L_MM_Mref_Release(&mref_handle_p);
    }
}/* End of LACP_RX_HandlePacket() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Is_Empty_Trunk
 *------------------------------------------------------------------------
 * FUNCTION: This function is understand whethe this trunk id is empty or
 *           a existing id.
 * INPUT   : mref -- LReference to data structure.
 *           slot -- slot number.
 *           port -- port number.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Is_Empty_Trunk( UI32_T trunk_id)
{
    TRK_MGR_TrunkEntry_T trunk_entry;

    trunk_entry.trunk_index = trunk_id;

    if (TRK_PMGR_GetTrunkEntry(&trunk_entry))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Print_PDU_Info
 *------------------------------------------------------------------------
 * FUNCTION: This function is used when debug.
 * INPUT   : pPDU   -- pointer to data structure.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Print_PDU_Info( LACP_PDU_U *pPDU)
{
    BACKDOOR_MGR_Printf("\nProtocol Subtype: %u", (int)((pPDU->lacp).protocol_subtype));
    BACKDOOR_MGR_Printf("\nProtocol Version: %u", (int)((pPDU->lacp).protocol_version));

    switch((int)((pPDU->lacp).protocol_subtype))
    {
    case LACP_LACP:
        BACKDOOR_MGR_Printf("\nActor:");
        LACP_Show_Info( &((pPDU->lacp).actor));
        BACKDOOR_MGR_Printf("\nPartner:");
        LACP_Show_Info( &((pPDU->lacp).partner));
        break;
    case LACP_MARKER:
        break;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_SetInterface
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to set the interface as up or down.
 * INPUT   : slot -- slot number.
 *           port -- port number.
 *           up_or_down -- TRUE/FALSE for Up/Down.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_SetInterface( UI32_T unit, UI32_T port, BOOL_T up_or_down)
{
    /* UI32_T  ifIndex; */
    UI32_T  interface_status;

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        return;
    }

#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
    LACP_PORT_T *port_ptr = NULL;
    SYS_TYPE_Uport_T unit_port;
    UI32_T trunk_id;

    unit_port.unit = (UI16_T)unit;
    unit_port.port = (UI16_T)port;

    port_ptr = LACP_FIND_PORT(unit, port);
    if(port_ptr
        && port_ptr->Is_static_lacp == TRUE
        && port_ptr->Last_InterfaceStatus == FALSE
        && up_or_down == FALSE)
    {
        return;
    }

    port_ptr->Last_InterfaceStatus = up_or_down;

    if(port_ptr && port_ptr->Is_static_lacp == TRUE)
    {
        if(SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_UserPortToTrunkPort(unit, port, &trunk_id))
        {
            SWCTRL_PMGR_SetTrunkMemberActiveStatus(trunk_id, unit_port, up_or_down);
        }
    }
#endif

 /*   SWCTRL_UserPortToLogicalPort( unit, port, &ifIndex);*/

    if(up_or_down == TRUE)
    {
        interface_status = VAL_LacpCollecting_collecting;
    }
    else
    {
        interface_status = VAL_LacpCollecting_not_collecting;
    }

    SWCTRL_PMGR_SetPortLacpCollecting(LACP_LINEAR_ID(unit, port), interface_status);
}

#if 0   /* Redesigned */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Appropriate_Port_From_ifIndex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get an appropiate port from the
 *           pass in interface index. If pass in ifIndex is a normal port,
 *           and not a trunk port then it will just return that port. If
 *           pass in ifIndex is a logical index then it should be a trunk
 *           port. If the logical ifIndex is a empty index then false else
 *           if it is a static LA then it will return the first port, if
 *           it is a dynamic LA, then it will return the first port in the
 *           aggregator.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
static LACP_PORT_T * LACP_Get_Appropriate_Port_From_ifIndex( UI32_T agg_index)
{
    UI32_T slot, port;
    UI32_T trunk_id,select_trunk_id;
    /* UI32_T port_status; */
    UI32_T i,j;
    UI32_T port_ifIndex = 0;

    LACP_PORT_T       *pPort;
    /* LACP_AGGREGATOR_T *pAgg; */

    TRK_MGR_TrunkEntry_T trunk_entry;

    if( (agg_index <= 0) ||
        (agg_index > MAX_PORTS) )
    {
        /* This means pass in parameters is wrong. */
        return (LACP_PORT_T *)NULL;
    }

    memset( &trunk_entry, 0x0, sizeof(TRK_MGR_TrunkEntry_T));

    if(agg_index <= MAX_PHYSICAL_PORTS)
    {
        /* This is a physical port. */
        if(SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort( agg_index, &slot, &port, &trunk_id))
        {
            /* Once it is in trunk, the database should always consider it as in the   */
            /* logical interface. If user try to access it from normal port, we should */
            /* consider it as a fault. */
            return (LACP_PORT_T *)NULL;
        }
        else
        {
            pPort = LACP_FIND_PORT( slot, port);
        }

        if(pPort)
        {
            if(STANDBY == pPort->Selected)
            {
                return (LACP_PORT_T *)NULL;
            }

            return pPort;
        }

    }
    else
    {
        /* He want a trunk port.                        */
        /* We have to check it is a logical interface.  */
        if(SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort( agg_index, &slot, &port, &trunk_id))
        {
            return (LACP_PORT_T *)NULL;
        }

        trunk_entry.trunk_index = trunk_id;

        if(FALSE == TRK_PMGR_GetTrunkEntry( &trunk_entry))
        {
            return (LACP_PORT_T *)NULL;
        }

        for(i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
        {
            if(trunk_entry.trunk_ports[i])
            {
                for(j=0;j<8;j++)
                {
                    if(trunk_entry.trunk_ports[i] & (1<<(7-j)))
                    {
                        port_ifIndex += (j+1);
                        break;
                    }
                }

                break;
            }

            /* One octet has 8 ports. */
            port_ifIndex += 8;
        }

        /* Now we got the port ifIndex and it must be the first port ifIndex. */
        if(SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_LogicalPortToUserPort( port_ifIndex, &slot, &port, &select_trunk_id))
        {
            if(FALSE == TRK_PMGR_IsDynamicTrunkId( select_trunk_id))
            {
                pPort = LACP_FIND_PORT( slot, port);
                if(pPort)
                {
                    return pPort;
                }

                return (LACP_PORT_T *)NULL;
            }

            pPort = LACP_FIND_PORT( slot, port);
            /* This port is not always the first port in aggregator. */
            /* So we have to find the real port we want.             */
            pPort = LACP_Get_First_Port_In_Aggregator( pPort);

            if(pPort)
            {
                return pPort;
            }
        }
        else
        {
            BACKDOOR_MGR_Printf("\nError in transform to ifIndex in LACP component...");
        }
    }

    return (LACP_PORT_T *)NULL;
}
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetNext_Appropriate_ifIndex_From_ifIndex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get an appropiate next ifIndex from the
 *           pass in interface index.
 * INPUT   : agg_index -- index of aggregator to get next
 * OUTPUT  : None
 * RETURN  : 0 -- Not found
 *           other -- next interface index.
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_GetNext_Appropriate_ifIndex_From_ifIndex( UI32_T agg_index)
{
#if 0
    UI32_T slot, port;
    UI32_T trunk_id;
#endif
    UI32_T next_ifIndex = agg_index;
    SWCTRL_Lport_Type_T lport_type;

    while(TRUE)
    {
        lport_type=SWCTRL_GetNextLogicalPort(&next_ifIndex);

        if(SWCTRL_LPORT_TRUNK_PORT == lport_type)
        {
            return next_ifIndex;
        }
        else if(SWCTRL_LPORT_UNKNOWN_PORT == lport_type)
        {
            return 0;
        }
    }

#if 0
    Port_Info_T         portInfo;

    if(agg_index > MAX_PHYSICAL_PORTS)
    {
        /* This means pass in parameters is wrong or is a pure logical port. */
        return 0;
    }

    while(TRUE == SWCTRL_GetNextPortInfo( &next_ifIndex, &portInfo))
    {
        lport_type = SWCTRL_LogicalPortToUserPort( next_ifIndex, &slot, &port, &trunk_id);

        if(SWCTRL_LPORT_TRUNK_PORT == lport_type)
        {
            return next_ifIndex;
        }
        else if(SWCTRL_LPORT_UNKNOWN_PORT == lport_type)
        {
            return 0;
        }

    }
#endif

    return 0;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_Appropriate_Physical_Port_From_ifIndex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get an appropiate port from the
 *           pass in interface index. If pass in ifIndex is a normal port,
 *           and not a trunk port then it will just return that port. If
 *           pass in ifIndex is a logical index then it should be a trunk
 *           port. If the logical ifIndex is a empty index then false else
 *           if it is a static LA then it will return the first port, if
 *           it is a dynamic LA, then it will return the first port in the
 *           aggregator.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
static LACP_PORT_T * LACP_Get_Appropriate_Physical_Port_From_ifIndex( UI32_T agg_index)
{
    /* This API only accept a physical port. It will return NULL when a */
    /* logical interface is up.                                         */
    UI32_T slot, port;
    UI32_T trunk_id; /* ,select_trunk_id; */
    /* UI32_T port_status; */

    LACP_PORT_T       *pPort;
    /* LACP_AGGREGATOR_T *pAgg; */

    SWCTRL_Lport_Type_T lport_type = SWCTRL_LPORT_UNKNOWN_PORT;

    if( (agg_index <= 0) ||
        (agg_index > MAX_PHYSICAL_PORTS) )
    {
        /* This means pass in parameters is wrong or is a pure logical port. */
        return (LACP_PORT_T *)NULL;
    }

    if(agg_index <= MAX_PHYSICAL_PORTS)
    {
        /* This is a physical port. */
        lport_type = SWCTRL_LogicalPortToUserPort( agg_index, &slot, &port, &trunk_id);

        if( (SWCTRL_LPORT_TRUNK_PORT == lport_type) ||
            (SWCTRL_LPORT_UNKNOWN_PORT == lport_type) )
        {
            return (LACP_PORT_T *)NULL;
        }

        /* We do not care about whether it is a trunk port or not. */
        pPort = LACP_FIND_PORT( slot, port);

        if(pPort)
        {
            return pPort;
        }

    }

    return (LACP_PORT_T *)NULL;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to get an appropiate ifIndex from the
 *           pass in interface index.
 * INPUT   : pPort -- pointer to port
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex( UI32_T agg_index)
{
    /* This API only accept a physical port. It will return NULL when a */
    /* logical interface is up.                                         */
    UI32_T slot, port;
    UI32_T trunk_id;
    UI32_T next_ifIndex = agg_index;

    SWCTRL_Lport_Type_T lport_type;
    Port_Info_T         portInfo;

    if(agg_index > MAX_PHYSICAL_PORTS)
    {
        /* This means pass in parameters is wrong or is a pure logical port. */
        return 0;
    }

    if(agg_index == 0)
    {
        return 1;
    }
    else
    {
        while(TRUE == SWCTRL_GetNextPortInfo( &next_ifIndex,&portInfo))
        {
            lport_type = SWCTRL_LogicalPortToUserPort( next_ifIndex, &slot, &port, &trunk_id);

            if( (SWCTRL_LPORT_NORMAL_PORT == lport_type) ||
                (SWCTRL_LPORT_TRUNK_PORT_MEMBER == lport_type) )
            {
                return next_ifIndex;
            }
            else if( (SWCTRL_LPORT_UNKNOWN_PORT == lport_type) ||
                     (SWCTRL_LPORT_TRUNK_PORT == lport_type) )
            {
                return 0;
            }

        }
    }

    return 0;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggMACAddress
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator MAC address
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pMAC      -- MAC address of the aggregator.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggMACAddress( UI32_T agg_id, UI8_T *pMAC)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
{
        /* This AggMAC don't implement, and for conformace with swctrl just get from swctrl
         * macauley, ASE3528B-E1-FLF-P5-00187
         */
#if 0
        pMAC[0] = aggregator_ptr->AggMACAddress[0];
        pMAC[1] = aggregator_ptr->AggMACAddress[1];
        pMAC[2] = aggregator_ptr->AggMACAddress[2];
        pMAC[3] = aggregator_ptr->AggMACAddress[3];
        pMAC[4] = aggregator_ptr->AggMACAddress[4];
        pMAC[5] = aggregator_ptr->AggMACAddress[5];
#endif
        SWCTRL_GetPortMac(aggregator_ptr->AggID+SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER-1, pMAC);
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T       *pPort;
    LACP_AGGREGATOR_T *pAgg;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
#if 0   /* Allenc */
        pAgg = LACP_FIND_AGGREGATOR( pPort->slot, pPort->port);
#endif  /* Allenc */
        pAgg    = LACP_UTIL_AllocateAggregator(pPort);

        if(pAgg)
        {
            /* I don't want to be swapped in pSOS. */
            /* memcpy( pMAC, pAgg->AggMACAddress, 0x6); */
            pMAC[0] = pAgg->AggMACAddress[0];
            pMAC[1] = pAgg->AggMACAddress[1];
            pMAC[2] = pAgg->AggMACAddress[2];
            pMAC[3] = pAgg->AggMACAddress[3];
            pMAC[4] = pAgg->AggMACAddress[4];
            pMAC[5] = pAgg->AggMACAddress[5];
            return TRUE;
        }
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Actor Priority
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of the system.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorSystemPriority( UI32_T agg_id, UI16_T *pPriority)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pPriority = aggregator_ptr->AggActorSystemPriority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    /* LAC_INFO_T  *pInfo; */
    LACP_SYSTEM_T       *lacp_system_ptr;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    /* If we have this index port then we give it else we just ignore it. */
    if(pPort)
    {
        lacp_system_ptr = LACP_OM_GetSystemPtr();
        *pPriority = lacp_system_ptr->SystemPriority;

        /* pInfo = &(pPort->AggActorPort); */

        /* *pPriority = pInfo->system_priority; */
        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Actor Priority
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pMAC      -- MAC address of Actor System.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorSystemID( UI32_T agg_id, UI8_T *pMAC)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        pMAC[0] = aggregator_ptr->AggActorSystemID[0];
        pMAC[1] = aggregator_ptr->AggActorSystemID[1];
        pMAC[2] = aggregator_ptr->AggActorSystemID[2];
        pMAC[3] = aggregator_ptr->AggActorSystemID[3];
        pMAC[4] = aggregator_ptr->AggActorSystemID[4];
        pMAC[5] = aggregator_ptr->AggActorSystemID[5];
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    /* LAC_INFO_T  *pInfo; */
    LACP_SYSTEM_T       *lacp_system_ptr;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    /* If we have this index port then we give it else we just ignore it. */
    if(pPort)
    {
        lacp_system_ptr = LACP_OM_GetSystemPtr();
        pMAC[0] = lacp_system_ptr->SystemId[0];
        pMAC[1] = lacp_system_ptr->SystemId[1];
        pMAC[2] = lacp_system_ptr->SystemId[2];
        pMAC[3] = lacp_system_ptr->SystemId[3];
        pMAC[4] = lacp_system_ptr->SystemId[4];
        pMAC[5] = lacp_system_ptr->SystemId[5];

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggAggregateOrIndividual
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator aggregate ability
 * INPUT   : agg_id        -- index of the aggregator.
 * OUTPUT  : pAggregatable -- aggregate ability of the aggregator.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggAggregateOrIndividual( UI32_T agg_id, UI8_T *pAggregatable)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pAggregatable = aggregator_ptr->AggAggregateOrIndividual;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        if(agg_index <= MAX_PHYSICAL_PORTS)
        {
            *pAggregatable = FALSE;
        }
        else
        {
            *pAggregatable = TRUE;
        }

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator admin key
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pAdminKey -- administration key of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorAdminKey( UI32_T agg_id, UI16_T *pAdminKey)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pAdminKey = aggregator_ptr->AggActorAdminKey;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorAdminPort);

        *pAdminKey = pInfo->key;

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator operational key
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorOperKey( UI32_T agg_id, UI16_T *pOperKey)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pOperKey = aggregator_ptr->AggActorOperKey;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        *pOperKey = pInfo->key;

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPartnerSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner ID.
 * INPUT   : agg_id -- index of the aggregator.
 * OUTPUT  : pMAC  -- Id of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPartnerSystemID( UI32_T agg_id, UI8_T *pMAC)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        pMAC[0] = aggregator_ptr->AggPartnerSystemID[0];
        pMAC[1] = aggregator_ptr->AggPartnerSystemID[1];
        pMAC[2] = aggregator_ptr->AggPartnerSystemID[2];
        pMAC[3] = aggregator_ptr->AggPartnerSystemID[3];
        pMAC[4] = aggregator_ptr->AggPartnerSystemID[4];
        pMAC[5] = aggregator_ptr->AggPartnerSystemID[5];
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        pMAC[0] = pInfo->system_id[0];
        pMAC[1] = pInfo->system_id[1];
        pMAC[2] = pInfo->system_id[2];
        pMAC[3] = pInfo->system_id[3];
        pMAC[4] = pInfo->system_id[4];
        pMAC[5] = pInfo->system_id[5];

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPartnerSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner priority.
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPartnerSystemPriority( UI32_T agg_id, UI16_T *pPriority)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pPriority = aggregator_ptr->AggPartnerSystemPriority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pPriority = pInfo->system_priority;

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPartnerOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner operational
 *           key.
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPartnerOperKey( UI32_T agg_id, UI16_T *pOperKey)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pOperKey = aggregator_ptr->AggPartnerOperKey;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pOperKey = pInfo->key;

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggCollectorMaxDelay
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Partner operational
 *           key.
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pDelay    -- MaxDelay of Collector.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggCollectorMaxDelay( UI32_T agg_id, UI16_T *pDelay)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
{
        *pDelay = aggregator_ptr->AggCollectorMaxDelay;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

#if 0 /* kelly */
    LACP_PORT_T *pPort;

    pPort = LACP_Get_Appropriate_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        /* No matter which aggregator or port it must be 0. */
        /* Unless you want to measure it.                   */
        *pDelay = 0;

        return TRUE;
    }

    return FALSE;
#endif /* kelly */
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggActorTimeout
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get related aggregator Actor Timeout
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : pTimeout -- timeout of the aggregator Actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggActorTimeout( UI32_T agg_id, UI32_T *pTimeout)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        *pTimeout = aggregator_ptr->AggActorTimeout;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;
}

#if 0
int LACP_GetLine(char s[], int lim)
{
    int c, i;

    for (i=0; i<lim-1 && (c=getchar()) != 0 && c!='\n'; ++i)
    {
        s[i] = c;
        BACKDOOR_MGR_Printf("%c", c);
    }
    if (c == '\n')
    {
        s[i] = c;
        ++i;
    }
    s[i] = '\0';
    return i;
}
#endif
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortListPorts
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a port list related to the
 *           aggregator which the pass in ifIndex associated with.
 * INPUT   : agg_id     -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortListPorts( UI32_T agg_id, UI8_T *pList)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *agg_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    agg_ptr         = &(lacp_system_ptr->aggregator[agg_id]);

    return LACP_UTIL_GetAggMemberList( agg_ptr, pList);
}

#if 0   /* Redesigned */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_PortListPorts
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a port list related to the
 *           aggregator which the pass in ifIndex associated with.
 * INPUT   : pPort -- port to aggregator port.
 * OUTPUT  : pList -- port list bit array.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T LACP_Get_PortListPorts( LACP_PORT_T *pPort, UI8_T *pList)
{
    LACP_PORT_T       *pMemberPort;
    LACP_AGGREGATOR_T *pAgg;

    UI32_T              linearNo;
    UI32_T             octet;
    UI32_T             bit;
    UI32_T             trunk_id;

    TRK_MGR_TrunkEntry_T trunk_entry;

    memset( pList, 0x0, LACP_PORT_LIST_OCTETS);

    if(pPort)
    {
        pAgg = pPort->pAggregator;

        if(pAgg)
        {
            /* Now find all ports in the aggregator. */
            pMemberPort = LACP_Get_First_Port_In_Aggregator( pPort);

            while(pMemberPort)
            {
                linearNo = LACP_LINEAR_ID( pMemberPort->slot, pMemberPort->port);

                octet = (linearNo-1)/8;
                bit   = (linearNo-1)%8;

                pList[octet] |= (UI8_T)(1<<(7-bit));
                #if 0
                octet = linearNo/8;
                bit   = linearNo%8;

                if(bit)
                {
                    pList[octet] |= (UI8_T)(1<<(8-bit));
                }
                else
                {
                    pList[octet] |= (UI8_T)1;
                }
                #endif

                pMemberPort = LACP_Get_Next_Port_In_Aggregator( pMemberPort);
            }

            return TRUE;
        }
        else
        {
            /* This must be a port that did not join LACP now.              */
            /* what we have to do is to check this is a trunk port or not.  */
            /* If this is a trunk port, then add all its related port, else */
            /* Only add itself.                                             */
            if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunk_id))
            {
                trunk_entry.trunk_index = trunk_id;

                if(FALSE == TRK_PMGR_GetTrunkEntry( &trunk_entry))
                {
                    return FALSE;
                }
                else
                    memcpy(pList, trunk_entry.trunk_ports, LACP_PORT_LIST_OCTETS);
            }
            else
            {
                linearNo = LACP_LINEAR_ID(pPort->slot, pPort->port);

                octet = (linearNo-1)/8;
                bit   = (linearNo-1)%8;

                pList[octet] |= (UI8_T)(1<<(7-bit));
            }

            return TRUE;
        }
    }

    return FALSE;
}
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - dot3adAggPortActorSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get actor system priority of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorSystemPriority( UI32_T agg_index, UI16_T *pPriority)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        *pPriority = pInfo->system_priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a actor system id of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorSystemID( UI32_T agg_index, UI8_T *pSystemID)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        pSystemID[0] = pInfo->system_id[0];
        pSystemID[1] = pInfo->system_id[1];
        pSystemID[2] = pInfo->system_id[2];
        pSystemID[3] = pInfo->system_id[3];
        pSystemID[4] = pInfo->system_id[4];
        pSystemID[5] = pInfo->system_id[5];
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get actor admin key of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pList     -- Bit array of port list.
 * RETURN  : TRUE  -- Get port list.
 *           FALSE -- fail to get port list
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorAdminKey( UI32_T agg_index, UI16_T *pAdminKey)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorAdminPort);

        *pAdminKey = pInfo->key;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get oper. key of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorOperKey( UI32_T agg_index, UI16_T *pOperKey)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        *pOperKey = pInfo->key;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner admin system priority of
 *           a port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminSystemPriority( UI32_T agg_index, UI16_T *pPriority)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerAdminPort);

        *pPriority = pInfo->system_priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperSystemPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner oper. system priority.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPriority -- priority of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperSystemPriority( UI32_T agg_index, UI16_T *pPriority)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pPriority = pInfo->system_priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get partner admin system id of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMAC  -- Id of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminSystemID( UI32_T agg_index, UI8_T *pMAC)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerAdminPort);

        pMAC[0] = pInfo->system_id[0];
        pMAC[1] = pInfo->system_id[1];
        pMAC[2] = pInfo->system_id[2];
        pMAC[3] = pInfo->system_id[3];
        pMAC[4] = pInfo->system_id[4];
        pMAC[5] = pInfo->system_id[5];
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperSystemID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner Oper. system id of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMAC  -- Id of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperSystemID( UI32_T agg_index, UI8_T *pMAC)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        pMAC[0] = pInfo->system_id[0];
        pMAC[1] = pInfo->system_id[1];
        pMAC[2] = pInfo->system_id[2];
        pMAC[3] = pInfo->system_id[3];
        pMAC[4] = pInfo->system_id[4];
        pMAC[5] = pInfo->system_id[5];
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner admin key of a physical
 *           port.
 *           key.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminKey( UI32_T agg_index, UI16_T *pAdminKey)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerAdminPort);

        *pAdminKey = pInfo->key;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner oper. key of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperKey  -- operational key of Partner.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperKey( UI32_T agg_index, UI16_T *pOperKey)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pOperKey = pInfo->key;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortSelectedAggID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a selected aggregator id of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pSelectedAggID  -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortSelectedAggID( UI32_T agg_index, UI32_T *pSelectedAggID)
{
    LACP_PORT_T       *pPort;
    LACP_AGGREGATOR_T *pAgg;

    UI32_T             trunk_id;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pAgg = pPort->pAggregator;

        if(!pAgg)
        {
            if( (TRUE == pPort->LACP_OperStatus) &&
                (TRUE == pPort->Port_OperStatus) )
            {
                /* Now we are in detach state or process it. */
                *pSelectedAggID = 0;

                return TRUE;
            }

#if 0   /* Allenc */
            pAgg = LACP_FIND_AGGREGATOR( pPort->slot, pPort->port);
#endif  /* Allenc */
            pAgg    = LACP_UTIL_AllocateAggregator(pPort);
        }
        /* We have to distinguish a port to trunk port or normal port. */
        if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunk_id))
        {
            /* This is a trunk member, so we have to return its logical aggregator */
            /* index.                                                              */
            *pSelectedAggID = trunk_id+MAX_PHYSICAL_PORTS;

            return TRUE;
        }
        if(!pAgg)
            *pSelectedAggID = 0;
     else
     {
            *pSelectedAggID = pAgg->AggID;
            LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
     }
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortAttachedAggID
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a attach aggregator of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAttachedAggID  -- attached aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortAttachedAggID( UI32_T agg_index, UI32_T *pAttachedAggID)
{
    LACP_PORT_T       *pPort;
    LACP_AGGREGATOR_T *pAgg;
    UI32_T             trunk_id;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pAgg = pPort->pAggregator;

        if(!pAgg)
        {
            if( (TRUE == pPort->LACP_OperStatus) &&
                (TRUE == pPort->Port_OperStatus) )
            {
                /* Now we are in detach state or process it. */
                *pAttachedAggID = 0;

                return TRUE;
            }
#if 0   /* Allenc */
            pAgg = LACP_FIND_AGGREGATOR( pPort->slot, pPort->port);
#endif  /* Allenc */
            pAgg    = LACP_UTIL_AllocateAggregator(pPort);
        }

        /* We have to distinguish a port to trunk port or normal port. */
        if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunk_id))
        {
            /* This is a trunk member, so we have to return its logical aggregator */
            /* index.                                                              */
            *pAttachedAggID = trunk_id+MAX_PHYSICAL_PORTS;

            return TRUE;
        }
        if(!pAgg)
            *pAttachedAggID = 0;
     else
     {
            *pAttachedAggID = pAgg->AggID;
            LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
     }

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Actor port number.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortNo   -- actor port number.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorPort( UI32_T agg_index, UI16_T *pPortNo)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        *pPortNo = pInfo->port_no;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorPortPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Actor port priority of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortPriority  -- actor port priority.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorPortPriority( UI32_T agg_index, UI16_T *pPortPriority)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        *pPortPriority = pInfo->port_priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get Partner admin port number of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortNo  -- admin port number of partner.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminPort( UI32_T agg_index, UI16_T *pPortNo)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerAdminPort);

        *pPortNo = pInfo->port_no;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperPort
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to oper. value of partner port number of
 *           a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortNo   -- oper. value of partner port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperPort( UI32_T agg_index, UI16_T *pPortNo)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pPortNo = pInfo->port_no;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminPortPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a partner admin port priority of
 *           a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortPriority  -- admin priority of partner port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminPortPriority( UI32_T agg_index, UI16_T *pPortPriority)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerAdminPort);

        *pPortPriority = pInfo->port_priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperPortPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get oper. value of port priority of
 *           a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPortPriority  -- oper. value of partner port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperPortPriority( UI32_T agg_index, UI16_T *pPortPriority)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pPortPriority = pInfo->port_priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorAdminState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get admin actor state of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAdminState  -- admin actor state.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorAdminState( UI32_T agg_index, UI8_T *pAdminState)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorAdminPort);

        *pAdminState = (pInfo->state).port_state;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortActorOperState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get a actor oper. port state of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperState  -- oper. state of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortActorOperState( UI32_T agg_index, UI8_T *pOperState)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggActorPort);

        *pOperState = (pInfo->state).port_state;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerAdminState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get partner admin port state.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAdminState -- partner admin port state.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerAdminState( UI32_T agg_index, UI8_T *pAdminState)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerAdminPort);

        *pAdminState = (pInfo->state).port_state;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortPartnerOperState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get partner oper. port state of a
 *           physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pOperState -- partner oper. state of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortPartnerOperState( UI32_T agg_index, UI8_T *pOperState)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pInfo;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pInfo = &(pPort->AggPartnerPort);

        *pOperState = (pInfo->state).port_state;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortAggregateOrIndividual
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get aggregate ability of a physical
 *           port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pAggregatable -- aggregate ability of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortAggregateOrIndividual( UI32_T agg_index, UI8_T *pAggregatable)
{
    LACP_PORT_T *pPort;
    /* LAC_INFO_T  *pInfo; */
    /*LACP_PORT_STATE_T *pPortState;*/

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        /*pPortState = &(((pPort->AggActorPort).state).lacp_port_state);*/

        /*if(pPortState->Aggregation)*/
        if ( LACP_TYPE_GET_PORT_STATE_BIT(((pPort->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Aggregation) )
        {
            *pAggregatable = LACP_AGGREGATABLE_LINK;
        }
        else
        {
            *pAggregatable = LACP_INDIVIDUAL_LINK;
        }

        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsLACPDUsRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsLACPDUsRx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->LACPDUsRx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerPDUsRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerPDUsRx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->MarkerPDUsRx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerResponsePDUsRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerResponsePDUsRx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->MarkerResponsePDUsRx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsUnknownRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsUnknownRx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->UnknownRx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsIllegalRx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsIllegalRx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->IllegalRx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsLACPDUsTx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsLACPDUsTx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->LACPDUsTx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerPDUsTx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerPDUsTx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->MarkerPDUsTx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortStatsMarkerResponsePDUsTx
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get statistics of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCounter  -- counter.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortStatsMarkerResponsePDUsTx( UI32_T agg_index, UI32_T *pCounter)
{
    LACP_PORT_T *pPort;

    LACP_PORT_STATISTICS_T *pStats;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pStats = &(pPort->AggPortStats);

        *pCounter = pStats->MarkerResponsePDUsTx;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugRxState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of a physical port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pRxState  -- Rx state of port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugRxState( UI32_T agg_index, UI8_T *pRxState)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        *pRxState = (UI8_T)pDebug->RxState;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugLastRxTime
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of a port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pRxTime   -- RxTime of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugLastRxTime( UI32_T agg_index, UI32_T *pRxTime)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        *pRxTime = pDebug->LastRxTime;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugMuxState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of a port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMuxState -- Mux state of a port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugMuxState( UI32_T agg_index, UI8_T *pMuxState)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        *pMuxState = (UI8_T)pDebug->MuxState;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugMuxReason
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of the port.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pMuxReason  -- Mux reason of this port.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugMuxReason( UI32_T agg_index, UI8_T *pMuxReason)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pMuxReason = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorChurnState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of Actor Churn state
 *           machine
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pACState  -- state of actor churn state machine.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorChurnState( UI32_T agg_index, UI8_T *pACState)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pACState = 1;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerChurnState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug state of partner churn
 *           machine
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pPCState  -- state of partner churn state machine.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerChurnState( UI32_T agg_index, UI8_T *pPCState)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pPCState = 1;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorChurnCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorChurnCount( UI32_T agg_index, UI32_T *pCount)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pCount = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerChurnCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerChurnCount( UI32_T agg_index, UI32_T *pCount)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pCount = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorSyncTransitionCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorSyncTransitionCount( UI32_T agg_index, UI32_T *pCount)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pCount = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerSyncTransitionCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerSyncTransitionCount( UI32_T agg_index, UI32_T *pCount)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pCount = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugActorChangeCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugActorChangeCount( UI32_T agg_index, UI32_T *pCount)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pCount = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adAggPortDebugPartnerChangeCount
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get debug counter.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount  -- counter value.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adAggPortDebugPartnerChangeCount( UI32_T agg_index, UI32_T *pCount)
{
    LACP_PORT_T *pPort;

    LACP_PORT_DEBUG_INFO_T *pDebug;

    pPort = LACP_Get_Appropriate_Physical_Port_From_ifIndex( agg_index);

    if(pPort)
    {
        pDebug = &(pPort->AggPortDebug);

        /* *pMuxReason = (UI8_T)pDebug->MuxState; */
        /* I am lazy for coding this. */
        *pCount = 0;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Get_dot3adTablesLastChanged
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to get last change of the table.
 * INPUT   : agg_index -- index of the aggregator.
 * OUTPUT  : pCount -- selected aggregator id.
 * RETURN  : TRUE  -- Found
 *           FALSE -- Not found
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Get_dot3adTablesLastChanged( UI32_T *pCount)
{

    #if 0 /* JinhuaWei, 01 August, 2008 3:55:38 */
    LACP_SYSTEM_T       *lacp_system_ptr; /*never used in this funciton, to remove warning*/
    #endif /* #if 0 */

    *pCount = lacp_last_change_time;

    return TRUE;

}
void LACP_Print_AggEntry( LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    BACKDOOR_MGR_Printf("\n\tdot3adAggIndex:                 %d", (int)agg_entry->dot3ad_agg_index);
    BACKDOOR_MGR_Printf("\n\tdot3adAggMACAddress:            %02x-%02x-%02x-%02x-%02x-%02x", agg_entry->dot3ad_agg_mac_address[0],
                                                                                agg_entry->dot3ad_agg_mac_address[1],
                                                                                agg_entry->dot3ad_agg_mac_address[2],
                                                                                agg_entry->dot3ad_agg_mac_address[3],
                                                                                agg_entry->dot3ad_agg_mac_address[4],
                                                                                agg_entry->dot3ad_agg_mac_address[5]);
    BACKDOOR_MGR_Printf("\n\tdot3adAggActorSystemPriority:   %04x", (int)agg_entry->dot3ad_agg_actor_system_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggActorSystemID:         %02x-%02x-%02x-%02x-%02x-%02x", agg_entry->dot3ad_agg_actor_system_id[0],
                                                                                agg_entry->dot3ad_agg_actor_system_id[1],
                                                                                agg_entry->dot3ad_agg_actor_system_id[2],
                                                                                agg_entry->dot3ad_agg_actor_system_id[3],
                                                                                agg_entry->dot3ad_agg_actor_system_id[4],
                                                                                agg_entry->dot3ad_agg_actor_system_id[5]);
    BACKDOOR_MGR_Printf("\n\tdot3adAggAggregateOrIndividual: %d", (int)agg_entry->dot3ad_agg_aggregate_or_individual);
    BACKDOOR_MGR_Printf("\n\tdot3adAggActorAdminKey:         %04x", (int)agg_entry->dot3ad_agg_actor_admin_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggActorOperKey:           %04x", (int)agg_entry->dot3ad_agg_actor_oper_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPartnerSystemID:       %02x-%02x-%02x-%02x-%02x-%02x", agg_entry->dot3ad_agg_partner_system_id[0],
                                                                                agg_entry->dot3ad_agg_partner_system_id[1],
                                                                                agg_entry->dot3ad_agg_partner_system_id[2],
                                                                                agg_entry->dot3ad_agg_partner_system_id[3],
                                                                                agg_entry->dot3ad_agg_partner_system_id[4],
                                                                                agg_entry->dot3ad_agg_partner_system_id[5]);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPartnerSystemPriority: %04x", (int)agg_entry->dot3ad_agg_partner_system_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPartnerOperKey:        %04x", (int)agg_entry->dot3ad_agg_partner_oper_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggCollectorMaxDelay:     %d", (int)agg_entry->dot3ad_agg_collector_max_delay);
}

void LACP_Print_PortListEntry( LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    UI32_T port;

    BACKDOOR_MGR_Printf("\nAggregator %2d Port List Entry:\n\t", (int)agg_port_list_entry->dot3ad_agg_index);
    for( port = 0; port < LACP_PORT_LIST_OCTETS; port++)
    {
        BACKDOOR_MGR_Printf("%x", agg_port_list_entry->dot3ad_agg_port_list_ports[port]);
    }
    BACKDOOR_MGR_Printf("\n");
}

void LACP_Print_PortEntry( LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortIndex:                      %d", (int)agg_port_entry->dot3ad_agg_port_index);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorSystemPriority:        %04x", (int)agg_port_entry->dot3ad_agg_port_actor_system_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorSystemID:              %02x-%02x-%02x-%02x-%02x-%02x",agg_port_entry->dot3ad_agg_port_actor_system_id[0],
                                                                                        agg_port_entry->dot3ad_agg_port_actor_system_id[1],
                                                                                        agg_port_entry->dot3ad_agg_port_actor_system_id[2],
                                                                                        agg_port_entry->dot3ad_agg_port_actor_system_id[3],
                                                                                        agg_port_entry->dot3ad_agg_port_actor_system_id[4],
                                                                                        agg_port_entry->dot3ad_agg_port_actor_system_id[5]);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorAdminKey:              %04x", (int)agg_port_entry->dot3ad_agg_port_actor_admin_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorOperKey:               %04x", (int)agg_port_entry->dot3ad_agg_port_actor_oper_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerAdminSystemPriority: %04x", (int)agg_port_entry->dot3ad_agg_port_partner_admin_system_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerOperSystemPriority:  %04x", (int)agg_port_entry->dot3ad_agg_port_partner_oper_system_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerAdminSystemID:       %02x-%02x-%02x-%02x-%02x-%02x",agg_port_entry->dot3ad_agg_port_partner_admin_system_id[0],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_admin_system_id[1],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_admin_system_id[2],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_admin_system_id[3],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_admin_system_id[4],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_admin_system_id[5]);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerOperSystemID:        %02x-%02x-%02x-%02x-%02x-%02x",agg_port_entry->dot3ad_agg_port_partner_oper_system_id[0],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_oper_system_id[1],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_oper_system_id[2],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_oper_system_id[3],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_oper_system_id[4],
                                                                                        agg_port_entry->dot3ad_agg_port_partner_oper_system_id[5]);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerAdminKey:            %04x", (int)agg_port_entry->dot3ad_agg_port_partner_admin_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerOperKey:             %04x", (int)agg_port_entry->dot3ad_agg_port_partner_oper_key);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortSelectedAggID:              %d", (int)agg_port_entry->dot3ad_agg_port_selected_agg_id);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortAttachedAggID:              %d", (int)agg_port_entry->dot3ad_agg_port_attached_agg_id);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorPort:                  %d", (int)agg_port_entry->dot3ad_agg_port_actor_port);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorPortPriority:          %04x", (int)agg_port_entry->dot3ad_agg_port_actor_port_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerAdminPort:           %d", (int)agg_port_entry->dot3ad_agg_port_partner_admin_port);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerOperPort:            %d", (int)agg_port_entry->dot3ad_agg_port_partner_oper_port);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerAdminPortPriority:   %04x", (int)agg_port_entry->dot3ad_agg_port_partner_admin_port_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerOperPortPriority:    %04x", (int)agg_port_entry->dot3ad_agg_port_partner_oper_port_priority);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorAdminState:            %1x", (int)agg_port_entry->dot3ad_agg_port_actor_admin_state);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortActorOperState:             %1x", (int)agg_port_entry->dot3ad_agg_port_actor_oper_state);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerAdminState:          %1x", (int)agg_port_entry->dot3ad_agg_port_partner_admin_state);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortPartnerOperState:           %1x", (int)agg_port_entry->dot3ad_agg_port_partner_oper_state);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortAggregateOrIndividual:      %1x", (int)agg_port_entry->dot3ad_agg_port_aggregate_or_individual);
}

void LACP_Print_PortStatsEntry( LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortIndex:                      %d", (int)agg_port_stats_entry->dot3ad_agg_port_index);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsLACPDUsRx:             %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_lac_pdus_rx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsMarkerPDUsRx:          %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_marker_pdus_rx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsMarkerResponsePDUsRx:  %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_marker_response_pdus_rx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsUnknownRx:             %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_unknown_rx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsIllegalRx:             %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_illegal_rx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsLACPDUsTx:             %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_lac_pdus_tx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsMarkerPDUsTx:          %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_marker_pdus_tx);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortStatsMarkerResponsePDUsTx:  %d", (int)agg_port_stats_entry->dot3ad_agg_port_stats_marker_response_pdus_tx);
}

void LACP_Print_PortDebugEntry( LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortIndex:                           %d", (int)agg_port_debug_entry->dot3ad_agg_port_index);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugRxState:                    ");
    switch(((UI32_T)(agg_port_debug_entry->dot3ad_agg_port_debug_rx_state)))
    {
    case Rxm_BEGIN:
        BACKDOOR_MGR_Printf("%s", "Internal Begin State...");
        break;
    case Rxm_CURRENT:
        BACKDOOR_MGR_Printf("%s", "CURRENT");
        break;
    case Rxm_EXPIRED:
        BACKDOOR_MGR_Printf("%s", "EXPIRED");
        break;
    case Rxm_DEFAULTED:
        BACKDOOR_MGR_Printf("%s", "DEFAULTED");
        break;
    case Rxm_INITIALIZE:
        BACKDOOR_MGR_Printf("%s", "INITIALIZE");
        break;
    case Rxm_LACP_DISABLED:
        BACKDOOR_MGR_Printf("%s", "LACP_DISABLED");
        break;
    case Rxm_PORT_DISABLED:
        BACKDOOR_MGR_Printf("%s", "PORT_DISABLED");
        break;
    }
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugLastRxTime:                 %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_last_rx_time);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugMuxState:                   ");
    switch(((UI32_T)(agg_port_debug_entry->dot3ad_agg_port_debug_mux_state)))
    {
    case Muxm_BEGIN:
        BACKDOOR_MGR_Printf("%s", "Internal Begin State...");
        break;
    case Muxm_DETACHED:
        BACKDOOR_MGR_Printf("%s", "DETACHED");
        break;
    case Muxm_WAITING:
        BACKDOOR_MGR_Printf("%s", "WAITING");
        break;
    case Muxm_ATTACHED:
        BACKDOOR_MGR_Printf("%s", "ATTACHED");
        break;
    case Muxm_COLLECTING:
        BACKDOOR_MGR_Printf("%s", "COLLECTING");
        break;
    case Muxm_DISTRIBUTING:
        BACKDOOR_MGR_Printf("%s", "DISTRIBUTING");
        break;
    case Muxm_COLLECTING_DISTRIBUTING:
        BACKDOOR_MGR_Printf("%s", "COLLECTING_DISTRIBUTING");
        break;
    }
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugMuxReason:                  %s", agg_port_debug_entry->dot3ad_agg_port_debug_mux_reason);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugActorChurnState:            ");
    switch(((UI32_T)(agg_port_debug_entry->dot3ad_agg_port_debug_actor_churn_state)))
    {
    case NO_CHURN:
        BACKDOOR_MGR_Printf("%s", "NO_CHURN");
        break;
    case CHURN:
        BACKDOOR_MGR_Printf("%s", "CHURN");
        break;
    case CHURN_MONITOR:
        BACKDOOR_MGR_Printf("%s", "CHURN_MONITOR");
        break;
    default:
        BACKDOOR_MGR_Printf("Unknown");
        break;
    }
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugPartnerChurnState:          ");
    switch(((UI32_T)(agg_port_debug_entry->dot3ad_agg_port_debug_partner_churn_state)))
    {
    case NO_CHURN:
        BACKDOOR_MGR_Printf("%s", "NO_CHURN");
        break;
    case CHURN:
        BACKDOOR_MGR_Printf("%s", "CHURN");
        break;
    case CHURN_MONITOR:
        BACKDOOR_MGR_Printf("%s", "CHURN_MONITOR");
        break;
    default:
        BACKDOOR_MGR_Printf("Unknown");
        break;
    }
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugActorChurnCount:            %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_actor_churn_count);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugPartnerChurnCount:          %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_partner_churn_count);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugActorSyncTransitionCount:   %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_actor_sync_transition_count);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugPartnerSyncTransitionCount: %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_partner_sync_transition_count);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugActorChangeCount:           %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_actor_change_count);
    BACKDOOR_MGR_Printf("\n\tdot3adAggPortDebugPartnerChangeCount:         %d", (int)agg_port_debug_entry->dot3ad_agg_port_debug_partner_change_count);
}

void LACP_Print_TablesLastChanged( LACP_MGR_LagMibObjects_T *lag_mib_objects)
{
    BACKDOOR_MGR_Printf("\n\tdot3adTablesLastChanged:         %d", (int)lag_mib_objects->dot3ad_tables_last_changed);
}
/*************************************************************************
 *************************************************************************
 *************************************************************************
 *************************************************************************
 *************************************************************************
 *************************************************************************
 *************************************************************************/
 /* Add for show partial information of LACP */


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_OperPortParameters
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to show oper. protocol parameters of
 *           a port.
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_OperPortParameters( UI32_T slot, UI32_T port)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pA;
    LAC_INFO_T  *pP;

    if( (slot < 1) ||
        (slot > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port < 1) ||
        (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
        BACKDOOR_MGR_Printf("\nLACP backdoor input exceed boundry");

        return;
    }


    pPort = LACP_FIND_PORT( slot, port);
    pA    = &(pPort->AggActorPort);
    pP    = &(pPort->AggPartnerPort);

    LACP_OM_EnterCriticalRegion();

    BACKDOOR_MGR_Printf("\nStatus of Slot:%2d Port:%2d", (int)slot, (int)port);

    BACKDOOR_MGR_Printf("\n  LACPEnabled          : %s", (pPort->LACPEnabled == TRUE)?("ENABLED"):("DISABLED"));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n                         Actor              Partner");
    BACKDOOR_MGR_Printf("\n  Oper. System Priority: 0x%04x             0x%04x", pA->system_priority, pP->system_priority);
    BACKDOOR_MGR_Printf("\n  Oper. System ID      : %02x-%02x-%02x-%02x-%02x-%02x  ",(int)(UI32_T)pA->system_id[0],
                                                                        (int)(UI32_T)pA->system_id[1],
                                                                        (int)(UI32_T)pA->system_id[2],
                                                                        (int)(UI32_T)pA->system_id[3],
                                                                        (int)(UI32_T)pA->system_id[4],
                                                                        (int)(UI32_T)pA->system_id[5]);
    BACKDOOR_MGR_Printf("%02x-%02x-%02x-%02x-%02x-%02x", (int)(UI32_T)pP->system_id[0],
                                            (int)(UI32_T)pP->system_id[1],
                                            (int)(UI32_T)pP->system_id[2],
                                            (int)(UI32_T)pP->system_id[3],
                                            (int)(UI32_T)pP->system_id[4],
                                            (int)(UI32_T)pP->system_id[5]);

    BACKDOOR_MGR_Printf("\n  Oper. Key            : 0x%04x             0x%04x", pA->key, pP->key);
    BACKDOOR_MGR_Printf("\n  Oper. Port Priority  : 0x%04x             0x%04x", pA->port_priority, pP->port_priority);
    BACKDOOR_MGR_Printf("\n  Oper. Port           : %02d                 %02d", (int)(pA->port_no), (int)(pP->port_no));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n      Actor Oper. State :  0x%02x          Partner Oper. State : 0x%02x", (pA->state).port_state, (pP->state).port_state);

    BACKDOOR_MGR_Printf("\n |Exp|Def|Dis|Col|Syn|Agg|Tmo|Act|   |Exp|Def|Dis|Col|Syn|Agg|Tmo|Act|");
    BACKDOOR_MGR_Printf("\n   %s   %s   %s   %s", (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Expired)?("Y"):("N"),
                                     (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Defaulted)?("Y"):("N"),
                                     (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Distributing)?("Y"):("N"),
                                     (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Collecting)?("Y"):("N"));

    BACKDOOR_MGR_Printf("   %s   %s   %s   %s", (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Synchronization)?("Y"):("N"),
                                   (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Aggregation)?("A"):("I"),
                                   (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Timeout)?("S"):("L"),
                                   (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Activity)?("A"):("P"));
    BACKDOOR_MGR_Printf("       %s   %s   %s   %s", (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Expired)?("Y"):("N"),
                                       (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Defaulted)?("Y"):("N"),
                                       (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Distributing)?("Y"):("N"),
                                       (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Collecting)?("Y"):("N"));

    BACKDOOR_MGR_Printf("   %s   %s   %s   %s", (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Synchronization)?("Y"):("N"),
                                   (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Aggregation)?("A"):("I"),
                                   (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Timeout)?("S"):("L"),
                                   (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Activity)?("A"):("P"));
    BACKDOOR_MGR_Printf("\n");

    LACP_OM_LeaveCriticalRegion();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_AdminPortParameters
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to show admin protocol parameters of
 *           a port.
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_AdminPortParameters( UI32_T slot, UI32_T port)
{
    LACP_PORT_T *pPort;
    LAC_INFO_T  *pA;
    LAC_INFO_T  *pP;

    if( (slot < 1) ||
        (slot > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port < 1) ||
        (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
        BACKDOOR_MGR_Printf("\nLACP backdoor input exceed boundry");

        return;
    }


    pPort = LACP_FIND_PORT( slot, port);
    pA    = &(pPort->AggActorAdminPort);
    pP    = &(pPort->AggPartnerAdminPort);

    LACP_OM_EnterCriticalRegion();

    BACKDOOR_MGR_Printf("\nStatus of Slot:%2d Port:%2d", (int)slot, (int)port);

    BACKDOOR_MGR_Printf("\n  LACPEnabled          : %s", (pPort->LACPEnabled == TRUE)?("ENABLED"):("DISABLED"));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n                         Actor              Partner");
    BACKDOOR_MGR_Printf("\n  Admin System Priority: 0x%04x             0x%04x", pA->system_priority, pP->system_priority);
    BACKDOOR_MGR_Printf("\n  Admin System ID      : %02x-%02x-%02x-%02x-%02x-%02x  ",(int)(UI32_T)pA->system_id[0],
                                                                        (int)(UI32_T)pA->system_id[1],
                                                                        (int)(UI32_T)pA->system_id[2],
                                                                        (int)(UI32_T)pA->system_id[3],
                                                                        (int)(UI32_T)pA->system_id[4],
                                                                        (int)(UI32_T)pA->system_id[5]);
    BACKDOOR_MGR_Printf("%02x-%02x-%02x-%02x-%02x-%02x", (int)(UI32_T)pP->system_id[0],
                                            (int)(UI32_T)pP->system_id[1],
                                            (int)(UI32_T)pP->system_id[2],
                                            (int)(UI32_T)pP->system_id[3],
                                            (int)(UI32_T)pP->system_id[4],
                                            (int)(UI32_T)pP->system_id[5]);

    BACKDOOR_MGR_Printf("\n  Admin Key            : 0x%04x             0x%04x", pA->key, pP->key);
    BACKDOOR_MGR_Printf("\n  Admin Port Priority  : 0x%04x             0x%04x", pA->port_priority, pP->port_priority);
    BACKDOOR_MGR_Printf("\n  Admin Port           : %02d                 %02d", (int)(pA->port_no), (int)(pP->port_no));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n      Actor Admin State :  0x%02x          Partner Admin State : 0x%02x", (pA->state).port_state, (pP->state).port_state);

    BACKDOOR_MGR_Printf("\n |Exp|Def|Dis|Col|Syn|Agg|Tmo|Act|   |Exp|Def|Dis|Col|Syn|Agg|Tmo|Act|");
    BACKDOOR_MGR_Printf("\n   %s   %s   %s   %s", (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Expired)?("Y"):("N"),
                                     (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Defaulted)?("Y"):("N"),
                                     (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Distributing)?("Y"):("N"),
                                     (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Collecting)?("Y"):("N"));

    BACKDOOR_MGR_Printf("   %s   %s   %s   %s", (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Synchronization)?("Y"):("N"),
                                   (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_Aggregation)?("A"):("I"),
                                   (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Timeout)?("S"):("L"),
                                   (((pA->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Activity)?("A"):("P"));
    BACKDOOR_MGR_Printf("       %s   %s   %s   %s", (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Expired)?("Y"):("N"),
                                       (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Defaulted)?("Y"):("N"),
                                       (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Distributing)?("Y"):("N"),
                                       (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Collecting)?("Y"):("N"));

    BACKDOOR_MGR_Printf("   %s   %s   %s   %s", (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Synchronization)?("Y"):("N"),
                                   (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_Aggregation)?("A"):("I"),
                                   (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Timeout)?("S"):("L"),
                                   (((pP->state).port_state)&LACP_dot3adAggPortActorOperState_LACP_Activity)?("A"):("P"));
    BACKDOOR_MGR_Printf("\n");

    LACP_OM_LeaveCriticalRegion();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_PortProtocolState
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set partner admin port state
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_PortProtocolState( UI32_T slot, UI32_T port)
{
    LACP_PORT_T            *pPort;
    LACP_PORT_DEBUG_INFO_T *p;
    LACP_PORT_TIMER_T      *pTimer;
    LACP_AGGREGATOR_T      *pAgg;

    if( (slot < 1) ||
        (slot > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port < 1) ||
        (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
        BACKDOOR_MGR_Printf("\nLACP backdoor input exceed boundry");

        return;
    }

    pPort = LACP_FIND_PORT( slot, port);
    p     = &(pPort->AggPortDebug);
    pTimer= &(pPort->port_timer);
    pAgg  = pPort->pAggregator;

    LACP_OM_EnterCriticalRegion();

    BACKDOOR_MGR_Printf("\nLACP Protocol State of Slot:%2d Port:%2d :", (int)slot, (int)port);
    BACKDOOR_MGR_Printf("\n  LACP State Machine(s):");
    BACKDOOR_MGR_Printf("\n       RxMachine:            ");
    switch(p->RxState)
    {
    case Rxm_BEGIN:
        BACKDOOR_MGR_Printf("BEGIN");
        break;
    case Rxm_CURRENT:
        BACKDOOR_MGR_Printf("CURRENT");
        break;
    case Rxm_EXPIRED:
        BACKDOOR_MGR_Printf("EXPIRED");
        break;
    case Rxm_DEFAULTED:
        BACKDOOR_MGR_Printf("DEFAULTED");
        break;
    case Rxm_INITIALIZE:
        BACKDOOR_MGR_Printf("INITIALIZE");
        break;
    case Rxm_LACP_DISABLED:
        BACKDOOR_MGR_Printf("LACP_DISABLED");
        break;
    case Rxm_PORT_DISABLED:
        BACKDOOR_MGR_Printf("PORT_DISABLED");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }
    BACKDOOR_MGR_Printf("\n       Mux State:            ");
    switch(p->MuxState)
    {
    case Muxm_BEGIN:
        BACKDOOR_MGR_Printf("BEGIN");
        break;
    case Muxm_DETACHED:
        BACKDOOR_MGR_Printf("DETACHED");
        break;
    case Muxm_WAITING:
        BACKDOOR_MGR_Printf("WAITING");
        break;
    case Muxm_ATTACHED:
        BACKDOOR_MGR_Printf("ATTACHED");
        break;
    case Muxm_COLLECTING:
        BACKDOOR_MGR_Printf("COLLECTING");
        break;
    case Muxm_DISTRIBUTING:
        BACKDOOR_MGR_Printf("DISTRIBUTING");
        break;
    case Muxm_COLLECTING_DISTRIBUTING:
        BACKDOOR_MGR_Printf("COLLECTING_DISTRIBUTING");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }
    BACKDOOR_MGR_Printf("\n       Match State:          %s", (pPort->Matched)?("True"):("False"));

    BACKDOOR_MGR_Printf("\n       Actor Churn State:    ");
    switch(p->ActorChurnState)
    {
    case NO_CHURN:
        BACKDOOR_MGR_Printf("NO CHURN");
        break;
    case CHURN:
        BACKDOOR_MGR_Printf("CHURN");
        break;
    case CHURN_MONITOR:
        BACKDOOR_MGR_Printf("CHURN MONITOR");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }

    BACKDOOR_MGR_Printf("\n       Partner Churn State:  ");
    switch(p->PartnerChurnState)
    {
    case NO_CHURN:
        BACKDOOR_MGR_Printf("NO CHURN");
        break;
    case CHURN:
        BACKDOOR_MGR_Printf("CHURN");
        break;
    case CHURN_MONITOR:
        BACKDOOR_MGR_Printf("CHURN MONITOR");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }

    BACKDOOR_MGR_Printf("\n  LACP Control Variables:");
    BACKDOOR_MGR_Printf("\n       Selected:             ");
    switch(pPort->Selected)
    {
    case UNSELECTED:
        BACKDOOR_MGR_Printf("UNSELECTED");
        break;
    case STANDBY:
        BACKDOOR_MGR_Printf("STANDBY");
        break;
    case SELECTED:
        BACKDOOR_MGR_Printf("SELECTED");
        break;
    default:
        BACKDOOR_MGR_Printf("UNKNOWN -- SHOULD_NOT_HAPPEN");
        break;
    }

    BACKDOOR_MGR_Printf("\n       PortOperStatus:       %s", (pPort->Port_OperStatus)?("True"):("False"));
    BACKDOOR_MGR_Printf("\n       PortEnabled:          %s", (pPort->PortEnabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n       LinkUp:               %s", (pPort->LinkUp)?("Up"):("Down"));

    BACKDOOR_MGR_Printf("\n       LACPOperStatus:       %s", (pPort->LACP_OperStatus)?("True"):("False"));
    BACKDOOR_MGR_Printf("\n       LACPEnabled:          %s", (pPort->LACPEnabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n       Full Duplex           %s", (pPort->FullDuplexMode)?("Full Duplex"):("Half Duplex"));

    BACKDOOR_MGR_Printf("\n       NTT:                  %d", (int)(pPort->NTT));
    BACKDOOR_MGR_Printf("\n       Port Speed            %d", (int)(pPort->PortSpeed));

    if(pAgg)
    {
        BACKDOOR_MGR_Printf("\n       AggID                 %d", (int)(pAgg->AggID));
        BACKDOOR_MGR_Printf("\n       Index in Agg          %d", (int)(pPort->port_index));
    }
    else
    {
        BACKDOOR_MGR_Printf("\n       Not Attach to Agg");
    }


    BACKDOOR_MGR_Printf("\n  LACP Counters:");
    BACKDOOR_MGR_Printf("\n       Actor Churn Count:    %d", (int)(p->ActorChurnCount));
    BACKDOOR_MGR_Printf("\n       Partner Churn Count:  %d", (int)(p->PartnerChurnCount));
    BACKDOOR_MGR_Printf("\n       Actor Sync Count:     %d", (int)(p->ActorSyncTransitionCount));
    BACKDOOR_MGR_Printf("\n       Partner Sync Count:   %d", (int)(p->PartnerSyncTransitionCount));
    BACKDOOR_MGR_Printf("\n       Actor Change Count:   %d", (int)(p->ActorChangeCount));
    BACKDOOR_MGR_Printf("\n       Partner Change Count: %d", (int)(p->PartnerChangeCount));


    BACKDOOR_MGR_Printf("\n  LACP Timers:");

    BACKDOOR_MGR_Printf("\n       CurrentWhile Timer:   ");
    BACKDOOR_MGR_Printf("\n                             %s", (pTimer->current_while_timer.enabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n                             %d tick(s)", (int)(pTimer->current_while_timer.tick));

    BACKDOOR_MGR_Printf("\n       Periodic Timer:");
    BACKDOOR_MGR_Printf("\n                             %s", (pTimer->periodic_timer.enabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n                             %d tick(s)", (int)(pTimer->periodic_timer.tick));

    BACKDOOR_MGR_Printf("\n       WaitWhile Timer:      ");
    BACKDOOR_MGR_Printf("\n                             %s", (pTimer->wait_while_timer.enabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n                             %d tick(s)", (int)(pTimer->wait_while_timer.tick));

    BACKDOOR_MGR_Printf("\n       ActorChurn Timer:     ");
    BACKDOOR_MGR_Printf("\n                             %s", (pTimer->actor_churn_timer.enabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n                             %d tick(s)", (int)(pTimer->actor_churn_timer.tick));

    BACKDOOR_MGR_Printf("\n       PartnerChurn Timer:   ");
    BACKDOOR_MGR_Printf("\n                             %s", (pTimer->partner_churn_timer.enabled)?("Enabled"):("Disabled"));
    BACKDOOR_MGR_Printf("\n                             %d tick(s)", (int)(pTimer->partner_churn_timer.tick));

    LACP_OM_LeaveCriticalRegion();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_PortStatistics
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set partner admin port state
 * INPUT   : slot  -- slot number
 *           port  -- port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_PortStatistics( UI32_T slot, UI32_T port)
{
    LACP_PORT_T            *pPort;
    LACP_PORT_STATISTICS_T *p;

    if( (slot < 1) ||
        (slot > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port < 1) ||
        (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
        BACKDOOR_MGR_Printf("\nLACP backdoor input exceed boundry");

        return;
    }

    pPort = LACP_FIND_PORT( slot, port);
    p     = &(pPort->AggPortStats);

    LACP_OM_EnterCriticalRegion();

    BACKDOOR_MGR_Printf("\n  LACP Statistics of Slot:%2d Port:%2d :", (int)slot, (int)port);

    BACKDOOR_MGR_Printf("\n       AnnounceReceive LACPDUs:       %d", (int)(p->AnnouncePDUsRx));
    BACKDOOR_MGR_Printf("\n");

    BACKDOOR_MGR_Printf("\n       Receive LACPDUs:               %d", (int)(p->LACPDUsRx));
    BACKDOOR_MGR_Printf("\n       Receive MarkerPDUs:            %d", (int)(p->MarkerPDUsRx));
    BACKDOOR_MGR_Printf("\n       Receive Marker Response PDUs:  %d", (int)(p->MarkerResponsePDUsRx));
    BACKDOOR_MGR_Printf("\n       Receive Unknown PDUs:          %d", (int)(p->UnknownRx));
    BACKDOOR_MGR_Printf("\n       Receive Illegal PDUs:          %d", (int)(p->IllegalRx));
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n       Transmit LACPDUs:              %d", (int)(p->LACPDUsTx));
    BACKDOOR_MGR_Printf("\n       Transmit MarkerPDUs:           %d", (int)(p->MarkerPDUsTx));
    BACKDOOR_MGR_Printf("\n       Transmit Marker Response PDUs: %d", (int)(p->MarkerResponsePDUsTx));

    LACP_OM_LeaveCriticalRegion();

}

void LACP_Reset_All_PortStatistics(void)
{
    UI32_T slot, port;

    for( slot = 1; slot <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; slot++)
    {
        for( port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            LACP_Reset_PortStatistics( slot, port);
        }
    }
}


void LACP_Reset_PortStatistics( UI32_T slot, UI32_T port)
{
    LACP_PORT_T            *pPort;
    LACP_PORT_STATISTICS_T *p;

    if( (slot < 1) ||
        (slot > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port < 1) ||
        (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
        return;
    }

    pPort = LACP_FIND_PORT( slot, port);
    p     = &(pPort->AggPortStats);

    p->LACPDUsRx            = 0;
    p->MarkerPDUsRx         = 0;
    p->MarkerResponsePDUsRx = 0;
    p->UnknownRx            = 0;
    p->IllegalRx            = 0;
    p->LACPDUsTx            = 0;
    p->MarkerPDUsTx         = 0;
    p->MarkerResponsePDUsTx = 0;
    p->AnnouncePDUsRx       = 0;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Show_All_Ports_In_Agg
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to show all ports in the aggregator.
 * INPUT   : slot : slot
 *           port : port
 * OUTPUT  : None
 * RETURN  : pointer to th port.
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Show_All_Ports_In_Agg( UI32_T slot, UI32_T port)
{
    LACP_AGGREGATOR_T *pAgg;
    LACP_PORT_T       *p;
    LACP_PORT_T       *port_ptr;

    if( (slot < 1) ||
        (slot > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port < 1) ||
        (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
        return;
    }

#if 0   /* Allenc */
    pAgg = LACP_FIND_AGGREGATOR( slot, port);
#endif  /* Allenc */
    port_ptr    = LACP_FIND_PORT(slot, port);
    pAgg    = LACP_UTIL_AllocateAggregator(port_ptr);
    /* Iterate all ports in aggregator. */
    if(!pAgg)
    {
        return;
    }

    LACP_OM_EnterCriticalRegion();

    p = pAgg->pLACP_Ports;
    BACKDOOR_MGR_Printf("\nAgg Id:%d owns", (int)pAgg->AggID);

    while(p)
    {
        BACKDOOR_MGR_Printf("\nslot:%d port:%d", (int)p->slot, (int)p->port);
        p = p->pNext;
    }

    LACP_OM_LeaveCriticalRegion();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Print_Fail_Message
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to print fail message to add to trunk.
 * INPUT   : err_msg: error message
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_Print_Fail_Message( UI32_T err_msg)
{
    switch(err_msg)
    {
    case TRK_MGR_SUCCESS:
        BACKDOOR_MGR_Printf("\nTRK_MGR_SUCCESS");
        break;
    case TRK_MGR_ERROR_TRUNK:
        BACKDOOR_MGR_Printf("\nTRK_MGR_ERROR_TRUNK");
        break;
    case TRK_MGR_ERROR_MEMBER:
        BACKDOOR_MGR_Printf("\nTRK_MGR_ERROR_MEMBER");
        break;
    case TRK_MGR_MEMBER_TOO_MANY:
        BACKDOOR_MGR_Printf("\nTRK_MGR_MEMBER_TOO_MANY");
        break;
    case TRK_MGR_DIFFERENT_PORT_TYPE:
        BACKDOOR_MGR_Printf("\nTRK_MGR_DIFFERENT_PORT_TYPE");
        break;
    case TRK_MGR_NOT_IN_THE_SAME_DEVICE:
        BACKDOOR_MGR_Printf("\nTRK_MGR_NOT_IN_THE_SAME_DEVICE");
        break;
    case TRK_MGR_ADMIN_INCONSIST:
        BACKDOOR_MGR_Printf("\nTRK_MGR_ADMIN_INCONSIST");
        break;
    case TRK_MGR_STATE_INCONSIST:
        BACKDOOR_MGR_Printf("\nTRK_MGR_STATE_INCONSIST: autoneg_capability, flow_control_cfg or speed_duplex_cfg");
        break;
    case TRK_MGR_NOT_ON_THE_SAME_VLAN:
        BACKDOOR_MGR_Printf("\nTRK_MGR_NOT_ON_THE_SAME_VLAN");
        break;
    case TRK_MGR_NOT_ON_THE_SAME_PRIVATE_VLAN:
        BACKDOOR_MGR_Printf("\nTRK_MGR_NOT_ON_THE_SAME_PRIVATE_VLAN");
        break;
    case TRK_MGR_ANALYZER_PORT_ERROR:
        BACKDOOR_MGR_Printf("\nTRK_MGR_ANALYZER_PORT_ERROR");
        break;
    case TRK_MGR_DEV_INTERNAL_ERROR:
        BACKDOOR_MGR_Printf("\nTRK_MGR_DEV_INTERNAL_ERROR");
        break;
    default:
        BACKDOOR_MGR_Printf("\nUnknown Error Code");
        break;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_Migrate_Ports
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to migrate ports to appropiate Aggregator.
 * INPUT   : pPort -- port to be migrate
 * OUTPUT  : None
 * RETURN  : TRUE  -- SUCCESS
 *           FALSE -- FAIL
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T LACP_Migrate_Port( LACP_PORT_T *pPort)
{
    LACP_AGGREGATOR_T *pOldAgg, *pNewAgg;

    if(!pPort)
    {
        return FALSE;
    }

    if(!(pPort->pAggregator))
    {
        return FALSE;
    }

#if 0   /* Allenc */
    pAggPort = LACP_Find_AggregatorPort( pPort);

    if(!pAggPort)
    {
        return FALSE;
    }

    pOldAgg = pPort->pAggregator;
    pNewAgg = LACP_FIND_AGGREGATOR( pAggPort->slot, pAggPort->port);

    if (!pOldAgg || !pNewAgg)
        return FALSE;
#endif  /* Allenc */

    pNewAgg = LACP_UTIL_AllocateAggregator(pPort);

    if (!pNewAgg)
        return FALSE;

    pOldAgg = pPort->pAggregator;

    if (pOldAgg)
    {
        if(0 == LACP_Remove_From_Aggregator( pOldAgg, pPort))
        {
            return FALSE;
        }

        if(pOldAgg->NumOfPortsInAgg > 0)
        {
            (pOldAgg->NumOfPortsInAgg)--;
        }
    }

    if(LACP_Add_To_Aggregator( pNewAgg, pPort))
    {
        (pNewAgg->NumOfPortsInAgg)++;
        return TRUE;
    }

    return FALSE;

}


/*===========================================================================*/
/* Functions for setting MIB entry                                           */
/*===========================================================================*/


/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SetDot3adAggActorAdminKey
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set related aggregator admin key
 * INPUT   : agg_id    -- index of the aggregator.
 *           admin_key  -- administration key of actor.
 *           is_default -- whether it is to set default value
 * OUTPUT  : None
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : 1. The admin key which is set to the aggregator is set to all
 *              the trunk member
 *           2. The static_admin_key flag is disabled if is_default is TRUE,
 *              else enabled.
 *------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_SetDot3adAggActorAdminKey(UI32_T agg_id, UI16_T admin_key, BOOL_T is_default)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;
    LACP_PORT_T         *port_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
        UI16_T aggAdminKeyOld = aggregator_ptr->AggActorAdminKey;
#endif
        aggregator_ptr->AggActorOperKey     = aggregator_ptr->AggActorAdminKey
                                            = admin_key;

        if (is_default == TRUE)
        {
            aggregator_ptr->static_admin_key    = FALSE;
        }
        else
        {
            aggregator_ptr->static_admin_key    = TRUE;
        }

        port_ptr    = aggregator_ptr->pLACP_Ports;
        while (port_ptr)
        {
            UI32_T  port_ifindex;
            if (SWCTRL_UserPortToIfindex(port_ptr->slot, port_ptr->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                LACP_UTIL_SetDot3adAggPortActorAdminKey(port_ifindex, admin_key);
                port_ptr    = port_ptr->pNext;
            }
        }
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
        if(aggAdminKeyOld !=0)
        {
            LACP_UTIL_SetStaticLacpDot3adAggPortActorAdminKey(aggAdminKeyOld, admin_key);
        }
#endif
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    else
    {
        return  FALSE;
    }
}/* LACP_UTIL_SetDot3adAggActorAdminKey */



BOOL_T LACP_UTIL_SetDot3adAggActorSystemPriority( UI32_T agg_id, UI16_T priority)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        LACP_PORT_T *port_ptr;

        aggregator_ptr->AggActorSystemPriority = priority;
        port_ptr    = aggregator_ptr->pLACP_Ports;
        while (port_ptr)
        {
            UI32_T  port_ifindex;
            if (SWCTRL_UserPortToIfindex(port_ptr->slot, port_ptr->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                LACP_UTIL_SetDot3adAggPortActorSystemPriority(port_ifindex, priority);
                port_ptr    = port_ptr->pNext;
            }
        }
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
     return TRUE;
    }
    return  FALSE;

}/* */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T priority)
{
    LACP_PORT_T *port_ptr;
    LAC_INFO_T  *admin_info_ptr, *oper_info_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        admin_info_ptr  = &(port_ptr->AggActorAdminPort);
        oper_info_ptr   = &(port_ptr->AggActorPort);
        admin_info_ptr->system_priority = oper_info_ptr->system_priority
                                        = priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;
}/* LACP_UTIL_SetDot3adAggPortActorSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T admin_key)
{
    LACP_PORT_T *port_ptr;
    LAC_INFO_T  *admin_info_ptr, *oper_info_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        admin_info_ptr              = &(port_ptr->AggActorAdminPort);
        oper_info_ptr               = &(port_ptr->AggActorPort);
        admin_info_ptr->key         = oper_info_ptr->key
                                    = admin_key;
        port_ptr->static_admin_key  = TRUE;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;

}/* LACP_UTIL_SetDot3adAggPortActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_ClearStaticAdminKeyFlag
 * ------------------------------------------------------------------------
 * PURPOSE  :   Clear the static admin key flag
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
void    LACP_UTIL_ClearStaticAdminKeyFlag(UI32_T port_index)
{
    LACP_PORT_T *port_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        port_ptr->static_admin_key  = FALSE;
    }

    return;
} /* End of LACP_UTIL_ClearStaticAdminKeyFlag */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T priority)
{
    LACP_PORT_T *port_ptr;
    LAC_INFO_T  *admin_info_ptr, *oper_info_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        admin_info_ptr  = &(port_ptr->AggActorAdminPort);
        oper_info_ptr   = &(port_ptr->AggActorPort);

        admin_info_ptr->port_priority   = oper_info_ptr->port_priority
                                        = priority;
        LACP_UTIL_SortAggregatorMembers(port_ptr->pAggregator);
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;

}/* LACP_UTIL_SetDot3adAggPortActorPortPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T priority)
{
    LACP_PORT_T *port_ptr;
    LAC_INFO_T  *admin_info_ptr, *oper_info_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        admin_info_ptr  = &(port_ptr->AggPartnerAdminPort);
        oper_info_ptr   = &(port_ptr->AggPartnerPort);
        admin_info_ptr->system_priority = oper_info_ptr->system_priority
                                        = priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;

}/* LACP_UTIL_SetDot3adAggPortPartnerAdminSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T admin_key)
{
    LACP_PORT_T *port_ptr;
    LAC_INFO_T  *admin_info_ptr, *oper_info_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        admin_info_ptr  = &(port_ptr->AggPartnerAdminPort);
        oper_info_ptr   = &(port_ptr->AggPartnerPort);
        admin_info_ptr->key     = oper_info_ptr->key
                                = admin_key;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }

    return FALSE;


}/* LACP_UTIL_SetDot3adAggPortPartnerAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- Collecting/Distributing state
 *              FALSE -- not in Collecting/Distributing state
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  LACP_UTIL_SetDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T priority)
{
    LACP_PORT_T *port_ptr;
    LAC_INFO_T  *admin_info_ptr, *oper_info_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex( port_index);

    if(port_ptr)
    {
        admin_info_ptr = &(port_ptr->AggPartnerAdminPort);
        oper_info_ptr = &(port_ptr->AggPartnerPort);
        admin_info_ptr->port_priority = oper_info_ptr->port_priority
                                      = priority;
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();

        return TRUE;
    }

    return FALSE;

}/* LACP_UTIL_SetDot3adAggPortPartnerAdminPortPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_SetDot3AggPortActorLacp_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   the periodic time out value. short time out or long time out
 * INPUT    :   UI32_T  port_index  -- the port ifindex
 *              UI32_T  timeout   -- the long or short timeout value
 * OUTPUT   :   None
 * RETURN   :   TRUE  -- set success
 *              FALSE -- set fail
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_UTIL_SetDot3AggPortActorLacp_Timeout(UI32_T port_index, UI32_T timeout)
{
    LACP_PORT_T *port_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex(port_index);

    if(port_ptr)
    {
       switch (timeout)
       {
           case LACP_LONG_TIMEOUT:
               LACP_TYPE_SET_PORT_STATE_BIT ( ((port_ptr->AggActorAdminPort).state).port_state,FALSE,LACP_dot3adAggPortActorOperState_LACP_Timeout);
               LACP_TYPE_SET_PORT_STATE_BIT ( ((port_ptr->AggActorPort).state).port_state,FALSE,LACP_dot3adAggPortActorOperState_LACP_Timeout);
               return TRUE;
               break;
           case LACP_SHORT_TIMEOUT:
               LACP_TYPE_SET_PORT_STATE_BIT ( ( (port_ptr->AggActorAdminPort).state).port_state,TRUE,LACP_dot3adAggPortActorOperState_LACP_Timeout);
               LACP_TYPE_SET_PORT_STATE_BIT ( ( (port_ptr->AggActorPort).state).port_state,TRUE,LACP_dot3adAggPortActorOperState_LACP_Timeout);
               return TRUE;
               break;
       }
    }

    return FALSE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_UTIL_GetDot3AggPortActorLacp_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the periodic time value.
 * INPUT    :   UI32_T  port_index  -- the port ifindex
 *              UI16_T  *timeout    -- the output field
 * OUTPUT   :   *timeout
 * RETURN   :   TRUE  -- get success
 *              FALSE -- get fail
 * NOTE     :   None
 * REF      :
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_UTIL_GetDot3AggPortActorLacp_Timeout(UI32_T port_index, UI32_T *timeout)
{
    LACP_PORT_T *port_ptr;

    port_ptr = LACP_Get_Appropriate_Physical_Port_From_ifIndex(port_index);

    if(port_ptr)
    {
        *timeout=  LACP_TYPE_GET_PORT_STATE_BIT(((port_ptr->AggActorPort).state).port_state,LACP_dot3adAggPortActorOperState_LACP_Timeout);
        return TRUE;
    }

    return FALSE;
}

BOOL_T LACP_UTIL_SetDot3AggLastChange_Time()
{
    LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
    return TRUE;;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SetDot3adAggActorTimeout
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to set related aggregator timeout
 * INPUT   : agg_id    -- index of the aggregator.
 * OUTPUT  : timeout -- timeout of actor.
 * RETURN  : TRUE  -- Collecting/Distributing state
 *           FALSE -- not in Collecting/Distributing state
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_SetDot3adAggActorTimeout( UI32_T agg_id, UI32_T timeout)
{
    LACP_SYSTEM_T       *lacp_system_ptr;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    lacp_system_ptr = LACP_OM_GetSystemPtr();
    aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
    if (aggregator_ptr)
    {
        LACP_PORT_T *port_ptr;
        LACP_PORT_T *pPort;
        aggregator_ptr->AggActorTimeout = timeout;
        port_ptr    = aggregator_ptr->pLACP_Ports;
        while (port_ptr)
        {
            UI32_T  port_ifindex;
            if (SWCTRL_UserPortToIfindex(port_ptr->slot, port_ptr->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                LACP_UTIL_SetDot3AggPortActorLacp_Timeout(port_ifindex, timeout);
                pPort = LACP_OM_GetPortPtr(port_ifindex);
                if(pPort)
                    LACP_Periodic_Machine( pPort, LACP_TIMER_EV);
            }
            port_ptr    = port_ptr->pNext;
        }
        LACP_UPDATE_SYSTEM_LAST_CHANGE_TIME();
        return TRUE;
    }
    return  FALSE;

}

#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_AddStaticLacpTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a static LACP trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *               pPort     -- which port to add
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/

BOOL_T LACP_UTIL_AddStaticLacpTrunkMember(UI32_T trunk_id, LACP_PORT_T *pPort)
{
    BOOL_T  is_static;
    UI32_T  trk_mgr_err_code;
    LACP_SYSTEM_T     *lacp_system_ptr;

    if(FALSE == LACP_Is_Empty_Trunk( trunk_id))
    {
        if ((FALSE == TRK_PMGR_IsDynamicTrunkId( trunk_id))
            &&  (TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
           )
        {
            return FALSE;
        }
        else
        {
            UI32_T  port_ifindex;
            if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            {
                trk_mgr_err_code    = TRK_MGR_ERROR_TRUNK;
                return FALSE;
            }
            else
            {
                trk_mgr_err_code    = TRK_PMGR_AddDynamicTrunkMember( trunk_id, port_ifindex, FALSE);
            }
            if(trk_mgr_err_code != TRK_MGR_SUCCESS)
            {
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nFail to TRK_PMGR_AddDynamicTrunkMember id=%d slot:%d port:%d ifindex:%d", (int)trunk_id, (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                    LACP_Print_Fail_Message( trk_mgr_err_code);
                }
                DEBUG_STATE_MACHING("%sFail to TRK_MGR_AddDynamicTrunkMember id=%lu slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (unsigned long)trunk_id, (unsigned long)pPort->slot, (long)pPort->port, (unsigned long)port_ifindex);
                return FALSE;
            }
            else
            {
                pPort->Is_static_lacp = TRUE;
                lacp_system_ptr = LACP_OM_GetSystemPtr();
                SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nLACP_UTIL_AddStaticLacpTrunkMember Add Trunk id:%d member slot:%d port:%d ifindex:%d", (int)trunk_id, (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                }
                DEBUG_STATE_MACHING("%sLACP_UTIL_AddStaticLacpTrunkMember Add Trunk id:%lu member slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (unsigned long)trunk_id, (unsigned long)pPort->slot, (unsigned long)pPort->port, (unsigned long)port_ifindex);
                return TRUE;
            }
        }
    }
    else
    {
        if (    TRK_PMGR_IsTrunkExist(trunk_id, &is_static)
            ||  TRK_PMGR_CreateDynamicTrunk(trunk_id)
           )
        {
            UI32_T  port_ifindex;
            if(bLACP_DbgAthAL)
            {
                BACKDOOR_MGR_Printf("\nLACP_UTIL_AddStaticLacpTrunkMember Allocate Trunk id:%d", (int)trunk_id);
            }
            DEBUG_STATE_MACHING("%sLACP_UTIL_AddStaticLacpTrunkMember Allocate Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)trunk_id);

            if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            {
                trk_mgr_err_code    = TRK_MGR_ERROR_TRUNK;
                return FALSE;
            }
            else
            {
                trk_mgr_err_code    = TRK_PMGR_AddDynamicTrunkMember(trunk_id, port_ifindex, FALSE);
            }
            if(trk_mgr_err_code != TRK_MGR_SUCCESS)
            {
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nLACP_UTIL_AddStaticLacpTrunkMember Fail to add trunk member Trunk id:%d slot:%d port:%d ifindex:%d", (int)trunk_id, (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                    LACP_Print_Fail_Message( trk_mgr_err_code);
                }
                DEBUG_STATE_MACHING("%sLACP_UTIL_AddStaticLacpTrunkMember Fail to add trunk member Trunk id:%lu slot:%lu port:%lu ifindex:%lu\r\n", DEBUG_HEADER(), (unsigned long)trunk_id, (unsigned long)pPort->slot, (unsigned long)pPort->port, (unsigned long)port_ifindex);

                TRK_PMGR_FreeTrunkIdDestroyDynamic(trunk_id);
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nLACP_UTIL_AddStaticLacpTrunkMember Free Trunk id:%d", (int)trunk_id);
                }
                DEBUG_STATE_MACHING("%sLACP_UTIL_AddStaticLacpTrunkMember Free Trunk id:%lu\r\n", DEBUG_HEADER(), (unsigned long)trunk_id);
                return FALSE;
            }
            else
            {
                pPort->Is_static_lacp = TRUE;
                lacp_system_ptr = LACP_OM_GetSystemPtr();
                SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));
                return TRUE;
            }
        }
        else
        {
            return FALSE;
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_DeleteStaticLacpTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a static LACP trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *               pPort     -- which port to delete
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_DeleteStaticLacpTrunkMember(UI32_T trunk_id, LACP_PORT_T *pPort)
{
    LACP_SYSTEM_T *lacp_system_ptr;

    if(TRUE == LACP_Is_Trunk_Member_Set( pPort->slot, pPort->port, &trunk_id))
    {
        if(TRUE == TRK_PMGR_IsDynamicTrunkId(trunk_id))
        {
            UI32_T  port_ifindex;
            if (SWCTRL_UserPortToIfindex(pPort->slot, pPort->port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (TRK_PMGR_DeleteDynamicTrunkMember(trunk_id, port_ifindex))
                {

                    if(bLACP_DbgAthAL)
                    {
                        BACKDOOR_MGR_Printf("\nLACP_UTIL_DeleteStaticLacpTrunkMember Delete Trunk id:%d member slot:%d port:%d", (int)trunk_id, (int)pPort->slot, (int)pPort->port);
                    }
                    pPort->Is_static_lacp = FALSE;
                    if (TRK_PMGR_GetTrunkMemberCounts(trunk_id)==0)
                    {
                        TRK_PMGR_FreeTrunkIdDestroyDynamic(trunk_id);
                        if(bLACP_DbgAthAL)
                        {
                            BACKDOOR_MGR_Printf("\nLACP_UTIL_DeleteStaticLacpTrunkMember Delete Trunk id:%d", (int)trunk_id);
                        }
                    }

                    lacp_system_ptr = LACP_OM_GetSystemPtr();
                    SYS_TIME_GetRealTimeBySec( &(lacp_system_ptr->last_changed));

                    return TRUE;
                }

            }
            else
            {
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nLACP_UTIL_DeleteStaticLacpTrunkMember :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)pPort->slot, (int)pPort->port, (int)port_ifindex);
                }
                return FALSE;
            }
        }
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckAggKeyIsMatchPortKeyAndAddStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check LAG admin key is matched with  port admin key,
                     then add a member to a static LACP trunking port
 * INPUT   : trunk_id -- trunk_id
 *               admin_key     -- LAG admin key
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckAggKeyIsMatchPortKeyAndAddStaticTrunkMember(UI32_T  trunk_id, UI16_T admin_key)
{
    LACP_PORT_T *port_ptr = NULL;
    UI32_T slot,port;
    UI32_T TrkId;

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
                continue;
            }

            port_ptr = LACP_FIND_PORT( slot, port);
            if (port_ptr
                && (port_ptr->AggActorAdminPort.key == admin_key)
                && (port_ptr->LACP_OperStatus == TRUE)
               )
            {
                if(port_ptr->Is_static_lacp == TRUE)
                {
                    continue;
                }
                if(FALSE == LACP_Is_Trunk_Member_Set( port_ptr->slot, port_ptr->port, &TrkId))
                {
                    if(port_ptr->static_admin_key == TRUE)
                    {
                        LACP_UTIL_AddStaticLacpTrunkMember(trunk_id, port_ptr);
                    }

                }
                else
                {
                    if(TrkId == trunk_id && port_ptr->Port_OperStatus == TRUE)
                    {
                        /*it does not need add port to trunk when link is up*/
                        port_ptr->Is_static_lacp = TRUE;
                    }
                }
            }
        }
    }
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckAggKeyIsMatchPortKeyAndDeleteStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check LAG admin key is matched with  port admin key, then delete a member from a static LACP trunking port
 * INPUT   : trunk_id -- trunk_id
 *               admin_key     -- LAG admin key
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckAggKeyIsMatchPortKeyAndDeleteStaticTrunkMember(UI32_T  trunk_id, UI16_T admin_key)
{
    LACP_PORT_T *port_ptr = NULL;
    UI32_T slot,port;
    UI32_T TrkId;
    BOOL_T is_CollectingDistributing = FALSE;

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
                continue;
            }

            port_ptr = LACP_FIND_PORT( slot, port);
            if (port_ptr
                && (port_ptr->AggActorAdminPort.key == admin_key)
                && (port_ptr->LACP_OperStatus == TRUE))
            {
                if((TRUE == LACP_Is_Trunk_Member_Set( port_ptr->slot, port_ptr->port, &TrkId))
                    && (TrkId == trunk_id))
                {
                    is_CollectingDistributing = (LACP_TYPE_GET_PORT_STATE_BIT(((port_ptr->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Collecting)
                                                 && LACP_TYPE_GET_PORT_STATE_BIT(((port_ptr->AggActorPort).state).port_state, LACP_dot3adAggPortActorOperState_Distributing));
                    if(port_ptr->Port_OperStatus == FALSE
                        || (port_ptr->Port_OperStatus == TRUE && FALSE == is_CollectingDistributing))
                    {
                        LACP_UTIL_DeleteStaticLacpTrunkMember(trunk_id, port_ptr);
                    }
                    if(TRUE == is_CollectingDistributing)
                    {
                        /*it can not delete member from trunk when link is up*/
                        port_ptr->Is_static_lacp = FALSE;
                    }
                }
            }
        }
    }
 }

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckPortKeyIsMatchAggKeyAndAddStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check port admin key is matched with LAG admin key,
                     then add a member to a static LACP trunking port
 * INPUT   : port_ptr -- port
 *               admin_key     -- port admin key
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckPortKeyIsMatchAggKeyAndAddStaticTrunkMember(LACP_PORT_T *port_ptr, UI16_T admin_key)
{
    UI32_T agg_index;
    UI32_T trunk_id;
    UI32_T TrkId;
    LACP_SYSTEM_T *lacp_system_ptr = LACP_OM_GetSystemPtr();

    if(admin_key == 0
        || port_ptr->static_admin_key == FALSE
        || port_ptr->Is_static_lacp == TRUE
      )
    {
        return;
    }

    for (agg_index = 0; agg_index < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; agg_index++)
    {
        trunk_id = agg_index + 1;
        if (lacp_system_ptr->aggregator[agg_index].AggActorAdminKey == admin_key)
        {
            /*it also need add port to trunk when link is up but lacp handshake fail*/
            LACP_UTIL_AddStaticLacpTrunkMember(trunk_id, port_ptr);
            return;
        }
    }

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckPortKeyIsMatchAggKeyAndDeleteStaticTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check port admin key is matched with  LAG admin key,
                     then delete a member from a static LACP trunking port
 * INPUT   : port_ptr -- port
 *               admin_key     -- port admin key
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void LACP_UTIL_CheckPortKeyIsMatchAggKeyAndDeleteStaticTrunkMember(LACP_PORT_T *port_ptr, UI16_T admin_key)
{
    UI32_T agg_index;
    UI32_T trunk_id;
    UI32_T TrkId;
    LACP_SYSTEM_T *lacp_system_ptr = LACP_OM_GetSystemPtr();

    if(admin_key == 0)
    {
        return;
    }

    for (agg_index = 0; agg_index < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; agg_index++)
    {
        trunk_id = agg_index + 1;
        if (lacp_system_ptr->aggregator[agg_index].AggActorAdminKey == admin_key
            || (/*detach member which is added to static lacp trunk after handshake success*/
                port_ptr->Is_static_lacp == TRUE
                && admin_key == 1
                && port_ptr->Port_OperStatus == FALSE
                && port_ptr->static_admin_key == FALSE
                )
           )
        {
            if((TRUE == LACP_Is_Trunk_Member_Set( port_ptr->slot, port_ptr->port, &TrkId))
                && (TrkId == trunk_id))
            {
                if(TRUE == LACP_UTIL_DeleteStaticLacpTrunkMember(trunk_id, port_ptr))
                {
                    LACP_UTIL_CheckAggKeyIsMatchPortKeyAndAddStaticTrunkMember(trunk_id, admin_key);
                }
                return;
            }
        }
    }
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_CheckAggKeyIsSameWithOtherAgg
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check LAG admin key is same with other LAG
 * INPUT   : trunk_id -- trunk_id
 *               admin_key     -- LAG admin key
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T LACP_UTIL_CheckAggKeyIsSameWithOtherAgg(UI32_T agg_id, UI16_T admin_key)
{
    UI32_T agg_index;
    LACP_SYSTEM_T *lacp_system_ptr = LACP_OM_GetSystemPtr();

    for (agg_index = 0; agg_index < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; agg_index++)
    {
        if(agg_index == agg_id)
        {
            continue;

        }
        if((lacp_system_ptr->aggregator[agg_index].AggActorAdminKey == admin_key)
            && (admin_key != 0))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LACP_UTIL_SetStaticLacpDot3adAggPortActorAdminKey
 * -------------------------------------------------------------------------
 * FUNCTION: This function  uses LAG admin to set port admin key for static LACP trunk member
 * INPUT   : old_admin_key --old  LAG admin key
 *               admin_key     -- LAG admin key
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void LACP_UTIL_SetStaticLacpDot3adAggPortActorAdminKey(UI16_T old_admin_key, UI16_T admin_key)
{
    LACP_PORT_T *port_ptr = NULL;
    UI32_T slot,port;
    UI32_T port_ifindex;
    UI32_T trunk_id;

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
                continue;
            }

            port_ptr = LACP_FIND_PORT( slot, port);
            if (port_ptr
                && (port_ptr->AggActorAdminPort.key == old_admin_key)
                && (port_ptr->LACP_OperStatus == TRUE)
                && (TRUE == LACP_Is_Trunk_Member_Set(slot, port, &trunk_id)))
            {
                SWCTRL_UserPortToIfindex(slot, port, &port_ifindex);
                LACP_UTIL_SetDot3adAggPortActorAdminKey(port_ifindex, admin_key);
            }
        }
    }
}

#endif
