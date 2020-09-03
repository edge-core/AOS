/*-----------------------------------------------------------------------------
 * FILE NAME: poe_engine.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    Implement IEEE802.3at Layer2 classification state machine
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    12/13/2007 - Daniel Chen, Created
 *     4/30/2008 - Add new state 'Disable' for manual high-power mode
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "poe_task.h"
#include "poe_mgr.h"
#include "poe_engine.h"
#include "leaf_3621.h"
#include "poe_backdoor.h"
#if 0 /* Eugene temp */
#include "lldp_mgr_notify.h"
#endif
#include "poe_om.h"
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "swctrl_pom.h"
#include "stktplg_pom.h"
#include "lldp_pmgr.h"
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
#include "stktplg_board.h"
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/* NAMING CONSTANT DECLARATIONS
 */
#define POE_SM_MAXIMUN_TIMER_EXPIRED_TIME1 300  /* interval max: 30 sec, hold time max: 10 sec*/
#define POE_SM_DEFAULT_TIMER_EXPIRED_TIME2 0    /* unit: second */

#define REMOTE_ACK 1
#define REMOTE_NACK 2
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define POE_DEBUG(fmt...) \
    { \
        if(POE_BACKDOOR_IsDebugMsgOn()==TRUE) \
            {printf(fmt);} \
    }

#define POE_STATE_STR(x) poe_dot3at_state_description[x]

#define POE_ENGINE_ENTER_CRITICAL_SECTION SYSFUN_ENTER_CRITICAL_SECTION(poe_dot3at_semaphore_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POE_ENGINE_LEAVE_CRITICAL_SECTION SYSFUN_LEAVE_CRITICAL_SECTION(poe_dot3at_semaphore_id)

/* Eugene temp */
#if 1
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif
/* End of Eugene temp */

/* DATA TYPE DECLARATIONS
 */

/* the state defined in Dot3at
 */
enum POE_ENGINE_DOT3AT_STATE_E
{
    POE_STATE_MACHINE_3AT_DISABLE = 0, /* state machine is disabled */
    POE_STATE_MACHINE_3AT_IDLE,
    POE_STATE_MACHINE_3AT_INITIAL,
    POE_STATE_MACHINE_3AT_RUNNING,
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    POE_STATE_MACHINE_3AT_REMOTE_REQUEST,
    POE_STATE_MACHINE_3AT_REMOTE_ACK,
    POE_STATE_MACHINE_3AT_REMOTE_NACK,
    POE_STATE_MACHINE_3AT_WAIT_FOR_REMOTE,
    POE_STATE_MACHINE_3AT_LOCAL_REQUEST,
    POE_STATE_MACHINE_3AT_LOCAL_ACK,
    POE_STATE_MACHINE_3AT_LOCAL_NACK,
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    POE_STATE_MACHINE_3AT_PD_POWER_REQUEST,
    POE_STATE_MACHINE_3AT_PSE_POWER_REVIEW,
    POE_STATE_MACHINE_3AT_PSE_POWER_REALLOCATION,
    POE_STATE_MACHINE_3AT_MIRROR_UPDATE,
#endif
};

/* the result of determining remote requested
 */
enum POE_ENGINE_REMOTE_REQUEST_E
{
    POE_ENGINE_REMOTE_REQUEST_SUCCESS = 0,
    POE_ENGINE_REMOTE_REQUEST_PORT_POWERLIMIT_FAIL,
    POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL,
    POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_LESS_THAN_PORT_POWERLIMIT
};

enum POE_ENGINE_STATE_CHG_REASON_E
{
    POE_ENGINE_MANUAL_HIGH_POWER = 0,
    POE_ENGINE_POEDRV_CHANGE,
    POE_ENGINE_HANDLE_LLDP_INFO,
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    POE_ENGINE_TIMER1_EXPIRED,
    POE_ENGINE_TIMER2_EXPIRED,
    POE_ENGINE_TIMER_EVENT,
#endif
    POE_ENGINE_LOCAL_CHANGE,
    POE_ENGINE_BACKDOOR_USER_CHANGED,
};

/* used variables in state machine
 */
typedef struct POE_ENGINE_PORT_DOT3AT_VARIABLE_S
{
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    UI32_T locRequestPowerAcknowledge;
    UI32_T locActualPowerValue;
    UI32_T locRequestedPowerValue;
    UI32_T remActualPowerValue;
    UI32_T remRequestedPowerValue;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    UI32_T PSEAllocatedPowerValue;
    UI32_T MirroredPSEAllocatedPowerValueEcho;
    UI32_T PDRequestedPowerValueEcho;
    UI32_T MirroredPDRequestedPowerValue;
#endif
}POE_ENGINE_PORT_DOT3AT_VARIABLE_T;

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/* Timer type will change disable to type1 when recognized PD support DLL
 * change type1 to type2 when type 1 expired
 * change to disable when port reset
 */
enum POE_ENGINE_DOT3AT_L2_TIMER_E
{
    POE_ENGINE_DOT3AT_TIMER_TYPE_DISABLE = 0,
    POE_ENGINE_DOT3AT_TIMER_TYPE1,
    POE_ENGINE_DOT3AT_TIMER_TYPE2
};

typedef struct
{
    UI16_T time; /* second */
    UI16_T type; /* disable, timer1, timer2 */
}POE_ENGINE_DOT3AT_L2_TIMER_T;
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T POE_ENGINE_CheckRemoteRequest(UI32_T unit, UI32_T port, UI32_T reqPowerValue, UI32_T* newPowervlaue);
static void POE_ENGINE_SetPortState(UI32_T unit, UI32_T port, UI32_T state, UI32_T reason);
static BOOL_T POE_ENGINE_SetPortVariable(UI32_T unit, UI32_T port, POE_ENGINE_PORT_DOT3AT_VARIABLE_T *pInfo);
static BOOL_T POE_ENGINE_GetPortVariable(UI32_T unit, UI32_T port, POE_ENGINE_PORT_DOT3AT_VARIABLE_T *pInfo);


/* STATIC VARIABLE DECLARATIONS
 */

/* variable for state machine */
static POE_ENGINE_PORT_DOT3AT_VARIABLE_T poe_dot3at_port_info[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];

/* used to recored the current state */
static UI32_T poe_dot3at_port_state[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];


#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/* timer of each poe port */
static POE_ENGINE_DOT3AT_L2_TIMER_T poe_dot3at_port_timer[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];

/* timeout value to keep last negotiation state 
 */
static UI16_T poe_dot3at_expired_timer1[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];

/* timeout value to remove power, 0 means never expired 
 * timer2 should start when timer1 expired
 */
static UI16_T poe_dot3at_expired_timer2[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];
#endif

/* used to protect critical section
 */
static UI32_T poe_dot3at_semaphore_id;

/* message to describe all state */
static char *poe_dot3at_state_description[] = 
{
    "DISABLE",
    "IDLE",
    "INITIAL",
    "RUNNING",
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    "REMOTE_REQUEST",
    "REMOTE_ACK",
    "REMOTE_NACK",
    "WAIT_FOR_REMOTE",
    "LOCAL_REQUEST",
    "LOCAL_ACK",
    "LOCAL_NACK",
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    "PD_POWER_REQUEST",
    "PSE_POWER_REVIEW",
    "PSE_POWER_REALLOCATION",
    "MIRROR_UPDATE",
#endif
};

static char *poe_state_change_reason_desc[]=
{
    "MANUAL_HIGH_POWER",
    "POEDRV_CHANGE",
    "HANDLE_LLDP_INFO",
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    "TIMER1_EXPIRED",
    "TIMER2_EXPIRED",
    "TIMER_EVENT",
#endif
    "LOCAL_CHANGE",
    "BACKDOOR_USER_CHANGED",
};

/* EXPORTED SUBPROGRAM BODIES
 */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_InitialSystemResource
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to iniail the all needed resource.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_ENGINE_InitialSystemResource(void)
{
    UI32_T unit,port;

    memset(poe_dot3at_port_info, 0, sizeof(POE_ENGINE_PORT_DOT3AT_VARIABLE_T)*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT);

    for (unit=1;unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit++)
    {
        for (port=1;port<=SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT;port++)
        {
            poe_dot3at_port_state[unit-1][port-1] = POE_STATE_MACHINE_3AT_IDLE; /* default state machine is enabled (idle state) */
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
            poe_dot3at_port_timer[unit-1][port-1].type = POE_ENGINE_DOT3AT_TIMER_TYPE_DISABLE;
            poe_dot3at_port_timer[unit-1][port-1].time = 0;
            poe_dot3at_expired_timer1[unit-1][port-1] = POE_SM_MAXIMUN_TIMER_EXPIRED_TIME1;
            poe_dot3at_expired_timer2[unit-1][port-1] = POE_SM_DEFAULT_TIMER_EXPIRED_TIME2;
#endif
        }
    }

    /* Create semaphore
     */
    if (SYSFUN_CreateSem (SYSFUN_SEMKEY_PRIVATE,1/* SEM_FULL */, SYSFUN_SEM_FIFO, &poe_dot3at_semaphore_id) != SYSFUN_OK)
    {
        printf("\n\rCreate poe_dot3at_semaphore_id failed.");
    }
} /* end POE_ENGINE_InitialSystemResource */


#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_LocalSystemChange
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is called when the local system change.
 * INPUT    : unit - unit number
 *            port - port number
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_LocalSystemChange(UI32_T unit, UI32_T port)
{
    UI32_T lport, result, pse_new_value, i;
    BOOL_T ret;
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;

    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT; i++)
    {
        if (port != 0)
			i = port;

        if (poe_dot3at_port_state[unit-1][i-1] == POE_STATE_MACHINE_3AT_RUNNING)
        {
            POE_ENGINE_GetPortVariable(unit, i, &dot3at_info);

            POE_ENGINE_SetPortState(unit ,i, 
                                       POE_STATE_MACHINE_3AT_PSE_POWER_REVIEW, 
                                       POE_ENGINE_LOCAL_CHANGE);

            result = POE_ENGINE_CheckRemoteRequest(unit, i, dot3at_info.PSEAllocatedPowerValue, &pse_new_value);
            switch (result)
            {
                case POE_ENGINE_REMOTE_REQUEST_SUCCESS:                /* budget is enough, do nothing */
                    POE_DEBUG("PSE power review ok for ethernet port %lu/%lu\n",unit,i);

                    POE_ENGINE_SetPortState(unit ,i, 
                                               POE_STATE_MACHINE_3AT_RUNNING, 
                                               POE_ENGINE_LOCAL_CHANGE);
                    break;

                case POE_ENGINE_REMOTE_REQUEST_PORT_POWERLIMIT_FAIL:   /* budget is not enough, reallocate */
                case POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL:
                case POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_LESS_THAN_PORT_POWERLIMIT:
                    if ((dot3at_info.PSEAllocatedPowerValue == dot3at_info.MirroredPSEAllocatedPowerValueEcho) &&
	                    (pse_new_value < dot3at_info.PSEAllocatedPowerValue))
                    {
                        POE_DEBUG("PSE power review fail for ethernet port %lu/%lu, reallocating...\n",unit,i);

                        POE_ENGINE_SetPortState(unit, i, 
                                                   POE_STATE_MACHINE_3AT_PSE_POWER_REALLOCATION,
                                                   POE_ENGINE_LOCAL_CHANGE);

                        dot3at_info.PSEAllocatedPowerValue = pse_new_value;
                        POE_ENGINE_SetPortVariable(unit, i, &dot3at_info);

                        if (pse_new_value > 130)  /* 13.0 watts */
                        {
                            ret = POE_MGR_SetPortDot3atHighPowerMode(unit, i, 1); /* high power */
                            POE_DEBUG("Ethernet port %lu/%lu is in 802.3at high power mode\n",unit,i);
                        }
                        else
                        {
                            ret = POE_MGR_SetPortDot3atHighPowerMode(unit, i, 0); /* normal mode */
                            POE_DEBUG("Ethernet port %lu/%lu is in 802.3at normal mode\n",unit,i);
                        }

                        if (ret == FALSE)
                        {
                            POE_DEBUG("POE_MGR_SetPortDot3atHighPowerMode ethernet port %lu/%lu Error\n",unit,i);
                        }

                        POE_ENGINE_SetPortState(unit, i, 
                                                   POE_STATE_MACHINE_3AT_MIRROR_UPDATE, 
                                                   POE_ENGINE_LOCAL_CHANGE);

                        SWCTRL_POM_UserPortToLogicalPort(unit, i, &lport);
                        LLDP_PMGR_NotifyPseTableChanged(lport);

                        POE_ENGINE_SetPortState(unit, i, 
                                                   POE_STATE_MACHINE_3AT_RUNNING, 
                                                   POE_ENGINE_LOCAL_CHANGE);
                    }
	                else
                    {
                        POE_DEBUG("PSE power review fail for ethernet port %lu/%lu. But condition is not satisfied, back to running state.\n",unit,i);

                        POE_ENGINE_SetPortState(unit, i, 
                                                   POE_STATE_MACHINE_3AT_RUNNING,
                                                   POE_ENGINE_LOCAL_CHANGE);
                    }

                    break;

                default:
                    POE_DEBUG("(%s)power review error return\n", __FUNCTION__);

                    break;
            }
        }
        else
        {
            POE_DEBUG("local system change in error state\n");
        }

        if (port != 0)
			break;
   	}

    return TRUE;
}
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is called when the period timer event occur. (1 seconed)
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ProcessTimerEvent(void)
{
    UI32_T unit,port;
    UI32_T power_value;
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;
    POE_ENGINE_DOT3AT_L2_TIMER_T *ptrTimer;
    
    for (unit=1;unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit++)
    {
        if (!STKTPLG_POM_UnitExist(unit))
            continue;

        for (port=1;port<=SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT;port++)
        {
            if (!SWCTRL_POM_UserPortExisting(unit, port))
                continue;
		
            POE_ENGINE_GetPortVariable(unit, port, &dot3at_info);
            ptrTimer = &poe_dot3at_port_timer[unit-1][port-1];
        
            if (ptrTimer->type != POE_ENGINE_DOT3AT_TIMER_TYPE_DISABLE)
            {
                ptrTimer->time++;

                switch (poe_dot3at_port_state[unit-1][port-1])
                {
                    case POE_STATE_MACHINE_3AT_INITIAL:
                    case POE_STATE_MACHINE_3AT_RUNNING:
                    case POE_STATE_MACHINE_3AT_REMOTE_REQUEST:
                    case POE_STATE_MACHINE_3AT_REMOTE_ACK:
                    case POE_STATE_MACHINE_3AT_REMOTE_NACK:
                    case POE_STATE_MACHINE_3AT_WAIT_FOR_REMOTE:
                    case POE_STATE_MACHINE_3AT_LOCAL_REQUEST:
                    case POE_STATE_MACHINE_3AT_LOCAL_ACK:
                    case POE_STATE_MACHINE_3AT_LOCAL_NACK:
                    {
                        /* timer 2 expired, remove power from port */
                        if ((poe_dot3at_expired_timer2[unit-1][port-1] != 0) && 
                            (ptrTimer->type == POE_ENGINE_DOT3AT_TIMER_TYPE2) &&
                            (ptrTimer->time >= poe_dot3at_expired_timer2[unit-1][port-1]))
                        {
                            POE_DEBUG("ethernet port %lu/%lu timer2 expired\n",unit,port);

                            /* Reset the PSE function of port */
                            POE_MGR_ResetPort(unit, port);

                            /* Set the power mode as normal mode */
                            POE_MGR_SetPortDot3atHighPowerMode(unit, port, 0);

                            dot3at_info.locActualPowerValue = 0;
                            dot3at_info.locRequestedPowerValue = 0;
                            dot3at_info.remActualPowerValue = 0;
                            dot3at_info.remRequestedPowerValue = 0;
                            POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

                            POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_IDLE, POE_ENGINE_TIMER2_EXPIRED);

                            /* Reset the timer */
                            ptrTimer->time = 0;
                            ptrTimer->type = POE_ENGINE_DOT3AT_TIMER_TYPE_DISABLE;

                        }
                    
                        /* timer 1 expired , back to running state */
                        if (ptrTimer->type == POE_ENGINE_DOT3AT_TIMER_TYPE1 &&
                            ptrTimer->time >= poe_dot3at_expired_timer1[unit-1][port-1])
                        {
                            POE_DEBUG("ethernet port %lu%lu timer1 expired\n",unit,port);
                            if (POE_OM_GetPsePortPowerConsumption(unit, port, &power_value) == FALSE)
                            {
                                power_value = 0;
                            }
                            dot3at_info.locActualPowerValue = power_value/100;
                            dot3at_info.locRequestedPowerValue = dot3at_info.locActualPowerValue;
                            dot3at_info.remActualPowerValue = 0;
                            dot3at_info.remRequestedPowerValue = 0;
                            POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

                            POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_RUNNING, POE_ENGINE_TIMER1_EXPIRED);

                            SWCTRL_POM_UserPortToLogicalPort(unit, port, &lport);
                            LLDP_PMGR_NotifyPseTableChanged(lport);

                            /* enable timer2 */
                            ptrTimer->type = POE_ENGINE_DOT3AT_TIMER_TYPE2;
                            ptrTimer->time = 0;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
	
	return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_RefreshTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to refresh timer
 * INPUT    : unit, port
 * OUTPUT   : None
 * RETURN   : None
 * Note     : This function should be called when receive the power control frame
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_RefreshTimer(UI32_T unit, UI32_T port)
{
    /* don't touch timer when state is 'disable' */
    if (poe_dot3at_port_state[unit-1][port-1] == POE_STATE_MACHINE_3AT_DISABLE)
    {
        POE_DEBUG("warning ! touch timer in ethernet port %lu/%lu disable state !! ",unit,port);
        return TRUE;
    }

    poe_dot3at_port_timer[unit-1][port-1].type = POE_ENGINE_DOT3AT_TIMER_TYPE1;
    poe_dot3at_port_timer[unit-1][port-1].time = 0;

    POE_DEBUG("ethernet port %lu/%lu touch timer\n",unit,port);

    poe_dot3at_expired_timer1[unit-1][port-1] = POE_SM_MAXIMUN_TIMER_EXPIRED_TIME1;

    return TRUE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ProcessLLDPInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function handle the state machine while receiving
 *            LLDP frame from Powerd Device
 * INPUT    : unit - unit number
 *            port - port number
 *            info - the information passed by LLDP
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ProcessLLDPInfo(UI32_T unit, UI32_T port, POE_TYPE_Dot3atPowerInfo_T *info)
{
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;
    UI32_T lport,port_status;
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    UI32_T power_used;
#endif
    BOOL_T ret;

    if (info == NULL)
        return FALSE;


    POE_ENGINE_GetPortVariable(unit, port, &dot3at_info);

    switch (poe_dot3at_port_state[unit-1][port-1])
    {
        case POE_STATE_MACHINE_3AT_DISABLE:
        {
            POE_DEBUG("%s in 'disable' state ! (ethernet port : %lu/%lu)\n", __FUNCTION__,unit,port);
            break;
        }

        /* receive LLDP frame in this state, it means the remtoe is type2 PD,
         * We start the state machine
         */
        case POE_STATE_MACHINE_3AT_IDLE:
        {
            if (POE_OM_GetPsePortDetectionStatus(unit, port, &port_status) == TRUE)
            {
                /* If the LLDP frame received first than Poedrv polling, the port 
                 * status may be VAL_pethPsePortDetectionStatus_searching
                 */
                if (port_status == VAL_pethPsePortDetectionStatus_deliveringPower || 
                    port_status == VAL_pethPsePortDetectionStatus_searching)
                {
                    POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_INITIAL, POE_ENGINE_HANDLE_LLDP_INFO);
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                    dot3at_info.locActualPowerValue = 0;
                    dot3at_info.locRequestedPowerValue = 0;
                    dot3at_info.remActualPowerValue = 0;
                    dot3at_info.remRequestedPowerValue = 0;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                    dot3at_info.PSEAllocatedPowerValue = 0;
                    dot3at_info.MirroredPSEAllocatedPowerValueEcho = 0;
                    dot3at_info.PDRequestedPowerValueEcho = 0;
                    dot3at_info.MirroredPDRequestedPowerValue = 0;
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                    if (POE_OM_GetPsePortPowerConsumption(unit, port, &power_used))
                    {
                        dot3at_info.locActualPowerValue = power_used/100;
                        dot3at_info.locRequestedPowerValue = dot3at_info.locActualPowerValue;
                    }
#endif

                    POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);
                    POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_RUNNING, POE_ENGINE_HANDLE_LLDP_INFO);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                    if (info->requested_power_value != info->power_value)
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                    if (info->pd_requested_power != info->pse_allocated_power)
#endif
                    {
                        POE_ENGINE_ProcessLLDPInfo(unit, port, info);
                    }
                }
            }
            break;
        }

        /* shouldn't receive frame in these states
         */
        case POE_STATE_MACHINE_3AT_INITIAL:
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
        case POE_STATE_MACHINE_3AT_REMOTE_REQUEST:
        case POE_STATE_MACHINE_3AT_REMOTE_ACK:
        case POE_STATE_MACHINE_3AT_REMOTE_NACK:
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
        case POE_STATE_MACHINE_3AT_PD_POWER_REQUEST:
        case POE_STATE_MACHINE_3AT_PSE_POWER_REALLOCATION:
        case POE_STATE_MACHINE_3AT_MIRROR_UPDATE:
        case POE_STATE_MACHINE_3AT_PSE_POWER_REVIEW:
#endif
        {
            POE_DEBUG("(%s)Receive LLDP frame in un-expected state: %s\n", __FUNCTION__, POE_STATE_STR(poe_dot3at_port_state[unit-1][port-1]));
            break;
        }

        case POE_STATE_MACHINE_3AT_RUNNING:
        {
            UI32_T result, pse_new_value;

            dot3at_info.MirroredPDRequestedPowerValue = info->pd_requested_power;
            dot3at_info.MirroredPSEAllocatedPowerValueEcho= info->pse_allocated_power;
            POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
            /* remoteRequestValue != remoteActualValue
             */
            if (info->requested_power_value != info->power_value)
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
            if ((dot3at_info.PDRequestedPowerValueEcho != dot3at_info.MirroredPDRequestedPowerValue) &&
				(dot3at_info.PSEAllocatedPowerValue == dot3at_info.MirroredPSEAllocatedPowerValueEcho))
#endif
            {

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_REMOTE_REQUEST, POE_ENGINE_HANDLE_LLDP_INFO);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_PD_POWER_REQUEST, POE_ENGINE_HANDLE_LLDP_INFO);
#endif

                /* To determine the result of remote request
                 */
                result = POE_ENGINE_CheckRemoteRequest(unit, port,
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                                                          info->requested_power_value, &pse_new_value);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                                                          info->pd_requested_power, &pse_new_value);
#endif
                switch (result)
                {
                    case POE_ENGINE_REMOTE_REQUEST_SUCCESS:
                    {
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                        dot3at_info.locActualPowerValue = info->requested_power_value;
                        dot3at_info.locRequestPowerAcknowledge = REMOTE_ACK;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                        dot3at_info.PSEAllocatedPowerValue = pse_new_value;
#endif
                        POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

                        POE_ENGINE_SetPortState(unit, port, 
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                                                   POE_STATE_MACHINE_3AT_REMOTE_ACK,
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                                                   POE_STATE_MACHINE_3AT_PSE_POWER_REALLOCATION,
#endif
                                                   POE_ENGINE_HANDLE_LLDP_INFO);

                        /* Notify LLDP_MGR to send ACK frame
                         */
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                        POE_DEBUG("Ack to ethernet port %lu/%lu remote request\n",unit,port);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                        POE_DEBUG("Accept to ethernet port %lu/%lu remote request\n",unit,port);
#endif

                        /* Change the power normal to High-Power mode or Normal mode
                         * according to the requested power value
                         */
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                        if (info->requested_power_value > 154)  /* 15.4 watts */
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                        if (info->pd_requested_power > 130)  /* 13.0 watts */
#endif
                        {
                            ret = POE_MGR_SetPortDot3atHighPowerMode(unit, port, 1); /* high power */
                            POE_DEBUG("Ethernet port %lu/%lu is in 802.3at high power mode\n",unit,port);
                        }
                        else
                        {
                            ret = POE_MGR_SetPortDot3atHighPowerMode(unit, port, 0); /* normal mode */
                            POE_DEBUG("Ethernet port %lu/%lu is in 802.3at normal mode\n",unit,port);
                        }
/* Timer */

                        if (ret == FALSE)
                        {
                            POE_DEBUG("POE_MGR_SetPortDot3atHighPowerMode ethernet port %lu/%lu Error\n",unit,port);
                        }

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                        SWCTRL_POM_UserPortToLogicalPort(unit, port, &lport);
                        LLDP_PMGR_NotifyPseTableChanged(lport);

                        POE_ENGINE_SetPortState(unit, port, 
                                                  POE_STATE_MACHINE_3AT_WAIT_FOR_REMOTE,
                                                  POE_ENGINE_HANDLE_LLDP_INFO);
#endif
                        break;
                    }
                    case POE_ENGINE_REMOTE_REQUEST_PORT_POWERLIMIT_FAIL:
                    case POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL:
                    case POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_LESS_THAN_PORT_POWERLIMIT:
                    {
                        char *err_msg[] = { "None", 
                                        "POE_ENGINE_REMOTE_REQUEST_PORT_POWERLIMIT_FAIL",
                                        "POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL", 
                                        "POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_LESS_THAN_PORT_POWERLIMIT"};

                        POE_DEBUG("(%s) %s\n", __FUNCTION__, err_msg[result]);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                        dot3at_info.locRequestPowerAcknowledge = REMOTE_NACK;
                        POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

                        POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_REMOTE_NACK, POE_ENGINE_HANDLE_LLDP_INFO);

                        /* Notify LLDP_MGR to send NACK frame
                         */
                        POE_DEBUG("NAck to ethernet port %lu/%lu remote request\n",unit,port);

                        SWCTRL_POM_UserPortToLogicalPort(unit, port, &lport);
                        LLDP_PMGR_NotifyPseTableChanged(lport);

                        POE_ENGINE_SetPortState(unit ,port, 
                                                  POE_STATE_MACHINE_3AT_WAIT_FOR_REMOTE, 
                                                  POE_ENGINE_HANDLE_LLDP_INFO);

#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                        dot3at_info.PSEAllocatedPowerValue = pse_new_value;
                        POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

                        POE_DEBUG("Reject to ethernet port %lu/%lu remote request\n",unit,port);
#endif
                        break;
                    }
                    default:
                        POE_DEBUG("(%s)remote request error return\n", __FUNCTION__);
                        break;
                }
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                POE_ENGINE_SetPortState(unit ,port, 
                                           POE_STATE_MACHINE_3AT_MIRROR_UPDATE, 
                                           POE_ENGINE_HANDLE_LLDP_INFO);

                dot3at_info.PDRequestedPowerValueEcho = info->pd_requested_power;
                POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);

                SWCTRL_POM_UserPortToLogicalPort(unit, port, &lport);
                LLDP_PMGR_NotifyPseTableChanged(lport);

                POE_ENGINE_SetPortState(unit ,port, 
                                           POE_STATE_MACHINE_3AT_RUNNING, 
                                           POE_ENGINE_HANDLE_LLDP_INFO);
#endif

            }
            else
            {
                POE_DEBUG("Receive an LLDP frame for ethernet port %lu/%lu. But condition is not satisfied, stay in running state.\n",unit,port);
            }
            break;
        }

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
        case POE_STATE_MACHINE_3AT_WAIT_FOR_REMOTE:
        {
            /* (remRequestedPowerValue == remActualPowerValue) normal condition
             * ==> running state
             */
            if (info->requested_power_value == info->power_value)
            {
                POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_RUNNING, POE_ENGINE_HANDLE_LLDP_INFO);
                /* ptrPort->locRequestPowerAcknowledge = 0; */
            }
            else 
            {
                /* This conidition shouldn't be happened,
                 * Unless we lost normal frame
                 * Change to running state , and handle this request again
                 */
                POE_DEBUG("Receive a power value requested frame in %s state\n", POE_STATE_STR(poe_dot3at_port_state[unit-1][port-1]));
                /* ptrPort->locRequestPowerAcknowledge = 0; */
                POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_RUNNING, POE_ENGINE_HANDLE_LLDP_INFO);
                POE_ENGINE_ProcessLLDPInfo(unit, port, info);
            }
            break;
        }

        /* Don't handle local request at present */
        case POE_STATE_MACHINE_3AT_LOCAL_REQUEST:
        case POE_STATE_MACHINE_3AT_LOCAL_ACK:
        case POE_STATE_MACHINE_3AT_LOCAL_NACK:
        {
            POE_DEBUG("(%s)Receive LLDP frame in un-expected state: %s\n", __FUNCTION__, POE_STATE_STR(poe_dot3at_port_state[unit-1][port-1]));
            break;
        }
#endif
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ProcessPortStateChange
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will reset the state machine when port state
 *            changed
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ProcessPortStateChange(UI32_T unit, UI32_T port)
{
    BOOL_T ret;
    UI32_T port_status;
    UI32_T lport;
//    UI32_T power_used;
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    POE_ENGINE_DOT3AT_L2_TIMER_T *ptrTimer;
#endif

    if (FALSE == POE_OM_UserPortExisting(unit,port))
    {
        return FALSE;
    }

    /* don't handle attributes in 'Disable' state */
    if (poe_dot3at_port_state[unit-1][port-1] == POE_STATE_MACHINE_3AT_DISABLE)
    {
        return TRUE;
    }

    ret = POE_OM_GetPsePortDetectionStatus(unit, port, &port_status);
    if (ret == FALSE)
    {
        return FALSE;
    }
    else
    {
        switch (port_status)
        {
            case VAL_pethPsePortDetectionStatus_disabled:
            case VAL_pethPsePortDetectionStatus_fault:
            case VAL_pethPsePortDetectionStatus_test:
            case VAL_pethPsePortDetectionStatus_otherFault:
            case VAL_pethPsePortDetectionStatus_searching:
            {
                /* enter idle state */
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                dot3at_info.locActualPowerValue = 0;
                dot3at_info.locRequestedPowerValue = 0;
                dot3at_info.remActualPowerValue = 0;
                dot3at_info.remRequestedPowerValue = 0;
                dot3at_info.locRequestPowerAcknowledge = 0;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                dot3at_info.PSEAllocatedPowerValue = 0;
                dot3at_info.MirroredPSEAllocatedPowerValueEcho = 0;
                dot3at_info.PDRequestedPowerValueEcho = 0;
                dot3at_info.MirroredPDRequestedPowerValue = 0;
#endif
                POE_ENGINE_SetPortVariable(unit,port,&dot3at_info);
                POE_ENGINE_SetPortState(unit,port,POE_STATE_MACHINE_3AT_IDLE,POE_ENGINE_POEDRV_CHANGE);

                /* Disable the High-Power Mode */
                POE_MGR_SetPortDot3atHighPowerMode(unit,port,0);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                ptrTimer = &poe_dot3at_port_timer[unit-1][port-1];

                ptrTimer->time = 0;
                ptrTimer->type = POE_ENGINE_DOT3AT_TIMER_TYPE_DISABLE;
#endif

                SWCTRL_POM_UserPortToLogicalPort(unit, port, &lport);
                LLDP_PMGR_NotifyPseTableChanged(lport);

                break;
            }
            case VAL_pethPsePortDetectionStatus_deliveringPower:
            {
                /* do nothing */
                POE_DEBUG("ethernet port %lu/%lu changed to delivering-power mode\n",unit,port);
                break;
            }
            default:
                POE_DEBUG("%s error\n", __FUNCTION__);
                break;
        }
    }

	return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_GetPortDot3atPowerInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get all infomation for LLDP tlvs of PoE, For LLDPDU tx
 * INPUT    : unit, port
 * OUTPUT   : None
 * RETURN   : None
 * Note     :
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_GetPortDot3atPowerInfo(UI32_T unit, UI32_T port, POE_TYPE_Dot3atPowerInfo_T *info)
{
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;
    UI32_T value;
    BOOL_T local_power;
	
    /* Get port priority */
    if (FALSE==POE_OM_GetPsePortPowerPriority(unit,port,&value))
    {
        return FALSE;
    }

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    info->power_priority = value;
	info->requested_power_priority = value;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    switch (value)
    {
        case VAL_pethPsePortPowerPriority_critical:
            info->power_priority = POE_ENGINE_TLV_POWER_PRIORITY_CRITICAL;
            break;
        case VAL_pethPsePortPowerPriority_high:
            info->power_priority = POE_ENGINE_TLV_POWER_PRIORITY_HIGH;
            break;
        case VAL_pethPsePortPowerPriority_low:
            info->power_priority = POE_ENGINE_TLV_POWER_PRIORITY_LOW;
            break;
        default:
            info->power_priority = POE_ENGINE_TLV_POWER_PRIORITY_UNKNOW;
            return FALSE;
    }
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    info->power_type = 0; /* PSE */
    info->requested_power_type = 0; /* PSE */
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    info->power_type = POE_ENGINE_TLV_POWER_TYPE_TYPE2_PSE; /* type-2 PSE */
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    info->power_source = 1; /* Primary power source */
    info->requested_power_source = 1; /* Primary power source */
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    POE_OM_GetUseLocalPower(unit, &local_power);
#else
    local_power = TRUE;
#endif

    if (local_power)
        info->power_source = POE_ENGINE_TLV_POWER_SOURCE_PSE_PRIMARY; /* Primary power source */
    else
        info->power_source = POE_ENGINE_TLV_POWER_SOURCE_PSE_BACKUP; /* RPS */
#endif

    POE_ENGINE_GetPortVariable(unit,port,&dot3at_info);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    info->requested_power_value = dot3at_info.locRequestedPowerValue ;
    info->power_value = dot3at_info.locActualPowerValue;
    info->acknowledge = dot3at_info.locRequestPowerAcknowledge;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    info->pd_requested_power = dot3at_info.PDRequestedPowerValueEcho;
    info->pse_allocated_power = dot3at_info.PSEAllocatedPowerValue;
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    /* No matter the ack value is ack or nack, 
     * We reset the ack value as 0
     */
    if (dot3at_info.locRequestPowerAcknowledge != 0)
    {
        dot3at_info.locRequestPowerAcknowledge = 0;
        POE_ENGINE_SetPortVariable(unit,port,&dot3at_info);
    }
#endif
    return TRUE;
}

/* called by backdoor */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_DumpDot3atInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Dump all state variable
 * INPUT    : unit: 0 - all unit, port: 0 - all port, else logical port
 * OUTPUT   : None
 * RETURN   : None
 * Note     :
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_DumpDot3atInfo(UI32_T unit,UI32_T port)
{
    UI32_T i,j;
	BOOL_T error = 0;
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    POE_ENGINE_DOT3AT_L2_TIMER_T *ptrTimer;
#endif

    if (unit == 0)
    {
        for (i=1;i<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
        {
            if (port == 0)
            {
                for (j=1;j<SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT;j++)
                {
                    if (POE_OM_UserPortExisting(i,j)==TRUE)
                    {
                        POE_ENGINE_GetPortVariable(i,j,&dot3at_info);
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                        ptrTimer = &poe_dot3at_port_timer[i-1][j-1];
                        printf("\n===========================================\n");
                        printf("ETHERNET PORT %lu/%lu STATE: %s\n", i, j, POE_STATE_STR(poe_dot3at_port_state[i-1][j-1]));
                        printf("LOCAL actual: %lu, requested: %lu, ack: %lu\n", dot3at_info.locActualPowerValue, 
                            dot3at_info.locRequestedPowerValue, dot3at_info.locRequestPowerAcknowledge);
                        printf("REMOTE actual: %lu, requested: %lu\n", dot3at_info.remActualPowerValue, dot3at_info.remRequestedPowerValue);
                        printf("timer type: %u, timer value: %u\n", ptrTimer->type, ptrTimer->time);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                        printf("\n===========================================\n");
                        printf("ETHERNET PORT %lu/%lu STATE: %s\n", i, j, POE_STATE_STR(poe_dot3at_port_state[i-1][j-1]));
                        printf("         PSE allocated: %lu, PD requested: %lu\n", dot3at_info.PSEAllocatedPowerValue, dot3at_info.PDRequestedPowerValueEcho);
                        printf("Mirrored PSE allocated: %lu, PD requested: %lu\n", dot3at_info.MirroredPSEAllocatedPowerValueEcho, dot3at_info.MirroredPDRequestedPowerValue);
#endif
                    }
                }
            }
            else
            {
                if (POE_OM_UserPortExisting(i,port)==TRUE)
                {
                    POE_ENGINE_GetPortVariable(i,port,&dot3at_info);
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                    ptrTimer = &poe_dot3at_port_timer[i-1][port-1];
                    printf("\n===========================================\n");
                    printf("PORT ETHERNET %lu/%lu STATE: %s\n", i, port, POE_STATE_STR(poe_dot3at_port_state[i-1][port-1]));
                    printf("LOCAL actual: %lu, requested: %lu, ack: %lu\n", dot3at_info.locActualPowerValue, 
                        dot3at_info.locRequestedPowerValue, dot3at_info.locRequestPowerAcknowledge);
                    printf("REMOTE actual: %lu, requested: %lu\n", dot3at_info.remActualPowerValue, dot3at_info.remRequestedPowerValue);
                    printf("timer type: %u, timer value: %u\n", ptrTimer->type, ptrTimer->time);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                    printf("\n===========================================\n");
                    printf("ETHERNET PORT %lu/%lu STATE: %s\n", i, port, POE_STATE_STR(poe_dot3at_port_state[i-1][port-1]));
                    printf("         PSE allocated: %lu, PD requested: %lu\n", dot3at_info.PSEAllocatedPowerValue, dot3at_info.PDRequestedPowerValueEcho);
                    printf("Mirrored PSE allocated: %lu, PD requested: %lu\n", dot3at_info.MirroredPSEAllocatedPowerValueEcho, dot3at_info.MirroredPDRequestedPowerValue);
#endif
                }
                else
                {
                    printf("port ERROR\n");
                    error++;
                }
            }
        }
    }
    else
    {
        if (port == 0)
        {
            for (j=1;j<SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT;j++)
            {
                if (POE_OM_UserPortExisting(unit,j)==TRUE)
                {
                    POE_ENGINE_GetPortVariable(unit,j,&dot3at_info);
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                    ptrTimer = &poe_dot3at_port_timer[unit-1][j-1];
                    printf("\n===========================================\n");
                    printf("ETHERNET PORT %lu/%lu STATE: %s\n", unit, j, POE_STATE_STR(poe_dot3at_port_state[unit-1][j-1]));
                    printf("LOCAL actual: %lu, requested: %lu, ack: %lu\n", dot3at_info.locActualPowerValue, 
                        dot3at_info.locRequestedPowerValue, dot3at_info.locRequestPowerAcknowledge);
                    printf("REMOTE actual: %lu, requested: %lu\n", dot3at_info.remActualPowerValue, dot3at_info.remRequestedPowerValue);
                    printf("timer type: %u, timer value: %u\n", ptrTimer->type, ptrTimer->time);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                    printf("\n===========================================\n");
                    printf("ETHERNET PORT %lu/%lu STATE: %s\n", unit, j, POE_STATE_STR(poe_dot3at_port_state[unit-1][j-1]));
                    printf("         PSE allocated: %lu, PD requested: %lu\n", dot3at_info.PSEAllocatedPowerValue, dot3at_info.PDRequestedPowerValueEcho);
                    printf("Mirrored PSE allocated: %lu, PD requested: %lu\n", dot3at_info.MirroredPSEAllocatedPowerValueEcho, dot3at_info.MirroredPDRequestedPowerValue);
#endif
                }
            }
        }
        else
        {
            if (POE_OM_UserPortExisting(unit,port)==TRUE)
            {
                POE_ENGINE_GetPortVariable(unit,port,&dot3at_info);
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
                ptrTimer = &poe_dot3at_port_timer[unit-1][port-1];
                printf("\n===========================================\n");
                printf("PORT ETHERNET %lu/%lu STATE: %s\n", unit, port, POE_STATE_STR(poe_dot3at_port_state[unit-1][port-1]));
                printf("LOCAL actual: %lu, requested: %lu, ack: %lu\n", dot3at_info.locActualPowerValue, 
                    dot3at_info.locRequestedPowerValue, dot3at_info.locRequestPowerAcknowledge);
                printf("REMOTE actual: %lu, requested: %lu\n", dot3at_info.remActualPowerValue, dot3at_info.remRequestedPowerValue);
                printf("timer type: %u, timer value: %u\n", ptrTimer->type, ptrTimer->time);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                printf("\n===========================================\n");
                printf("ETHERNET PORT %lu/%lu STATE: %s\n", unit, port, POE_STATE_STR(poe_dot3at_port_state[unit-1][port-1]));
                printf("         PSE allocated: %lu, PD requested: %lu\n", dot3at_info.PSEAllocatedPowerValue, dot3at_info.PDRequestedPowerValueEcho);
                printf("Mirrored PSE allocated: %lu, PD requested: %lu\n", dot3at_info.MirroredPSEAllocatedPowerValueEcho, dot3at_info.MirroredPDRequestedPowerValue);
#endif
            }
            else
            {
                printf("port ERROR\n");
                error++;
            }
        }
    }

	if (error)
		return FALSE;
	else
		return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_CheckRemoteRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will check if remote requested power value is vaild or not
 * INPUT    : unit, port
 *          : reqPowerValue : unit: 0.1W
 * OUTPUT   : newPowerValue : unit: 0.1W
 * RETURN   : Successful or fail reason
 * Note     : None
 *-------------------------------------------------------------------------
 */
static UI32_T POE_ENGINE_CheckRemoteRequest(UI32_T unit, UI32_T port, UI32_T reqPowerValue, UI32_T* newPowervlaue)
{
    UI32_T value;
    UI32_T portPower_limit;
    UI32_T consumption_power;
    UI32_T power_budget;
    UI32_T result = POE_ENGINE_REMOTE_REQUEST_SUCCESS;
    BOOL_T ret;
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;
    BOOL_T local_power, ret2;
#endif

    *newPowervlaue = reqPowerValue;

    /* 1. check the power limit of port */
    ret = POE_OM_GetPortPowerMaximumAllocation(unit, port, &portPower_limit) ;
    POE_DEBUG("request power %lumW, port power limit %lumW, \r\n", reqPowerValue*100, portPower_limit);
    if (ret == TRUE)
    {
        /*  unit: reqPowerValue: 0.1W, portlimit value: 0.001W
         */
        if ((reqPowerValue*100) > portPower_limit)
        {
            *newPowervlaue = portPower_limit/100;
            result = POE_ENGINE_REMOTE_REQUEST_PORT_POWERLIMIT_FAIL;
        }
    }
    else
    {
        *newPowervlaue = 0;
        result = POE_ENGINE_REMOTE_REQUEST_PORT_POWERLIMIT_FAIL;
    }

    /* 2. check the system budget */
    ret = POE_OM_GetMainpowerMaximumAllocation(unit, &value);
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    STKTPLG_OM_GetUnitBoardID(unit, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
    ret2 = POE_OM_GetUseLocalPower(unit, &local_power);
#endif

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    if ((ret == TRUE) && (ret2 == TRUE))
#else
    if (ret == TRUE)
#endif
    {
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
        //if(local_power && (value > SYS_HWCFG_MAX_POWER_ALLOCATION_LOCAL))
        if(local_power && (value > board_info.main_pse_power_max_allocation))
            value = board_info.main_pse_power_max_allocation;
#endif

        /* unit: reqPowerValue: 0.1W, system limit: 1W
         */
        if (POE_OM_GetMainPseConsumptionPower(unit, &consumption_power)==FALSE)
        {
            *newPowervlaue = 0;
            result = POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL;
        }
        else
        {
            power_budget = value - consumption_power; /* power budget */
            POE_DEBUG("power max-alloc %lumW, power consum %lumW, power budget %lumW\r\n", value, consumption_power, power_budget);
            if (reqPowerValue > (power_budget*10))
            {
                if (power_budget*10 < *newPowervlaue)
                    *newPowervlaue = power_budget*10;
                result = POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL;
            }
        }
    }
    else
    {
        *newPowervlaue = 0;
        result = POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_FAIL;
    }

    /* 2008/4/30, we don't need check in 3Com project.
     * bcm5910x remove this limitation due to 3Com requirement
     */
#if 0
    /* 3. check if the power budget is more than port power_limit  */
    if ((power_budget*1000) < portPower_limit)
    {
        *newPowervlaue = 0;
        return POE_ENGINE_REMOTE_REQUEST_SYSTEM_BUDGET_LESS_THAN_PORT_POWERLIMIT;
    }
#endif

	return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will change the dot3at state of the port
 * INPUT    : unit, port
 *          : state : the value defined in POE_ENGINE_DOT3AT_STATE_E
 * OUTPUT   : None
 * RETURN   : None
 * Note     : 
 *-------------------------------------------------------------------------
 */
static void POE_ENGINE_SetPortState(UI32_T unit, UI32_T port, UI32_T state, UI32_T reason)
{
    if (poe_dot3at_port_state[unit-1][port-1] != state)
    {
        POE_DEBUG("(%s) Ethernet port %lu/%lu changes from %s to %s state\n", poe_state_change_reason_desc[reason],
			unit, port, POE_STATE_STR(poe_dot3at_port_state[unit-1][port-1]), POE_STATE_STR(state));

        poe_dot3at_port_state[unit-1][port-1] = state;
    }
    else
    {
        POE_DEBUG("(%s) Ethernet port %lu/%lu stay at %s state\n", poe_state_change_reason_desc[reason], unit, port, POE_STATE_STR(state));
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_SetPortVariable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the related variables of dot3at SM
 * INPUT    : unit, port
 *          : pInfo : data of type POE_ENGINE_PORT_DOT3AT_VARIABLE_T
 * OUTPUT   : None
 * RETURN   : None
 * Note     : 
 *-------------------------------------------------------------------------
 */
static BOOL_T POE_ENGINE_SetPortVariable(UI32_T unit, UI32_T port, POE_ENGINE_PORT_DOT3AT_VARIABLE_T *pInfo)
{
    if (pInfo == NULL)
    {
        return FALSE;
    }

    POE_ENGINE_ENTER_CRITICAL_SECTION;

    memcpy(&poe_dot3at_port_info[unit-1][port-1], pInfo, sizeof(POE_ENGINE_PORT_DOT3AT_VARIABLE_T));

    POE_ENGINE_LEAVE_CRITICAL_SECTION;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_GetPortVariable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the related variables of dot3at SM
 * INPUT    : unit, port
 * OUTPUT   : pInfo : data of type POE_ENGINE_PORT_DOT3AT_VARIABLE_T
 * RETURN   : None
 * Note     : 
 *-------------------------------------------------------------------------
 */
static BOOL_T POE_ENGINE_GetPortVariable(UI32_T unit, UI32_T port, POE_ENGINE_PORT_DOT3AT_VARIABLE_T *pInfo)
{
    if (pInfo == NULL)
    {
        return FALSE;
    }

    POE_ENGINE_ENTER_CRITICAL_SECTION;

    memcpy(pInfo, &poe_dot3at_port_info[unit-1][port-1], sizeof(POE_ENGINE_PORT_DOT3AT_VARIABLE_T));

    POE_ENGINE_LEAVE_CRITICAL_SECTION;

    return TRUE;
}


#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
/* ---------- for manual high-power mode ----------*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ENABLE_PORT_DOT3AT
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to enable state machine
 * INPUT    : unit, port
 * OUTPUT   : None
 * RETURN   : None
 * Note     : only called by poe_task.c
 *-------------------------------------------------------------------------
 */
void POE_ENGINE_ENABLE_PORT_DOT3AT(UI32_T unit, UI32_T port)
{
    /* Change state only orignal is 'disable' */
    if (poe_dot3at_port_state[unit-1][port-1] == POE_STATE_MACHINE_3AT_DISABLE)
    {
        POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_IDLE, POE_ENGINE_MANUAL_HIGH_POWER);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_DISABLE_PORT_DOT3AT
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to disable state machine
 * INPUT    : unit, port
 * OUTPUT   : None
 * RETURN   : None
 * Note     : only called by poe_task.c
 *-------------------------------------------------------------------------
 */
void POE_ENGINE_DISABLE_PORT_DOT3AT(UI32_T unit, UI32_T port)
{
    POE_ENGINE_PORT_DOT3AT_VARIABLE_T dot3at_info;
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    POE_ENGINE_DOT3AT_L2_TIMER_T *ptrTimer;
#endif

    if (poe_dot3at_port_state[unit-1][port-1] != POE_STATE_MACHINE_3AT_DISABLE)
    {

        /* clear all variable and timer */
        memset(&dot3at_info, 0, sizeof(POE_ENGINE_PORT_DOT3AT_VARIABLE_T));
        POE_ENGINE_SetPortVariable(unit, port, &dot3at_info);
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
        ptrTimer = &poe_dot3at_port_timer[unit-1][port-1];
        ptrTimer->time = 0;
        ptrTimer->type = POE_ENGINE_DOT3AT_TIMER_TYPE_DISABLE;
#endif

        /* Disable the dot3at High-Power Mode */
        POE_MGR_SetPortDot3atHighPowerMode(unit, port, 0);

        /* Disable state machine */
        POE_ENGINE_SetPortState(unit, port, POE_STATE_MACHINE_3AT_DISABLE, POE_ENGINE_MANUAL_HIGH_POWER);
    }

}
#endif

