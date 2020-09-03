/*-----------------------------------------------------------------------------
 * MODULE NAME: LACP_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of LACP OM which are only used by LACP.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/21     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef LACP_OM_PRIVATE_H
#define LACP_OM_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "lacp_om.h"
#include "lacp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

enum LACP_OM_RETURN_CODE_E
{
    LACP_OM_RETURN_OK                        = LACP_RETURN_SUCCESS,
    LACP_OM_RETURN_ERROR                     = LACP_RETURN_ERROR,
    LACP_OM_RETURN_OTHERS
};

enum
{
    LACP_OM_TMINF_ACCESS_INC,
    LACP_OM_TMINF_ACCESS_DEC,
    LACP_OM_TMINF_ACCESS_SET,
    LACP_OM_TMINF_ACCESS_GET
};

/*
enum
{
    LACP_OM_TMINF_OPSTATE_SLAVE,
    LACP_OM_TMINF_OPSTATE_TRANSITION,
    LACP_OM_TMINF_OPSTATE_MASTER
};
*/


/* MACRO FUNCTION DECLARATIONS
 */

#define LACP_FIND_PORT( unit, port)       LACP_OM_FindPortPtr(unit, port)
#if 0
#define LACP_FIND_AGGREGATOR( unit, port) LACP_OM_FindAggregatorPtr(unit, port)
#endif


/* DATA TYPE DECLARATIONS
 */

/* Lewis: the following is what we plan to save in flash for now */
#pragma pack(1)
typedef struct LACP_EE_SYSTEM_DATA_T
{
    SYSTEM_PRIORITY_T priority;
    UI8_T             lacp_admin; /* 1: enable, 2: disable */
} LACP_EE_SYSTEM_DATA_T;

typedef struct LACP_EE_PORT_DATA_T
{
    UI8_T             lacp_admin;
    PORT_PRIORITY_T   actor_priority;
    LACP_PORT_STATE_T actor_admin_state;
    PORT_PRIORITY_T   partner_priority;
    LACP_PORT_STATE_T partner_admin_state;
} LACP_EE_PORT_DATA_T;
#pragma pack()

/* Lewis: Needs to define AggregationPort according to MIB */

typedef struct LACP_PORT_OBJECT_S /* LACP Port data */
{
    LAC_INFO_T              AggActorPort;        /* Actor Port & Oper data */
    LAC_INFO_T              AggPartnerPort;
    LAC_INFO_T              AggActorAdminPort;
    LAC_INFO_T              AggPartnerAdminPort;
    LACP_PORT_STATISTICS_T  AggPortStats;
    LACP_PORT_DEBUG_INFO_T  AggPortDebug;
    UI8_T                   AggPortAggregateOrIndividual;
    BOOL_T                  PortEnabled;
    BOOL_T                  LACPEnabled;
    BOOL_T                  LinkUp;
    BOOL_T                  NTT;
    BOOL_T                  PortSpeed;
    UI8_T                   PortDuplexMode;
    SELECTED_VALUE_E        Selected;     /* ? */
    BOOL_T                  Matched;      /* ? */
    BOOL_T                  Attach;
    BOOL_T                  Attached;
    BOOL_T                  PartnerChurn; /* ? */
    BOOL_T                  ActorChurn;   /* ? */
    UI8_T                   FramesInLastSec;
    UI8_T                   LogicalTrunkMemberFlag;
    UI8_T                   DisconDelayedFlag;
    UI8_T                   PortBlockedFlag;
    UI32_T                  InitialPDUTimer;
    BOOL_T                  PortMoved;
} LACP_PORT_OBJECT_T;

/*
 * Task Information
 */
typedef struct LACP_TaskData_S
{
    UI32_T                  task_id;
    UI32_T                  lacp_timer_id;          /* Used to set bpdu timer event periodically */
    UI32_T                  msgq_id;
/*  UI8_T                   operation_state;  */    /* 0:Transition; 1:Master; 2:Slave; */
} LACP_TaskData_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

//LACP_TaskData_T*           LACP_OM_TmPtr(void);
/* LACP_EE_SYSTEM_DATA_T*     LACP_OM_EESystemPtr(void); */
/* LACP_AGGREGATOR_OBJECT_T*  LACP_OM_AggrObjPtr(UI32_T aggr_index); */
LACP_PORT_OBJECT_T*        LACP_OM_PortObjPtr(UI32_T port_index);
UI32_T  LACP_OM_Tm_AccessTaskId(UI32_T value, UI8_T action);
//UI32_T  LACP_OM_Tm_AccessTimerId(UI32_T value, UI8_T action);
/* UI8_T   LACP_OM_Tm_AccessOperationState(UI8_T value, UI8_T action);  */
void LACP_OM_Init(void);
void LACP_OM_RefreshAllObject(void);

void LACP_OM_FactoryDefaultEEObject(void);
void LACP_OM_EESetSystemPriority(UI16_T priority);
void LACP_OM_EESetSystemAdmin(UI8_T admin);
void LACP_OM_EESetPortAdmin(UI8_T admin);
void LACP_OM_EESetPortActorAdminStateActivity(UI8_T acvitity);
void LACP_OM_EESetPortActorAdminStateTimeOut(UI8_T timeout);
void LACP_OM_EESetPortActorAdminStateAggregation(UI8_T aggregation);
void LACP_OM_EESetPortActorAdminState(UI8_T admin_state);
void LACP_OM_EESetPortActorPriority(UI16_T priority);
void LACP_OM_EESetPortPartnerPriority(UI16_T priority);
void LACP_OM_EESetPortPartnerAdminStateActivity(UI8_T acvitity);
void LACP_OM_EESetPortPartnerAdminStateTimeOut(UI8_T timeout);
void LACP_OM_EESetPortPartnerAdminStateAggregation(UI8_T aggregation);
void LACP_OM_EESetPortPartnerAdminState(UI8_T admin_state);

void LACP_OM_GetSlowProtocolMCAddr(UI8_T* mac_addr);

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
LACP_PORT_T*   LACP_OM_GetPortPtr(UI32_T lport);

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
LACP_AGGREGATOR_T*  LACP_OM_GetAggPtr(UI32_T agg_index);

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
LACP_SYSTEM_T*   LACP_OM_GetSystemPtr(void);

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
LACP_PORT_T*   LACP_OM_FindPortPtr(UI32_T unit, UI32_T port);


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
LACP_AGGREGATOR_T*   LACP_OM_FindAggregatorPtr(UI32_T unit, UI32_T port);

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
void LACP_OM_RefreshPortObject(UI32_T starting_port_ifindex, UI32_T number_of_port);


/*=============================================================================
 * Moved from lacp_mgr.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_EnterCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical region before a task invokes the LACP
 *                     objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in LACP_TASK_WaitEvent and
 *              LACP_MGR_Function to make the LACP task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
void    LACP_OM_EnterCriticalRegion(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_LeaveCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical region after a task invokes the LACP
 *               objects.
 * INPUT    :   cli_msgq_handle - the handle of message queue of CLI
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in LACP_TASK_WaitEvent and
 *              LACP_MGR_Function to make the LACP task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
void    LACP_OM_LeaveCriticalRegion(void);


#endif /* #ifndef LACP_OM_PRIVATE_H */
