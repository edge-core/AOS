/*-------------------------------------------------------------------------
 * Module Name: xstp_engine.h
 *-------------------------------------------------------------------------
 * PURPOSE: Declarations for xstp_engine.c
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */

#ifndef _XSTP_ENGINE_H
#define _XSTP_ENGINE_H

#include "xstp_om_private.h"
#include "xstp_type.h"


/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1W */
#ifdef  XSTP_TYPE_PROTOCOL_RSTP

enum XSTP_ENGINE_SM_PTI_STATE_E                         /* 17.20 */
{
    XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND,
    XSTP_ENGINE_SM_PTI_STATE_TICK,

    /* Progress */
    XSTP_ENGINE_SM_PTI_STATE_PROGRESS_ONE_SECOND,
    XSTP_ENGINE_SM_PTI_STATE_PROGRESS_TICK
};

enum XSTP_ENGINE_SM_PIM_STATE_E                         /* 17.21 */
{
    XSTP_ENGINE_SM_PIM_STATE_DISABLED,
    XSTP_ENGINE_SM_PIM_STATE_AGED,
    XSTP_ENGINE_SM_PIM_STATE_UPDATE,
    XSTP_ENGINE_SM_PIM_STATE_CURRENT,
    XSTP_ENGINE_SM_PIM_STATE_SUPERIOR,
    XSTP_ENGINE_SM_PIM_STATE_REPEAT,
    XSTP_ENGINE_SM_PIM_STATE_AGREEMENT,
    XSTP_ENGINE_SM_PIM_STATE_RECEIVE,

    /* Progress */
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_DISABLED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_UPDATE,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_CURRENT,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_SUPERIOR,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_REPEAT,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGREEMENT,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_RECEIVE
};

enum XSTP_ENGINE_SM_PRS_STATE_E                         /* 17.22 */
{
    XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE,
    XSTP_ENGINE_SM_PRS_STATE_ROLE_SELECTION,

    /* Progress */
    XSTP_ENGINE_SM_PRS_STATE_PROGRESS_INIT_BRIDGE,
    XSTP_ENGINE_SM_PRS_STATE_PROGRESS_ROLE_SELECTION
};

enum XSTP_ENGINE_SM_PRT_STATE_E                         /* 17.23 */
{
    XSTP_ENGINE_SM_PRT_STATE_INIT_PORT,
    XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT,
    XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT,
    XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT,
    XSTP_ENGINE_SM_PRT_STATE_ROOT_PROPOSED,
    XSTP_ENGINE_SM_PRT_STATE_ROOT_AGREED,
    XSTP_ENGINE_SM_PRT_STATE_ROOT_FORWARD,
    XSTP_ENGINE_SM_PRT_STATE_ROOT_LEARN,
    XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT,
    XSTP_ENGINE_SM_PRT_STATE_REROOT,
    XSTP_ENGINE_SM_PRT_STATE_REROOTED,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PROPOSE,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_SYNCED,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_RETIRED,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_FORWARD,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LEARN,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LISTEN,
    XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT,

    /* Progress */
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_INIT_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCK_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BACKUP_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_PROPOSED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_AGREED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_FORWARD,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_LEARN,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_REROOT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_REROOTED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_PROPOSE,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_SYNCED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_RETIRED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_FORWARD,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_LEARN,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_LISTEN,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_PORT
};

enum XSTP_ENGINE_SM_PST_STATE_E                         /* 17.24 */
{
    XSTP_ENGINE_SM_PST_STATE_DISCARDING,
    XSTP_ENGINE_SM_PST_STATE_LEARNING,
    XSTP_ENGINE_SM_PST_STATE_FORWARDING,

    /* Progress */
    XSTP_ENGINE_SM_PST_STATE_PROGRESS_DISCARDING,
    XSTP_ENGINE_SM_PST_STATE_PROGRESS_LEARNING,
    XSTP_ENGINE_SM_PST_STATE_PROGRESS_FORWARDING
};

enum XSTP_ENGINE_SM_TCM_STATE_E                         /* 17.25 */
{
    XSTP_ENGINE_SM_TCM_STATE_INIT,
    XSTP_ENGINE_SM_TCM_STATE_INACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_ACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_DETECTED,
    XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN,
    XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC,
    XSTP_ENGINE_SM_TCM_STATE_PROPAGATING,
    XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED,

    /* Progress */
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INIT,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_DETECTED,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_NOTIFIED_TCN,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_NOTIFIED_TC,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_PROPAGATING,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACKNOWLEDGED
};

enum XSTP_ENGINE_SM_PPM_STATE_E                         /* 17.26 */
{
    XSTP_ENGINE_SM_PPM_STATE_INIT,
    XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_SEND_STP,
    XSTP_ENGINE_SM_PPM_STATE_SENDING_STP,

    /* Progress */
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_INIT,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SEND_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SEND_STP,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_STP
};

enum XSTP_ENGINE_SM_PTX_STATE_E                         /* 17.27 */
{
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP,
    XSTP_ENGINE_SM_PTX_STATE_IDLE,

    /* Progress */
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_INIT,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_PERIODIC,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_CONFIG,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_TCN,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_RSTP,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE
};

enum XSTP_ENGINE_PORTVAR_INFO_IS_E                      /* 17.18.6 */
{
    XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED                = 0,
    XSTP_ENGINE_PORTVAR_INFO_IS_MINE,
    XSTP_ENGINE_PORTVAR_INFO_IS_AGED,
    XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED
};

enum XSTP_ENGINE_PORTVAR_ROLE_E                         /* 17.18.30 */
{
    XSTP_ENGINE_PORTVAR_ROLE_DISABLED                   = 0,
    XSTP_ENGINE_PORTVAR_ROLE_ROOT,
    XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED,
    XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE,
    XSTP_ENGINE_PORTVAR_ROLE_BACKUP
};

enum XSTP_ENGINE_PORTVAR_RCVD_MSG_E
{
    XSTP_ENGINE_PORTVAR_RCVD_MSG_SUPERIOR_DESIGNATED_MSG,
    XSTP_ENGINE_PORTVAR_RCVD_MSG_REPEATED_DESIGNATED_MSG,
    XSTP_ENGINE_PORTVAR_RCVD_MSG_CONFIRMED_ROOT_MSG,
    XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG
};


#endif /* XSTP_TYPE_PROTOCOL_RSTP */
/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1S */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP

enum XSTP_ENGINE_SM_PTI_STATE_E                         /* 13.27 */
{
    XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND,
    XSTP_ENGINE_SM_PTI_STATE_TICK,

    /* Progress */
    XSTP_ENGINE_SM_PTI_STATE_PROGRESS_ONE_SECOND,
    XSTP_ENGINE_SM_PTI_STATE_PROGRESS_TICK
};

enum XSTP_ENGINE_SM_PRX_STATE_E                         /* 13.28 */
{
    XSTP_ENGINE_SM_PRX_STATE_DISCARD,
    XSTP_ENGINE_SM_PRX_STATE_RECEIVE,

    /* Progress */
    XSTP_ENGINE_SM_PRX_STATE_PROGRESS_DISCARD,
    XSTP_ENGINE_SM_PRX_STATE_PROGRESS_RECEIVE
};

enum XSTP_ENGINE_SM_PPM_STATE_E                         /* 13.29 */ /* 17.26 */
{
    XSTP_ENGINE_SM_PPM_STATE_INIT,
    XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_SEND_STP,
    XSTP_ENGINE_SM_PPM_STATE_SENDING_STP,

    /* Progress */
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_INIT,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SEND_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_RSTP,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SEND_STP,
    XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_STP
};

enum XSTP_ENGINE_SM_PTX_STATE_E                         /* 13.30 */ /* 17.27 */
{
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN,
    XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP,
    XSTP_ENGINE_SM_PTX_STATE_IDLE,

    /* Progress */
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_INIT,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_PERIODIC,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_CONFIG,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_TCN,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_TRANSMIT_RSTP,
    XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE
};

enum XSTP_ENGINE_SM_PIM_STATE_E                         /* 13.31 */
{
    XSTP_ENGINE_SM_PIM_STATE_DISABLED,
    XSTP_ENGINE_SM_PIM_STATE_ENABLED,
    XSTP_ENGINE_SM_PIM_STATE_AGED,
    XSTP_ENGINE_SM_PIM_STATE_UPDATE,
    XSTP_ENGINE_SM_PIM_STATE_CURRENT,
    XSTP_ENGINE_SM_PIM_STATE_SUPERIOR_DESIGNATED,
    XSTP_ENGINE_SM_PIM_STATE_REPEATED_DESIGNATED,
    XSTP_ENGINE_SM_PIM_STATE_ROOT,
    XSTP_ENGINE_SM_PIM_STATE_OTHER,
    XSTP_ENGINE_SM_PIM_STATE_RECEIVE,

    /* Progress */
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_DISABLED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_ENABLED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_UPDATE,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_CURRENT,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_SUPERIOR_DESIGNATED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_REPEATED_DESIGNATED,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_ROOT,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_OTHER,
    XSTP_ENGINE_SM_PIM_STATE_PROGRESS_RECEIVE
};

enum XSTP_ENGINE_SM_PRS_STATE_E                         /* 13.32 */
{
    XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE,
    XSTP_ENGINE_SM_PRS_STATE_RECEIVE,

    /* Progress */
    XSTP_ENGINE_SM_PRS_STATE_PROGRESS_INIT_BRIDGE,
    XSTP_ENGINE_SM_PRS_STATE_PROGRESS_RECEIVE
};

enum XSTP_ENGINE_SM_PRT_STATE_E                         /* 13.33 */
{
    XSTP_ENGINE_SM_PRT_STATE_INIT_PORT,
    XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT,
    XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT,
    XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_PROPOSED,
    XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_AGREED,
    XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROPOSED,
    XSTP_ENGINE_SM_PRT_STATE_PROPOSING,
    XSTP_ENGINE_SM_PRT_STATE_AGREES,
    XSTP_ENGINE_SM_PRT_STATE_SYNCED,
    XSTP_ENGINE_SM_PRT_STATE_REROOT,
    XSTP_ENGINE_SM_PRT_STATE_FORWARD,
    XSTP_ENGINE_SM_PRT_STATE_LEARN,
    XSTP_ENGINE_SM_PRT_STATE_LISTEN,
    XSTP_ENGINE_SM_PRT_STATE_REROOTED,
    XSTP_ENGINE_SM_PRT_STATE_ROOT,
    XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT,

    /* Progress */
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_INIT_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCK_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BACKUP_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_PROPOSED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_PROPOSING,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_AGREES,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_SYNCED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_REROOT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_FORWARD,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_LEARN,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_LISTEN,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_REROOTED,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT,
    XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ACTIVE_PORT
};

enum XSTP_ENGINE_SM_PST_STATE_E                         /* 13.34 */ /* 17.24 */
{
    XSTP_ENGINE_SM_PST_STATE_DISCARDING,
    XSTP_ENGINE_SM_PST_STATE_LEARNING,
    XSTP_ENGINE_SM_PST_STATE_FORWARDING,

    /* Progress */
    XSTP_ENGINE_SM_PST_STATE_PROGRESS_DISCARDING,
    XSTP_ENGINE_SM_PST_STATE_PROGRESS_LEARNING,
    XSTP_ENGINE_SM_PST_STATE_PROGRESS_FORWARDING
};

enum XSTP_ENGINE_SM_TCM_STATE_E                         /* 13.35 */ /* 17.25 */
{
    XSTP_ENGINE_SM_TCM_STATE_INIT,
    XSTP_ENGINE_SM_TCM_STATE_INACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_ACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_DETECTED,
    XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN,
    XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC,
    XSTP_ENGINE_SM_TCM_STATE_PROPAGATING,
    XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED,

    /* Progress */
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INIT,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACTIVE,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_DETECTED,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_NOTIFIED_TCN,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_NOTIFIED_TC,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_PROPAGATING,
    XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACKNOWLEDGED
};

enum XSTP_ENGINE_PORTVAR_INFO_IS_E                      /* 13.24 */ /* 17.18.6 */
{
    XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED                = 0,
    XSTP_ENGINE_PORTVAR_INFO_IS_MINE,
    XSTP_ENGINE_PORTVAR_INFO_IS_AGED,
    XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED
};

enum XSTP_ENGINE_PORTVAR_ROLE_E                         /* 13.24.23 */
{
    XSTP_ENGINE_PORTVAR_ROLE_DISABLED                   = 0,
    XSTP_ENGINE_PORTVAR_ROLE_ROOT,
    XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED,
    XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE,
    XSTP_ENGINE_PORTVAR_ROLE_BACKUP,
    XSTP_ENGINE_PORTVAR_ROLE_MASTER
};

enum XSTP_ENGINE_PORTVAR_RCVD_INFO_E
{
    XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO,
    XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO,
    XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO,
    XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO
};

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

enum XSTP_ENGINE_SM_BDM_STATE_E                         /* 802.1D-2004 17.25*/
{
    XSTP_ENGINE_SM_BDM_STATE_EDGE,
    XSTP_ENGINE_SM_BDM_STATE_NOT_EDGE,

    /* Progress */
    XSTP_ENGINE_SM_BDM_STATE_PROGRESS_EDGE,
    XSTP_ENGINE_SM_BDM_STATE_PROGRESS_NOT_EDGE,
};

/* ===================================================================== */
/* ===================================================================== */
/* State Machine Functions */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitAllInstances
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize all the instances
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_ENGINE_InitAllInstances(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the state machines
 * INPUT    : om_ptr    -- om pointer for this instance
 *            xstid     -- Spanning tree identifier
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_ENGINE_InitStateMachine(XSTP_OM_InstanceData_T *om_ptr, UI32_T xstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitPortStateMachines
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the port state machine variables
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- logical port number
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_InitPortStateMachines(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_ENGINE_PTIStateMachineProgress
 * PURPOSE  : To motivate the port timer state machine
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : TRUE if any of the timers is expired, else FALSE
 * NOTES    : None
 */
BOOL_T  XSTP_ENGINE_PTIStateMachineProgress(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_ENGINE_StateMachineProgress
 * PURPOSE  : To motivate the port/bridge state machines
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_StateMachineProgress(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_CheckStateMachineBegin
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the state machine begin status and reset it
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine is at the begin status, else FALSE
 * NOTES    : None
 */
BOOL_T  XSTP_ENGINE_CheckStateMachineBegin(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the state machine begin status and reset it
 * INPUT    : om_ptr        -- om pointer for this instance
 *            learning      --
 *            forwarding    --
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_SetPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, BOOL_T learning, BOOL_T forwarding);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_SetMstEnableStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the state machine begin status and reset it
 * INPUT    : state         --
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_SetMstEnableStatus(BOOL_T state);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_RestartStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : State machine restart means read configuration again,
 *            not clear all the configuration parameters and use default.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : Call the function when user change MST Configuration Indentifier.
 */
void XSTP_ENGINE_RestartStateMachine(void);
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

#endif /* _XSTP_ENGINE_H */
