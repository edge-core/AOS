/*-----------------------------------------------------------------------------
 * Module Name: lacp_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definition for the LACP object and the access functions.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/07/2001 - Lewis Kang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_bld.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "stktplg_pom.h"
#include "lacp_type.h"
#include "lacp_om.h"
#include "lacp_om_private.h"
#include "lacp_util.h"

/* LACP state machine needs */
static LACP_SYSTEM_T     lacp_system;
static LACP_PORT_T       lacp_port[MAX_PORTS];

/* Protected Data */
//static LACP_TaskData_T         LACP_TaskManagement;
static LACP_EE_SYSTEM_DATA_T   LACP_EESystemData;
static LACP_EE_PORT_DATA_T     LACP_EEPortData;

static UI8_T LACP_SlowProtocolMCAddr[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};

/* moved from lacp_mgr.c */
static  UI32_T  LACP_OM_SemId;
static  UI32_T  original_priority;


/* moved from lacp_mgr.c */
static void    LACP_OM_InitSemaphore(void);

#if 0   /* Allenc */
static void LACP_OM_LoadData(UI32_T unit, UI32_T port);
#endif  /* Allenc */
static void LACP_OM_LoadPortData( UI32_T unit, UI32_T port);
static void LACP_OM_LoadAggData(UI32_T agg_index);
static void LACP_OM_InitPort( LACP_PORT_T *pPort);

/* Interface */
void LACP_OM_GetSlowProtocolMCAddr(UI8_T* mac_addr)
{
    memcpy(mac_addr, LACP_SlowProtocolMCAddr, 6);

    return;
}

LACP_EE_SYSTEM_DATA_T*     LACP_OM_SystemPtr(void)
{
    return &LACP_EESystemData;
}

/* Interface for Task Management */
/*
LACP_TaskData_T* LACP_OM_TmPtr(void)
{
    return &LACP_TaskManagement;
}

UI32_T  LACP_OM_Tm_AccessTaskId(UI32_T value, UI8_T action)
{
    switch (action)
    {
        case LACP_OM_TMINF_ACCESS_SET:
            LACP_TaskManagement.task_id = value;
            break;
        default:
            break;
    }

    return (LACP_TaskManagement.task_id);
}

UI32_T  LACP_OM_Tm_AccessTimerId(UI32_T value, UI8_T action)
{
    switch (action)
    {
        case LACP_OM_TMINF_ACCESS_SET:
            LACP_TaskManagement.lacp_timer_id = value;
            break;
        default:
            break;
    }

    return (LACP_TaskManagement.lacp_timer_id);
}
*/
#if 0
UI8_T   LACP_OM_Tm_AccessOperationState(UI8_T value, UI8_T action)
{
    switch (action)
    {
        case LACP_OM_TMINF_ACCESS_SET:
            LACP_TaskManagement.operation_state = value;
            break;
        default:
            break;
    }

    return (LACP_TaskManagement.operation_state);
}
#endif

void LACP_OM_RefreshAllObject(void)
{
    UI8_T   mac_addr[6];
    UI32_T  unit, port, agg_index;/* , admin_status; */

    LACP_PORT_T *pPort;
    Port_Info_T port_info;

    /* Lewis: init lacp_system for state machines  */
    memset( &lacp_system, 0x0, sizeof(lacp_system)); /* Lewis: lacp_system is used in running state machine */
    LACP_OM_FactoryDefaultEEObject();
    SWCTRL_GetCpuMac(mac_addr);
    memcpy(lacp_system.SystemId, mac_addr, 6);

    for (agg_index = 0; agg_index < SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; agg_index++)
    {
        LACP_OM_LoadAggData(agg_index);
    } /* End of for */

    /* Lewis: init a key for each port, get default and flash value for Ports & Init the state machines */
    for(unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ; unit++ )
    {
        /* if unit doesn't exist -> break! */
/*     Lewis marked out the following tempararily until lacp state machines check existed
        if (!STKTPLG_MGR_UnitExist(unit))
            break; */
        for( port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            /* if port doesn't exist -> continue. Lewis: if always continue -> break; */
/*     Lewis marked out the following tempararily until lacp state machines check existed   */
/*            if (!STKTPLG_MGR_PortExist(unit, port))
                break; */
#if 0   /* Allenc */
            LACP_OM_LoadData( unit, port); /* remember to init Key! */
#endif  /* Allenc */
            LACP_OM_LoadPortData( unit, port);
#if 0   /* Allenc */
            memcpy( (lacp_system.aggregator[LACP_LINEAR_ID( unit, port)-1]).AggMACAddress, mac_addr, 6);
#endif  /* Allenc */
            /* Initialize this port. */
            LACP_OM_InitPort( LACP_FIND_PORT(unit, port)); /* state machines begin */
        }
    } /* End of for */

    /* Lewis: Begin the state machines with correct port admin values */
    for(unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ; unit++ )
    {
        /* if unit doesn't exist -> break! */
/*     Lewis marked out the following tempararily until lacp state machines check existed

        if (!STKTPLG_MGR_UnitExist(unit))
            break;*/
        for( port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
            UI32_T  admin_state, oper_state;
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */


            pPort = LACP_FIND_PORT( unit, port);
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
            if (pPort->LACPEnabled)
            {
                admin_state = VAL_lacpPortStatus_enabled;
            }
            else
            {
                admin_state = VAL_lacpPortStatus_disabled;
            }
            if (pPort->LACP_OperStatus)
            {
                oper_state  = VAL_lacpPortStatus_enabled;
            }
            else
            {
                oper_state  = VAL_lacpPortStatus_disabled;
            }
            SWCTRL_PMGR_SetPortLacpAdminEnable(LACP_LINEAR_ID( unit , port), admin_state);
            SWCTRL_PMGR_SetPortLacpOperEnable(LACP_LINEAR_ID( unit , port), oper_state);
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

            if(SWCTRL_GetPortInfo(LACP_LINEAR_ID( unit, port), &port_info) != FALSE)
            {
                switch(port_info.admin_state)
                {
                case VAL_ifAdminStatus_down:
                    /* If this port is "DISABLED".             */
                    LACP_EnablePort( pPort, FALSE);
                    break;
                default:
                case VAL_ifAdminStatus_up:
                    /* Default it's considered as "ENABLED". */
                    LACP_EnablePort( pPort, TRUE);
                    break;
                }
            }

        }
    }
}

void LACP_OM_FactoryDefaultEEObject(void)
{
    lacp_system.SystemPriority = LACP_EESystemData.priority = LACP_SYSTEM_DEFAULT_PRIORITY;
    lacp_system.lacp_oper = LACP_EESystemData.lacp_admin = LACP_DEFAULT_SYSTEM_ADMIN;
    /* SWCTRL_SetLacpEnable((UI32_T)LACP_DEFAULT_SYSTEM_ADMIN); */
    LACP_EEPortData.lacp_admin = LACP_DEFAULT_PORT_ADMIN;
    LACP_EEPortData.actor_priority = LACP_PORT_DEFAULT_PRIORITY;
    LACP_EEPortData.actor_admin_state.port_state = LACP_DEFAULT_PORT_ACTOR_ADMIN_STATE;
    LACP_EEPortData.partner_priority = LACP_SYSTEM_DEFAULT_PRIORITY;
    LACP_EEPortData.partner_admin_state.port_state = LACP_DEFAULT_PORT_PARTNER_ADMIN_STATE;
}

void LACP_OM_EESetSystemPriority(UI16_T priority)
{
    LACP_EESystemData.priority = priority;
}

void LACP_OM_EESetSystemAdmin(UI8_T admin)
{
    LACP_EESystemData.lacp_admin = admin;
}

void LACP_OM_EESetPortAdmin(UI8_T admin)
{
    LACP_EEPortData.lacp_admin = admin;
}

void LACP_OM_EESetPortActorAdminStateActivity(UI8_T acvitity)
{
    LACP_EEPortData.actor_admin_state.port_state = (LACP_EEPortData.actor_admin_state.port_state & (!LACP_dot3adAggPortActorOperState_LACP_Activity)) | acvitity;
}

void LACP_OM_EESetPortActorAdminStateTimeOut(UI8_T timeout)
{
    LACP_EEPortData.actor_admin_state.port_state = (LACP_EEPortData.actor_admin_state.port_state & (!LACP_dot3adAggPortActorOperState_LACP_Timeout)) | timeout;
}

void LACP_OM_EESetPortActorAdminStateAggregation(UI8_T aggregation)
{
    LACP_EEPortData.actor_admin_state.port_state = (LACP_EEPortData.actor_admin_state.port_state & (!LACP_dot3adAggPortActorOperState_Aggregation)) | aggregation;
}

void LACP_OM_EESetPortActorAdminState(UI8_T admin_state)
{
    LACP_EEPortData.actor_admin_state.port_state = admin_state;
}

void LACP_OM_EESetPortActorPriority(UI16_T priority)
{
    LACP_EEPortData.actor_priority = priority;
}

void LACP_OM_EESetPortPartnerPriority(UI16_T priority)
{
    LACP_EEPortData.partner_priority = priority;
}

void LACP_OM_EESetPortPartnerAdminStateActivity(UI8_T acvitity)
{
    LACP_EEPortData.partner_admin_state.port_state = (LACP_EEPortData.partner_admin_state.port_state & (!LACP_dot3adAggPortActorOperState_LACP_Activity)) | acvitity;
}

void LACP_OM_EESetPortPartnerAdminStateTimeOut(UI8_T timeout)
{
    LACP_EEPortData.partner_admin_state.port_state = (LACP_EEPortData.partner_admin_state.port_state & (!LACP_dot3adAggPortActorOperState_LACP_Timeout)) | timeout;
}

void LACP_OM_EESetPortPartnerAdminStateAggregation(UI8_T aggregation)
{
    LACP_EEPortData.partner_admin_state.port_state = (LACP_EEPortData.partner_admin_state.port_state & (!LACP_dot3adAggPortActorOperState_Aggregation)) | aggregation;
}

void LACP_OM_EESetPortPartnerAdminState(UI8_T admin_state)
{
    LACP_EEPortData.partner_admin_state.port_state = admin_state;
}

/* OM Initialization */
void LACP_OM_Init(void)
{
    LACP_OM_InitSemaphore();

    /* LACP_OM_LportMgmt_Init(); */

    /* LACP_OM_InitAllObject(FALSE); */

    lacp_system.lacp_oper = LACP_ADMIN_OFF; /* for not processing any LACP event at this INIT stage */
    return;
}

#if 0   /* Allenc */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_OM_LoadData
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to load EE configuration to memory.
 * INPUT   : unit -- unit.
 *           port -- port.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_OM_LoadData( UI32_T unit, UI32_T port)
{
    /* All Actor and Partner EE data should load here. */
    LACP_PORT_T       *pPort;
    LACP_AGGREGATOR_T *pAgg;


    pPort = LACP_FIND_PORT( unit, port);
#if 0   /* Allenc */
    pAgg  = LACP_FIND_AGGREGATOR( unit, port);
#endif  /* Allenc */
    pAgg    = LACP_UTIL_AllocateAggregator(pPort);

    (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_DEFAULT;

    /* Once this two is initialized, it will never being modified. */
    pPort->slot = unit;
    pPort->port = port;

    /* Aggregator identifier is always starting from 1 as port since one port own */
    /* one aggregator.                                                            */
    pAgg->AggID = LACP_LINEAR_ID( unit, port);
    pPort->AggActorAdminPort.port_no   = (UI16_T)pAgg->AggID;
    pPort->AggPartnerAdminPort.port_no = (UI16_T)pAgg->AggID;

    /* Set system priority and id to aggregator */
    pPort->AggActorAdminPort.system_priority = lacp_system.SystemPriority;
    pPort->AggPartnerAdminPort.system_priority = lacp_system.SystemPriority;

    memcpy( pPort->AggActorAdminPort.system_id, lacp_system.SystemId, 6);
    /* We will not set partner system ID, it will be 00-00-00-00-00-00 */
    memset( pPort->AggPartnerAdminPort.system_id, 0x0, 6);

    /* Initialize port priority. */
    pPort->AggActorAdminPort.port_priority   = LACP_EEPortData.actor_priority;
    pPort->AggPartnerAdminPort.port_priority = LACP_EEPortData.partner_priority;

    /* Here we always set to the same state with previous state. */
    pPort->AggActorAdminPort.state.port_state   = LACP_EEPortData.actor_admin_state.port_state;
    pPort->AggPartnerAdminPort.state.port_state = LACP_EEPortData.partner_admin_state.port_state;
}
#endif  /* Allenc */
/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_OM_LoadPortData
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to load default configuration to the
 *           port database.
 * INPUT   : unit -- unit.
 *           port -- port.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_OM_LoadPortData( UI32_T unit, UI32_T port)
{
    LACP_PORT_T       *port_ptr;


    port_ptr    = LACP_FIND_PORT( unit, port);

    (port_ptr->AggActorAdminPort).key   = SYS_DFLT_LACP_KEY_DEFAULT;
    port_ptr->static_admin_key          = FALSE;

    /* Once this two is initialized, it will never being modified. */
    port_ptr->slot  = unit;
    port_ptr->port  = port;
    port_ptr->pAggregator   = NULL;

    port_ptr->AggActorAdminPort.port_no     = (UI16_T)LACP_LINEAR_ID( unit, port);
    port_ptr->AggPartnerAdminPort.port_no   = (UI16_T)LACP_LINEAR_ID( unit, port);

    /* Set system priority and id to aggregator */
    port_ptr->AggActorAdminPort.system_priority     = lacp_system.SystemPriority;
    port_ptr->AggPartnerAdminPort.system_priority   = lacp_system.SystemPriority;

    memcpy( port_ptr->AggActorAdminPort.system_id, lacp_system.SystemId, 6);
    /* We will not set partner system ID, it will be 00-00-00-00-00-00 */
    memset( port_ptr->AggPartnerAdminPort.system_id, 0x0, 6);

    /* Initialize port priority. */
    port_ptr->AggActorAdminPort.port_priority   = LACP_EEPortData.actor_priority;
    port_ptr->AggPartnerAdminPort.port_priority = LACP_EEPortData.partner_priority;

    /* Here we always set to the same state with previous state. */
    port_ptr->AggActorAdminPort.state.port_state    = LACP_EEPortData.actor_admin_state.port_state;
    port_ptr->AggPartnerAdminPort.state.port_state  = LACP_EEPortData.partner_admin_state.port_state;
    return;
} /* End of LACP_OM_LoadPortData */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_OM_LoadAggData
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to load default configuration to the
 *           aggregator database.
 * INPUT   : agg_index  -- aggregator index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_OM_LoadAggData(UI32_T agg_index)
{
    LACP_AGGREGATOR_T *agg_ptr;

    agg_ptr = LACP_OM_GetAggPtr(agg_index);
    agg_ptr->AggID          = agg_index+1;
    agg_ptr->pLACP_Ports    = NULL;
    agg_ptr->static_admin_key   = FALSE;
    memcpy( agg_ptr->AggMACAddress,     lacp_system.SystemId, 6);
    memcpy( agg_ptr->AggActorSystemID,  lacp_system.SystemId, 6);
    agg_ptr->AggActorSystemPriority = lacp_system.SystemPriority;
    memset( agg_ptr->AggPartnerSystemID, 0x0, 6);
    agg_ptr->AggAggregateOrIndividual=LACP_AGGREGATABLE_LINK;
    agg_ptr->AggActorTimeout = LACP_DEFAULT_SYSTEM_TIMEOUT;
    return;
} /* End of LACP_OM_LoadAggData */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_init_port
 *------------------------------------------------------------------------
 * FUNCTION: This function is trying to initialize the port.
 * INPUT   : pPort -- pointer to the port we want to initialize.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void LACP_OM_InitPort( LACP_PORT_T *pPort)
{
    /*UI32_T admin_status;*/
    /*Port_Info_T port_info;*/
    /* One interesting thing is what should we do here? The answer may be funny.     */
    /* I think, first we just don't care about the port state of this port. This     */
    /* means , just consider this port as "Disabled". At this time, we don't process */
    /* any frame pass from GAL_NIC.                                                  */
    pPort->PortEnabled = FALSE;
    pPort->Port_OperStatus = FALSE;

    pPort->FullDuplexMode = (BOOL_T)(LACP_DEFAULT_PORT_ADMIN == LACP_ADMIN_ON);
    pPort->PortSpeed = 0; /* VAL_portSpeedDpxStatus_error 1 */

    /* The LACP_Enabled is decided by EEPROM and here we only assign it.             */
    pPort->LACPEnabled =  (LACP_EESystemData.lacp_admin ==  LACP_ADMIN_ON) && (LACP_EEPortData.lacp_admin == LACP_ADMIN_ON);
    pPort->LACP_OperStatus = pPort->FullDuplexMode && pPort->LACPEnabled;

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    SWCTRL_PMGR_SetPortLacpAdminEnable(LACP_LINEAR_ID( pPort->slot , pPort->port), LACP_DEFAULT_PORT_ADMIN); /* If default is LACP On -> wait at Dormant first to avoid state oscillate in swctrl */
#else
    SWCTRL_PMGR_SetPortLacpEnable(LACP_LINEAR_ID( pPort->slot , pPort->port), LACP_DEFAULT_PORT_ADMIN); /* If default is LACP On -> wait at Dormant first to avoid state oscillate in swctrl */
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */
    pPort->pNext = NULL;

    pPort->Matched = FALSE; /* Lewis added */
    /* Initialize this port, and inform related state machine. */
    LACP_Tx_Machine      ( pPort, LACP_SYS_INIT);
    LACP_Periodic_Machine( pPort, LACP_SYS_INIT);
    LACP_Mux_Machine     ( pPort, LACP_SYS_INIT);
    LACP_Rx_Machine      ( pPort, LACP_SYS_INIT, NULL);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetPortPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns the pointer of the port
 * INPUT    : lport -- lport number
 * OUTPUT   : None
 * RETUEN   : Pointer to the port variables
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_PORT_T*   LACP_OM_GetPortPtr(UI32_T lport)
{
    return &(lacp_port[lport - 1]);
} /* End of LACP_OM_GetPortPtr */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetAggPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns the pointer of the specified aggregator
 * INPUT    : agg_index -- aggregator index
 * OUTPUT   : None
 * RETUEN   : Pointer to the specified aggregator
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_AGGREGATOR_T*  LACP_OM_GetAggPtr(UI32_T agg_index)
{
    return &(lacp_system.aggregator[agg_index]);
} /* End of LACP_OM_GetAggPtr */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetSystemPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns the pointer of the system
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : Pointer to the system variables
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_SYSTEM_T*   LACP_OM_GetSystemPtr(void)
{
    return &(lacp_system);
} /* End of LACP_OM_GetSystemPtr */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_FindPortPtr
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : unit  -- unit number
 *            port  -- port number
 * OUTPUT   : None
 * RETUEN   : Pointer to the port variables
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_PORT_T*   LACP_OM_FindPortPtr(UI32_T unit, UI32_T port)
{
    return &(lacp_port[LACP_LINEAR_ID(unit,port)-1]);
} /* End of LACP_OM_FindPortPtr */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_FindAggregatorPtr
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : unit  -- unit number
 *            port  -- port number
 * OUTPUT   : None
 * RETUEN   : Pointer to the aggregator variables
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_AGGREGATOR_T*   LACP_OM_FindAggregatorPtr(UI32_T unit, UI32_T port)
{
    return &(lacp_system.aggregator[LACP_LINEAR_ID(unit,port)-1]);
} /* End of LACP_OM_FindAggregatorPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_RefreshPortObject
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will refresh the port OM to the default value
 * INPUT    : starting_port_ifindex -- the ifindex of the first port
 *            number_of_port        -- the number of ports to be refreshed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LACP_OM_RefreshPortObject(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T      port_ifindex;
    LACP_PORT_T *pPort;
    UI32_T      lport;
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    UI32_T  admin_state, oper_state;
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

    for(port_ifindex = 0; port_ifindex < number_of_port ; port_ifindex++ )
    {
        /* Initialize this port. */
        lport   = starting_port_ifindex+port_ifindex;
        pPort   = LACP_OM_GetPortPtr(lport);
        LACP_OM_InitPort( pPort ); /* state machines begin */
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
        if (pPort->LACPEnabled)
        {
            admin_state = VAL_lacpPortStatus_enabled;
        }
        else
        {
            admin_state = VAL_lacpPortStatus_disabled;
        }
        if (pPort->LACP_OperStatus)
        {
            oper_state  = VAL_lacpPortStatus_enabled;
        }
        else
        {
            oper_state  = VAL_lacpPortStatus_disabled;
        }
        SWCTRL_PMGR_SetPortLacpAdminEnable(lport, admin_state);
        SWCTRL_PMGR_SetPortLacpOperEnable(lport, oper_state);
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */
    } /* End of for */

    return;
} /* End of LACP_OM_RefreshPortObject */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for XSTP OM.
 *
 * INPUT   : msgbuf_p - input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p - output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : 1. The size of msgbuf_p->msg_buf must be large enough to carry
 *              any response messages.
 *-----------------------------------------------------------------------------
 */
BOOL_T LACP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    LACP_OM_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (LACP_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding XSTP_OM function
     */
    switch (msg_p->type.cmd)
    {
        case LACP_OM_IPC_GETRUNNINGDOT3ADLACPPORTENABLED:
        	msg_p->type.ret_ui32 = LACP_OM_GetRunningDot3adLacpPortEnabled(
        	    msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = LACP_OM_GET_MSG_SIZE(arg_grp1);
            break;

    	case LACP_OM_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINSYSTEMID:
    	    msg_p->type.ret_ui32 =
    	        LACP_OM_GetRunningDot3adAggPortPartnerAdminSystemId(
    	        msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
    	    msgbuf_p->msg_size = LACP_OM_GET_MSG_SIZE(arg_grp2);
    	    break;

    	default:
    	    SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = LACP_RETURN_ERROR;
            msgbuf_p->msg_size = LACP_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of LACP_OM_HandleIPCReqMsg */

/*=============================================================================
 * Moved from lacp_mgr.c
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_EnterCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical region before a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in LACP_TASK_WaitEvent and
 *              LACP_MGR_Function to make the LACP task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
void    LACP_OM_EnterCriticalRegion(void)
{
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LACP_OM_SemId);

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_LeaveCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical region after a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in LACP_TASK_WaitEvent and
 *              LACP_MGR_Function to make the LACP task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
void    LACP_OM_LeaveCriticalRegion(void)
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(LACP_OM_SemId, original_priority);

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the LACP enabled value of a port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *lacp_state        -- pointer of the enable value
 *                                         LACP_ADMIN_ON or LACP_ADMIN_OFF
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T LACP_OM_GetRunningDot3adLacpPortEnabled(UI32_T ifindex, UI32_T *lacp_state)
{
    UI32_T  slot = 0,
            port = 0;
    UI32_T  trunk_id = 0;

    LACP_PORT_T *pPort;

    SWCTRL_Lport_Type_T l_type = SWCTRL_LPORT_UNKNOWN_PORT;

    l_type = SWCTRL_LogicalPortToUserPort( ifindex, &slot, &port, &trunk_id);

    if ( (l_type == SWCTRL_LPORT_UNKNOWN_PORT) ||
         (l_type == SWCTRL_LPORT_TRUNK_PORT) )
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    /* mark for temporary patch, need to refine LACP later
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LACP_OM_SemId);
    */
    pPort = LACP_FIND_PORT( slot, port);

    if (pPort->LACPEnabled)
        *lacp_state = LACP_ADMIN_ON;
    else
        *lacp_state = LACP_ADMIN_OFF;
    /* mark for temporary patch, need to refine LACP later
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(LACP_OM_SemId, original_priority);
    */
    if ((*lacp_state) == LACP_DEFAULT_PORT_ADMIN)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_system_priority information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggActorSystemPriority(UI16_T agg_index, UI16_T *priority)
{
    return  SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* End of LACP_OM_GetRunningDot3adAggActorSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggCollectorMaxDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_collector_max_delay information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI32_T  *max_delay  -- the dot3ad_agg_collector_max_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggCollectorMaxDelay(UI16_T agg_index, UI32_T *max_delay)
{
    return  SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id)
{
    return  SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_port -- the dot3ad_agg_port_partner_admin_port value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortPartnerAdminPort(UI16_T port_index, UI16_T *admin_port)
{
    return  SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortActorAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *admin_state-- the dot3ad_agg_port_actor_admin_state value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortActorAdminState(UI16_T port_index, UI8_T *admin_state)
{
    return  SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *admin_state-- the dot3ad_agg_port_partner_admin_state value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortPartnerAdminState(UI16_T port_index, UI8_T *admin_state)
{
    return  SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for LACP objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in LACP_TASK_Init.
 *-------------------------------------------------------------------------
 */
static void    LACP_OM_InitSemaphore(void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
            &LACP_OM_SemId) != SYSFUN_OK)
    {
        printf("\n%s: get lldp om sem id fail.\n", __FUNCTION__);
    }
/*
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &LACP_OM_SemId)
            != SYSFUN_OK)
    {
        printf("%s: get om sem id fail.\n", __FUNCTION__);
    }
*/
    return;
}
