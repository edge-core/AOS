/*-----------------------------------------------------------------------------
 * FILE NAME: poe_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides the APIs for upper-layer to access the PoE controller.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    04/7/2003 - Kelly Hung, Created
 *    08/6/2007 - Daniel Chen, Add code to determine if the user port
 *                             is also a PoE Port.
 *    04/30/2008 - Daniel Chen, Add manual high-power mode, In this mode,
 *                              dot3at state machine of the port will be suspended temorarily
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "poe_om.h"
#include "poe_mgr.h"
#include "poedrv.h"
#include "sysfun.h"
#include "sys_time.h"
#include "eh_mgr.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "sys_adpt.h"
#include "syslog_type.h"
#include "trap_event.h"
#include "snmp_pmgr.h"
//#include "leaf_es3626a.h"
#include "leaf_3621.h"
#include "stktplg_pom.h"
#include "sysdrv.h"
#include "poe_type.h"
#include "sys_cpnt.h"
#include <stdio.h>
#include <stdlib.h>

#include "poe_task.h"
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#include "poe_engine.h"
#include "lldp_pmgr.h"
#endif
#include "poe_backdoor.h"

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
#include "time_range_type.h"
#include "time_range_pmgr.h"
#endif
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
#include "stktplg_board.h"
#endif

/*---------------------------------------------------------------------
 *          STATIC VARIABLE DEFINIATIONS
 *---------------------------------------------------------------------
 */

//static UI32_T                                   poe_mgr_semaphore_id;
static UI32_T                                   old_usage_status;
static UI32_T                                   current_usage_status;

#ifdef SYS_CPNT_POE_PSE_DOT3AT
// Eugene temp, static UI32_T                                   poe_mgr_taskid;
// Eugene temp, static UI32_T                                   poe_mgr_msgid;
static UI8_T                                    poe_statuschg_portmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
#endif

SYSFUN_DECLARE_CSC                    /* declare variables used for transition mode  */

/*--------------
    define
  --------------*/
#define POE_MGR_LOCK()                          //SYSFUN_TakeSem(poe_mgr_semaphore_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POE_MGR_UNLOCK()                        //SYSFUN_GiveSem(poe_mgr_semaphore_id)

#define POE_MGR_NO_USE_FUNC_NO                  0xffffffff

#define POE_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL)                            \
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE){   \
        EH_MGR_Handle_Exception (SYS_MODULE_POE,                         \
                                 POE_MGR_NO_USE_FUNC_NO,                    \
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,            \
                                 EH_MGR_FOR_DEBUG_MSG_PURPOSE|SYSLOG_LEVEL_ERR);  \
        return (RET_VAL);                                                   \
    }

#define POE_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) return (RET_VAL);

#define MANAGED_OBJECT_CLASS                              "ESN310"
#define MANAGED_OBJECT_INSTANCE                           "ESN310"
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/*-----------
    enum
  -----------*/
typedef enum
{
    POE_MGR_PSE_PORT_D              = 1,
    POE_MGR_MAIN_PSE_D              = 2,
    POE_MGR_NOTIFICATION_CONTROL_D  = 3
} POE_MGR_DataBase_T;

typedef enum
{
    POE_MGR_POWER_USAGE_ON          = 1,
    POE_MGR_POWER_USAGE_OFF         = 2
} POE_MGR_PowerUsage_T;

/*-------------
    function
  -------------*/
void POE_MGR_SetPsePortAdmin_callback(UI32_T group_index, UI32_T port_index, UI32_T value);
void POE_MGR_SetPsePortDetectionStatus_callback(UI32_T group_index, UI32_T port_index, UI32_T value);
void POE_MGR_SetPsePortPoweConsumption_callback(UI32_T group_index, UI32_T port_index, UI32_T value);
void POE_MGR_SetPsePortPowerClassification_callback(UI32_T group_index, UI32_T port_index, UI32_T value);
void POE_MGR_SetMainPsePower_callback(UI32_T group_index, UI32_T value);
void POE_MGR_SetMainPseOperStatus_callback(UI32_T group_index, UI32_T value);
void POE_MGR_SetMainPseConsumptionPower_callback(UI32_T group_index, UI32_T value);
void POE_MGR_SetLegacyDetection_callback(UI32_T group_index, UI8_T value);

static SYS_TYPE_CallBack_T *PortOverloadStatusChange_callbacklist;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_InitiateSystemResources
 * -------------------------------------------------------------------------
 * FUNCTION: init POE system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_InitiateSystemResources()
{
DBG_PRINT();
    /* Create semaphore
     */
/*    if(SYSFUN_CreateSem (SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &poe_mgr_semaphore_id) != SYSFUN_OK)
    {
        printf ("\n\r POE_MGR: Create semaphore failed. LOCK");
        while (1)
            ;
    }
*/
    POE_OM_InitSemaphore();

    /* Power usage threshold
     */
    old_usage_status     = POE_MGR_POWER_USAGE_OFF;
    current_usage_status = POE_MGR_POWER_USAGE_OFF;

    PortOverloadStatusChange_callbacklist = 0;

#ifdef SYS_CPNT_POE_PSE_DOT3AT
    memset(poe_statuschg_portmask, 0, sizeof(poe_statuschg_portmask));
// Eugene temp,     poe_mgr_taskid = 0;
// Eugene temp,     poe_mgr_msgid = 0;
#endif

    /* Register callback function
     */
#if 0 /* Eugene temp */
    POEDRV_Register_PortDetectionStatusChange_CallBack(POE_MGR_SetPsePortDetectionStatus_callback);
    POEDRV_Register_MainPseConsumptionChange_CallBack(POE_MGR_SetMainPseConsumptionPower_callback);
    POEDRV_Register_PseOperStatusChange_CallBack(POE_MGR_SetMainPseOperStatus_callback);
    POEDRV_Register_PortOverloadStatusChange_CallBack(POE_MGR_Notify_PortOverloadStatusChange);
    POEDRV_Register_PortPowerConsumptionChange_CallBack(POE_MGR_SetPsePortPoweConsumption_callback);
    POEDRV_Register_PortPowerClassificationChange_CallBack(POE_MGR_SetPsePortPowerClassification_callback);
    POEDRV_Register_PowerDeniedOccurFrequently_CallBack(POE_MGR_Notify_PowerDeniedOccurFrequently);
    #if 0
    POEDRV_Register_Legacy_Detection_CallBack(POE_MGR_SetLegacyDetection_callback);
    #endif

#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if 0 /* Eugene temp */
    LLDP_MGR_RegisterDot3atReceive(POE_MGR_NotifyLldpFameReceive_Callback);
#endif
    /* Register Backdoor */
#if 0 /* Eugene mark, do it in POE_PROC_Create_InterCSC_Relation */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("POE", POE_BACKDOOR_Main);
#endif
#endif
#endif

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: enter master state
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_EnterMasterMode(void)
{
DBG_PRINT();
    SYSFUN_ENTER_MASTER_MODE();
    POE_OM_GetDBDefaultValue();
    POE_OM_SetDBToDefault();
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE POE_MGR_HandleHotInsertion
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * -------------------------------------------------------------------------*/
void POE_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
        UI32_T number_of_port,
        BOOL_T use_default)
{
    UI32_T unit=0, port=0, trunk_id;

    if(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_POM_LogicalPortToUserPort(starting_port_ifindex, &unit, &port, &trunk_id))
    {
        POE_OM_SetDBToDefaultForHotInsert(unit);
    }
    return;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: enter slave state
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_EnterSlaveMode(void)
{
DBG_PRINT();
    SYSFUN_ENTER_SLAVE_MODE();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: set transition state flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_SetTransitionMode(void)
{
DBG_PRINT();
    SYSFUN_SET_TRANSITION_MODE();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: enter transition state
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_EnterTransitionMode(void)
{
DBG_PRINT();
    SYSFUN_ENTER_TRANSITION_MODE();
    POE_OM_ClearDB();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T value)
{
DBG_PRINT();

    /* BODY
    */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if (STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
         POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
        {
            UI32_T old_value;
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
            UI8_T  time_range[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH+1];
#endif

            POE_MGR_LOCK();

            /* Daniel Chen, 2007/10/26
             * Check current status, If the setting is the same, than we
             * return directly. For saving CPU cycles
             */
            POE_OM_GetPsePortAdmin(group_index, port_index, &old_value);
            if (value == old_value)
            {
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)

            POE_OM_GetPsePortTimeRangeName(group_index, port_index, time_range);

            /* PSE function priority: TIME_RANGE CSC > PoE CSC.
             *   - value shall be VAL_pethPsePortAdminEnable_false, VAL_pethPsePortAdminEnable_enable or VAL_pethPsePortAdminEnable_test mode for PSE port function.
             */
            if ((strncmp((char *) time_range, SYS_DFLT_PSE_PORT_TIME_RANGE_NAME, sizeof(time_range)) == 0) || ((TIME_RANGE_PMGR_IsTimeRangeEntryActive(time_range) == TRUE) && (value != VAL_pethPsePortAdminEnable_false)))
            {
#endif
            //set to drv
            if (POEDRV_SetPortAdminStatus(group_index, port_index, value))
            {
                //set to OM database
                POE_OM_SetPsePortAdmin(group_index, port_index, value);
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }
                else
                {
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
            }
            POE_OM_SetPsePortAdmin(group_index, port_index, value);
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);

#endif
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortAdmin_callback
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortAdmin_callback(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* BODY
     */
//    POE_MGR_USE_CSC_CHECK_OPER_MODE(NULL);

    if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
    {
        //set to OM database
        POE_OM_SetPsePortAdmin(group_index, port_index, value);
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
    }
//    else
//    {
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
//    }

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortDetectionStatus_callback
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortDetectionStatus_callback(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    TRAP_EVENT_TrapData_T   trapdata;
    UI32_T  notification_ctrl_value;

    /* BODY
     */
//    POE_MGR_USE_CSC_CHECK_OPER_MODE(NULL);

    if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
    {
        //set to OM database
        POE_OM_SetPsePortDetectionStatus(group_index, port_index, value);

        {
            POE_OM_GetPseNotificationCtrl(group_index, &notification_ctrl_value);
            if (notification_ctrl_value == VAL_pethNotificationControlEnable_true &&
                value != VAL_pethPsePortDetectionStatus_searching) /* except searching mode(RFC3621) */
            {
                trapdata.community_specified=FALSE;
                trapdata.trap_type=TRAP_EVENT_PETH_PSE_PORT_ON_OFF_TRAP;
                trapdata.u.peth_pse_port_on_off_trap.instance_pethPsePortDetectionStatus[0] = group_index;
                trapdata.u.peth_pse_port_on_off_trap.instance_pethPsePortDetectionStatus[1] = port_index;
                trapdata.u.peth_pse_port_on_off_trap.pethPsePortDetectionStatus = value;

                /* Ericsson would like to stop sending pethPsePortOnOffNotification trap since
                 * HP OpenView cannot interpret this trap.
                 */
#if (SYS_CPNT_TRAPMGMT == TRUE)
                TRAP_MGR_ReqSendTrapOptional(&trapdata, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
                SNMP_PMGR_ReqSendTrapOptional(&trapdata, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
            }
        }

#ifdef SYS_CPNT_POE_PSE_DOT3AT
        /* set changed port mask, and notify poe task to handle it
         */
        POE_MGR_LOCK();
//        poe_statuschg_portmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1-((port_index-1)/8)] |= 0x1 << (7-((port_index-1)%8));

//        POE_MGR_GetPortDetectionStatusPortMask(pmask);
//        for (i=0;i<SYS_ADPT_TOTAL_NBR_OF_LPORT;i++)
        {
//            if (poe_statuschg_portmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1-(i/8)] & (0x1<<(7-(i%8))))
            {
                POE_ENGINE_ProcessPortStateChange(group_index,port_index);
            }
        }

        POE_MGR_UNLOCK();

//        SYSFUN_SendEvent(poe_mgr_taskid, POE_TYPE_EVENT_PORT_POE_STATE_CHANGE);
#endif

//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
    }
//    else
//    {
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
//    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPoweConsumption_callback
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value power consumption
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortPoweConsumption_callback(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* BODY
     */

    if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
    {
        //set to OM database
        POE_OM_SetPsePortPowerConsumption(group_index, port_index, value);
     }

}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerClassification_callback
 * -------------------------------------------------------------------------
 * FUNCTION: update power classification for this port.
 * INPUT   : group_index
             port_index
             value power class
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortPowerClassification_callback(UI32_T group_index, UI32_T port_index, UI32_T value)
{
#if 0 /* Eugene temp */
    TRAP_EVENT_TrapData_T   trapdata;
#endif
    //UI32_T  notification_ctrl_value;

    /* BODY
     */

    if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
    {
        //set to OM database
        POE_OM_SetPsePortPowerClassification(group_index, port_index, value);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
             port_index
             value = signal(1)means that the signal pairs only are in use.
                   = spare(2) means that the spare pairs only are in use.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : If the value of pethPsePortPowerPairsControl is true,
             this object is writable.
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* BODY
     */
    UI32_T powerpairsctrlability;

    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
        {
            POE_MGR_LOCK();
            POE_OM_GetPsePortPowerPairsCtrlAbility(group_index, port_index, &powerpairsctrlability);
            if(powerpairsctrlability==VAL_pethPsePortPowerPairsControlAbility_true)
            {
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
                if (POEDRV_SetPortPowerPairs(group_index, port_index, value) == FALSE)
                {
                    POE_MGR_UNLOCK();
                    POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
                }
#endif
                POE_OM_SetPsePortPowerPairs(group_index, port_index, value);//set to OM database
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }
            else
            {
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
        }
        else
        {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

#if 0
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : group_index
             port_index
             value = auto(1)enables the power detection mechanism of the port.
                   = test(2)puts the port in a testmode:
                     force continuous discovery without applying
                     power regardless of whether PD detected.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortPowerDetectionCtrl(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    BOOL_T        ret;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
#if 0 /* Eugene marked for not using universal image */
    if (STKTPLG_MGR_IsPoeDevice(group_index)==FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
#endif
    {
        if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
        {
            POE_MGR_LOCK();
            //set to drv
            ret = POEDRV_SetPortPowerDetectionControl(group_index, port_index, value);
            //set to OM database
            if (ret)
                POE_OM_SetPsePortPowerDetectionCtrl(group_index, port_index, value);
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        return ret;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_GetPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_GetPsePortPowerDetectionCtrl(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
#if 0 /* Eugene marked for not using universal image */
    if (STKTPLG_MGR_IsPoeDevice(group_index)==FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
#endif
    {

        if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
        {
            POE_MGR_LOCK();
            POE_OM_GetPsePortPowerDetectionCtrl(group_index, port_index, value); //get data
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_GetNextPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_GetNextPsePortPowerDetectionCtrl(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    if (! POE_MGR_GetPsePortPowerDetectionCtrl(*group_index, *port_index, value))
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_GetRunningPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_MGR_GetRunningPsePortPowerDetectionCtrl(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (! POE_MGR_GetPsePortPowerDetectionCtrl(group_index, port_index, value))
        POE_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (*value == PSE_PORT_POWER_DETECTION_CONTROL_DEF)
    {
    POE_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
    }
    else
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_GetNextRunningPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_MGR_GetNextRunningPsePortPowerDetectionCtrl(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    SYS_TYPE_Get_Running_Cfg_T ret;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        POE_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = POE_MGR_GetRunningPsePortPowerDetectionCtrl(*group_index, *port_index, value);

    POE_MGR_RETURN_AND_RELEASE_CSC(ret); //return check result
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
             of view of a power management algorithm.
 * INPUT   : group_index
             port_index
             value = critical(1)
                   = high(2)
                   = low(3)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : The priority that
             is set by this variable could be used by a control mechanism
             that prevents over current situations by disconnecting first
             ports with lower power priority. Ports that connect devices
             critical to the operation of the network - like the E911
             telephones ports - should be set to higher priority.
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T value)
{
#if 0 /* Eugene temp */
    BOOL_T        ret;
#endif

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (POE_OM_UserPortExisting(group_index, port_index)) //exist ?
        {
            /* Daniel Chen, 2007/10/26
             * Check current status, If the setting is the same, than we
             * return directly. For saving CPU cycles
             */
            UI32_T old_value;

            POE_MGR_LOCK();
            POE_OM_GetPsePortPowerPriority(group_index, port_index, &old_value);
            if (value == old_value)
            {
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }

            //set to drv
            if (POEDRV_SetPortPowerPriority(group_index, port_index, value))
            {
            //set to OM database
                POE_OM_SetPsePortPowerPriority(group_index, port_index, value);
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);

#ifdef SYS_CPNT_POE_PSE_DOT3AT
{
                UI32_T        lport;

                SWCTRL_POM_UserPortToLogicalPort(group_index, port_index, &lport);
                LLDP_PMGR_NotifyPseTableChanged(lport);
}
#endif
            }
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else
        {
           POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
             that indicates the type of the device that is connected
             to theport. This value can be the result of the mapping
             the address of the station connected to the port and of
             the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
             port_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortType(UI32_T group_index, UI32_T port_index, UI8_T *value, UI32_T len)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (
            (POE_OM_UserPortExisting(group_index, port_index)) &&
            (len>=(MINSIZE_pethPsePortType+1)) &&
            (len<=(MAXSIZE_pethPsePortType+1))
           ) //port_exist && len is in the range?
        {
            POE_MGR_LOCK();
            //set to OM database
            POE_OM_SetPsePortType(group_index, port_index, value, len);
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Set a specified port the maximum power
 * INPUT   : group_index
             port_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    UI32_T        old_portMaxAllocation;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (SWCTRL_POM_UserPortExisting(group_index, port_index)) //exist ?
        {
            POE_MGR_LOCK();
            /* Daniel Chen, 2007/10/26
             * Check current status, If the setting is the same, than we
             * return directly. For saving CPU cycles
             */
            POE_OM_GetPortPowerMaximumAllocation(group_index, port_index, &old_portMaxAllocation);
            if (value == old_portMaxAllocation)
            {
                POE_MGR_UNLOCK();
                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }

            if (POEDRV_SetPortPowerMaximumAllocation(group_index, port_index, value))
            {
                //set to OM database
                POE_OM_SetPortPowerMaximumAllocation(group_index, port_index, value);
                POE_MGR_UNLOCK();

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                POE_ENGINE_LocalSystemChange(group_index, port_index);
#endif

                POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);

            }
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LLDP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T POE_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    POE_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_ui32 = POE_TYPE_RETURN_ERROR;
        msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding POE_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case POE_MGR_IPC_SETMAINPOWERMAXIMUMALLOCATION:
            msg_p->type.ret_ui32 =
            POE_MGR_SetMainpowerMaximumAllocation(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETPSEPORTADMIN:
            msg_p->type.ret_ui32 =
            POE_MGR_SetPsePortAdmin(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETPORTPOWERMAXIMUMALLOCATION:
            msg_p->type.ret_ui32 =
            POE_MGR_SetPortPowerMaximumAllocation(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETPSEPORTPOWERPRIORITY:
            msg_p->type.ret_ui32 =
            POE_MGR_SetPsePortPowerPriority(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETLEGACYDECTECTION:
            msg_p->type.ret_ui32 =
            POE_MGR_SetLegacyDetection(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
        case POE_MGR_IPC_SETPORTMANUALHIGHPOWERMODE:
            msg_p->type.ret_ui32 =
            POE_MGR_SetPortManualHighPowerMode(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case POE_MGR_IPC_SETPSEPORTPOWERPAIRS:
            msg_p->type.ret_ui32 =
            POE_MGR_SetPsePortPowerPairs(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETPSEPORTPOWERTYPE:
            msg_p->type.ret_ui32 =
            POE_MGR_SetPsePortType(msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui8,msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui32_3);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETMAINPSEUSAGETHRESHOLD:
            msg_p->type.ret_ui32 =
            POE_MGR_SetMainPseUsageThreshold(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_SETNOTIFICATIONCTRL:
            msg_p->type.ret_ui32 =
            POE_MGR_SetNotificationCtrl(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
        case POE_MGR_IPC_BINDTIMERANGETOPSEPORT:
            msg_p->type.ret_ui32 =
            POE_MGR_BindTimeRangeToPsePort(msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2,msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;

        case POE_MGR_IPC_UNBINDTIMERANGETOPSEPORT:
            msg_p->type.ret_ui32 =
            POE_MGR_UnbindTimeRangeToPsePort(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = POE_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = POE_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of POE_MGR_HandleIPCReqMsg */





/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPsePower_callback
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE power
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_SetMainPsePower_callback(UI32_T group_index, UI32_T value)
{
    /* BODY
     */
//    POE_MGR_USE_CSC_CHECK_OPER_MODE(NULL);

    if (POE_OM_UserPortExisting(group_index, 1)) //exist ?
    {
        //set to OM database
        POE_OM_SetMainPsePower(group_index, value);
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
    }
//    else
//    {
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
//    }

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPseConsumptionPower_callback
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE power consumption by callback
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_SetMainPseConsumptionPower_callback(UI32_T group_index, UI32_T value)
{
    UI32_T                  mainpower_limit=0;
    UI32_T                  mainpower_threshold=0;
    TRAP_EVENT_TrapData_T   trapdata;
    UI32_T                  notification_ctrl_value;

    /* BODY
     */
//    POE_MGR_USE_CSC_CHECK_OPER_MODE(NULL);
    if (POE_OM_UserPortExisting(group_index, 1)) //exist ?
    {
        //set to OM database
        POE_OM_SetMainPseConsumptionPower(group_index, value);

        if ( !POE_OM_GetMainPsePower(group_index, &mainpower_limit) )
              return;

        if ( !POE_OM_GetMainPseUsageThreshold(group_index, &mainpower_threshold) )
              return;

        /* Check to see if power usage in percents exceeds usage threshold
         */
        if ( (value*100/mainpower_limit) > mainpower_threshold )
             current_usage_status = POE_MGR_POWER_USAGE_ON;
        else
             current_usage_status = POE_MGR_POWER_USAGE_OFF;
     #if 0
        printf("\n\r-power: %ld used, %ld available, threshold=%ld, old_usage_status=%ld, current_usage_status=%ld",
               value, mainpower_limit, mainpower_threshold, old_usage_status, current_usage_status);
     #endif
        /* Check to see if usage status has been changed
         */
        if ( old_usage_status != current_usage_status )
        {
             old_usage_status = current_usage_status;
             POE_OM_GetPseNotificationCtrl(group_index, &notification_ctrl_value);

             if (notification_ctrl_value == VAL_pethNotificationControlEnable_true)
             {
                 switch (current_usage_status)
                 {
                    case POE_MGR_POWER_USAGE_ON:  /* Turn power usage on */
                      #if 0
                         printf("\n\rSending trap: POWER_USAGE_ON");
                      #endif
                        trapdata.community_specified=FALSE;  /* send trap to all community */
                        trapdata.trap_type=TRAP_EVENT_PETH_MAIN_POWER_USAGE_ON_TRAP;
                        trapdata.u.peth_main_power_usage_on_trap.instance_pethMainPseConsumptionPower = group_index;
                        trapdata.u.peth_main_power_usage_on_trap.pethMainPseConsumptionPower = value;
#if (SYS_CPNT_TRAPMGMT == TRUE)
                        TRAP_MGR_ReqSendTrapOptional(&trapdata, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
                        SNMP_PMGR_ReqSendTrapOptional(&trapdata, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
                         break;

                    case POE_MGR_POWER_USAGE_OFF: /* Turn power usage off */
                      #if 0
                         printf("\n\rSending trap: POWER_USAGE_OFF");
                      #endif

                        trapdata.community_specified=FALSE;  /* send trap to all community */
                        trapdata.trap_type=TRAP_EVENT_PETH_MAIN_POWER_USAGE_OFF_TRAP;
                        trapdata.u.peth_main_power_usage_off_trap.instance_pethMainPseConsumptionPower = group_index;
                        trapdata.u.peth_main_power_usage_off_trap.pethMainPseConsumptionPower = value;

#if (SYS_CPNT_TRAPMGMT == TRUE)
                        TRAP_MGR_ReqSendTrapOptional(&trapdata, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
                        SNMP_PMGR_ReqSendTrapOptional(&trapdata, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif

                         break;
                 } /* End of switch (current_usage_status) */
             } /* End of if (notification_ctrl_value..) */

        } /* End of if ( old_usage_status != current_usage_status ) */
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
    }
//    else
//    {
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
//    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPseOperStatus_callback
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_SetMainPseOperStatus_callback(UI32_T group_index, UI32_T value)
{
    /* BODY
     */
//    POE_MGR_USE_CSC_CHECK_OPER_MODE(NULL);

    if (POE_OM_UserPortExisting(group_index, 1)) //exist ?
    {
        //set to OM database
        POE_OM_SetMainPseOperStatus(group_index, value);
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
    }
//    else
//    {
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
//    }

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: set the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.
 * INPUT   : group_index
             value = (1..99)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetMainPseUsageThreshold(UI32_T group_index, UI32_T value)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (POE_OM_UserPortExisting(group_index, 1)) //exist ?
        {
            POE_MGR_LOCK();
            //set to OM database
            POE_OM_SetMainPseUsageThreshold(group_index, value);
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index : unit
 *           value       : watt
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetMainpowerMaximumAllocation(UI32_T group_index, UI32_T value)
{
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T power_supported, board_id;
    BOOL_T local_power = TRUE;
#endif
    UI32_T new_power = value;
    BOOL_T ret;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
        POE_OM_GetUseLocalPower(group_index, &local_power);
        STKTPLG_OM_GetUnitBoardID(group_index, &board_id);
        STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);

        //power_supported = (local_power? SYS_HWCFG_MAX_POWER_ALLOCATION_LOCAL : SYS_HWCFG_MAX_POWER_ALLOCATION_RPS);
        power_supported = (local_power? board_info.main_pse_power_max_allocation: board_info.main_pse_power_max_allocation_rps);
        if(new_power > power_supported)
            new_power = power_supported;
#endif
        POE_MGR_LOCK();
        ret = POEDRV_SetMainpowerMaximumAllocation(group_index, new_power);

        if (ret)
        {
            POE_OM_SetMainpowerMaximumAllocation(group_index, new_power);
            POE_MGR_UNLOCK();

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
            POE_ENGINE_LocalSystemChange(group_index, 0);
#endif

            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_UNLOCK();

            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enable Notification from Agent.
 * INPUT   : group_index
             value = enable(1)
                   = disable(2)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetNotificationCtrl(UI32_T group_index, UI32_T value)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (POE_OM_UserPortExisting(group_index, 1)) //exist ?
        {
            POE_MGR_LOCK();
            //set to OM database
            POE_OM_SetNotificationCtrl(group_index, value);
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Notify_PortOverloadStatusChange
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_Notify_PortOverloadStatusChange(UI32_T group_index, UI32_T port_index, BOOL_T is_overload)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    /* BODY
     */
   if (is_overload == TRUE) //Disable a pse port when overoad occured to prevent infinite power cycle
   {
       POE_MGR_SetPsePortAdmin(group_index, port_index, VAL_pethPsePortAdminEnable_false);
   }
   else
   {
   }
     /* needn't callback to other level again ,    Torin
    for(fun_list=PortOverloadStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
        fun_list->func(group_index, port_index, is_overload);
    */
} /* End of POE_MGR_Notify_PortOverloadStatusChange() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Notify_PowerDeniedOccurFrequently
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_Notify_PowerDeniedOccurFrequently(UI32_T group_index, UI32_T port_index)
{
    /* BODY
     */
    POE_MGR_SetPsePortAdmin(group_index, port_index, VAL_pethPsePortAdminEnable_false);

} /* End of POE_MGR_Notify_PowerDeniedOccurFrequently() */



/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Register_PortOverloadStatusChange_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used for LED MGR to register its callback
 *           function for overload status.
 * INPUT:    fun -- the pointer of callback function.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : void *fun(UI32_T unit, UI32_T port, BOOL_T is_overload)
 *           unit -- unit ID
 *           port -- port ID
 *           is_overload -- TRUE: overload, FALSE: normal condition
 * -------------------------------------------------------------------------*/
void POE_MGR_Register_PortOverloadStatusChange_CallBack(void (*fun)(UI32_T unit,
                                                                    UI32_T port,
                                                                    BOOL_T is_overload))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortOverloadStatusChange_callbacklist);

} /* End of POE_MGR_Register_PortOverloadStatusChange_CallBack() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SoftwareDownload
 * -------------------------------------------------------------------------
 * FUNCTION: Software download to PoE controller
 * INPUT   : unit -- unit ID
 *         : filename -- filename to be downloaded to PoE controller
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SoftwareDownload(UI32_T unit, UI8_T *filename)
{
    BOOL_T        ret;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(unit) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        /* Semantic check
         */
        if((unit < 1) || (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK))
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        /* Software download to PoE controller
         */
        POE_MGR_LOCK();
        ret = POEDRV_SoftwareDownload(unit, filename);
        POE_MGR_UNLOCK();

        POE_MGR_RETURN_AND_RELEASE_CSC(ret);
    }
} /* End of POE_MGR_SoftwareDownload() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection
 * INPUT   : unit
 *           value (1 for Enable, 0 for disable)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetLegacyDetection(UI32_T unit, UI32_T value)
{
    BOOL_T        ret=FALSE;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(unit) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        POE_MGR_LOCK();
        //set to drv
        ret = POEDRV_SetCapacitorDetectionControl(unit, (UI8_T)value);
        //set to OM database
        if (ret)
            POE_OM_SetLegacyDetection(unit, value);
        POE_MGR_UNLOCK();

        POE_MGR_RETURN_AND_RELEASE_CSC(ret);
    }
}

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_PowerStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: when power status changes,
 * INPUT   : unit
 *           power  -- VAL_swIndivPowerIndex_externalPower
 *                     VAL_swIndivPowerIndex_internalPower
 *           status -- VAL_swIndivPowerStatus_green
 *                     VAL_swIndivPowerStatus_red
 *                     VAL_swIndivPowerStatus_notPresent
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
void POE_MGR_PowerStatusChanged_CallBack(UI32_T unit, UI32_T power, UI32_T status)
{
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T value, power_supported, board_id;
    BOOL_T local_power = TRUE, ret;

    if(STKTPLG_OM_IsPoeDevice(unit) == FALSE)
    {
        return;
    }

    POE_OM_GetUseLocalPower(unit, &local_power);

    if(local_power)
    {
        /* if local power fail */
        if(power == VAL_swIndivPowerIndex_internalPower && status != VAL_swIndivPowerStatus_green)
        {
            local_power = FALSE;
        }
        else
        {
            return;
        }
    }
    else
    {
        /* if local power recover */
        if(power == VAL_swIndivPowerIndex_internalPower && status == VAL_swIndivPowerStatus_green)
        {
            local_power = TRUE;
        }
        else
        {
            return;
        }
    }

    /* 1. set power status to database
     */
    POE_OM_SetUseLocalPower(unit, local_power);
    STKTPLG_OM_GetUnitBoardID(unit, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);

    /* 2. check if power maximum allcation need to be changed
     */
    POE_OM_GetMainpowerMaximumAllocation(unit, &value);

    //power_supported = (local_power? SYS_HWCFG_MAX_POWER_ALLOCATION_LOCAL : SYS_HWCFG_MAX_POWER_ALLOCATION_RPS);
    power_supported = (local_power? board_info.main_pse_power_max_allocation: board_info.main_pse_power_max_allocation_rps);
    if(value > power_supported)
        value = power_supported;
#if (SYS_CPNT_POE_MAX_ALLOC_FIXED == TRUE)
    else if(value < power_supported)
        value = power_supported;
#endif

    /* 2-1. change power maximum allcation
     */
    POE_MGR_LOCK();
    ret = POEDRV_SetMainpowerMaximumAllocation(unit, value);
    POE_MGR_UNLOCK();

    if(ret == TRUE)
        POE_OM_SetMainpowerMaximumAllocation(unit, value);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    /* 2-2. notify poe engine
     */
    POE_ENGINE_LocalSystemChange(unit, 0);
#endif

#ifdef SYS_CPNT_POE_PSE_DOT3AT
    /* 3. notify lldp if needed
     */
    LLDP_PMGR_NotifyPseTableChanged(0); /* all port */
#endif

}
#endif

#if 0
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetLegacyDetection_callback
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection Callback
 * INPUT   : unit
 *           value (1 for Enable, 0 for disable)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_SetLegacyDetection_callback(UI32_T group_index, UI8_T value)
{
    /* BODY
     */
//    POE_MGR_USE_CSC_CHECK_OPER_MODE(NULL);

    if (POE_OM_UserPortExisting(group_index, 1)) //exist ?
    {
        //set to OM database
        POE_OM_SetLegacyDetection(group_index, (UI32_T)value);
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
    }
//    else
//    {
//        POE_MGR_RETURN_AND_RELEASE_CSC(NULL);
//    }
}
#endif

/* Ed_huang 2006.7.19, modeifiy for 3COM style poe implementation */
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
/* Ed_huang 2006.9.25, create API to check max allocation and priority at same time */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPortPowerMaximumAllocationByPri
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Set a specified port the maximum power
             according to the priority.
 * INPUT   : group_index
             port_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPortPowerMaximumAllocationByPri(UI32_T group_index, UI32_T port_index, UI32_T priority, UI32_T power)
{
    BOOL_T        ret;
    UI32_T        MaxAllocation;
    UI32_T        GuranteedAllocation;
    UI32_T        old_portMaxAllocation;

    /* BODY
     */
    GuranteedAllocation = 0;
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    if(STKTPLG_OM_IsPoeDevice(group_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
        if (SWCTRL_POM_UserPortExisting(group_index, port_index)) //exist ?
        {
            POE_MGR_LOCK();
            //set to drv
            if (priority == VAL_pethPsePortPowerPriority_critical)
            {
                POE_OM_GetMainpowerMaximumAllocation(group_index, &MaxAllocation);
                POE_OM_GetExistedHighPriorityPowerMaxAllocation(group_index, port_index, &GuranteedAllocation);
                POE_OM_GetPortPowerMaximumAllocation(group_index, port_index, &old_portMaxAllocation);
                if( (GuranteedAllocation + power) > (MaxAllocation)  )
                {
                    POE_MGR_UNLOCK();
                    POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
                }
            }
            //set to drv
            ret = POEDRV_SetPortPowerPriority(group_index, port_index, priority);
            //set to OM database
            if (ret)
                POE_OM_SetPsePortPowerPriority(group_index, port_index, priority);

            //set to drv
            ret = POEDRV_SetPortPowerMaximumAllocation(group_index, port_index, power);
            //set to OM database
            if (ret)
                POE_OM_SetPortPowerMaximumAllocation(group_index, port_index, power);
            POE_MGR_UNLOCK();
            POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        return ret;
    }
}
#endif


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the bridge operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_MGR_GetOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of POE_MGR_GetOperationMode*/

#ifdef SYS_CPNT_POE_PSE_DOT3AT
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_ResetPort
 *-------------------------------------------------------------------------
 * PURPOSE  : reset the a poe port in ASIC
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_ResetPort(UI32_T unit,UI32_T port)
{
    BOOL_T ret;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
    POE_MGR_LOCK();
    ret = POEDRV_ResetPort(unit, port);
    POE_MGR_UNLOCK();
#elif (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
    ret = TRUE;
#endif

    POE_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of POE_MGR_ResetPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPortDot3atHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (for DLL)
 * INPUT    : lport
 *            mode : 1 - high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_SetPortDot3atHighPowerMode(UI32_T unit, UI32_T port, UI32_T mode)
{
    BOOL_T ret;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
    POE_MGR_LOCK();
    ret = POEDRV_SetPortDot3atHighPowerMode(unit, port, mode);
    POE_MGR_UNLOCK();
#elif (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
    ret = TRUE;
#endif

    POE_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of POE_MGR_SetHighPowerMode */

#if 0 /* Eugene call poedrv directly */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPortForceHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (for DLL)
 * INPUT    : lport
 *            mode : 1 - high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_SetPortForceHighPowerMode(UI32_T lport, UI32_T mode)
{
    BOOL_T ret ;

    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    POE_MGR_LOCK();
    ret = POEDRV_SetPortForceHighPowerMode(lport, mode);
    POE_MGR_UNLOCK();
    POE_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of POE_MGR_SetHighPowerMode */
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_GetPortDetectionStatusPortMask
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the port status change mask
 * INPUT    : None
 * OUTPUT   : portmask: bitmask to indict the status-changed port
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_GetPortDetectionStatusPortMask(UI8_T portmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    POE_MGR_LOCK();

    /* Get the changed portmask */
    memcpy(portmask, poe_statuschg_portmask, sizeof(poe_statuschg_portmask));

    /* clear proecessed data */
    memset(poe_statuschg_portmask, 0, sizeof(poe_statuschg_portmask));

    POE_MGR_UNLOCK();

    POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

#if 0 /* Eugene temp */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPoeMgtId
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the PoE task ID, message queue id
 * INPUT    : taskId : the ID of PoE Task
 *            msgid: the ID of message queue for PoE
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void POE_MGR_SetPoeMgtId(UI32_T taskId, UI32_T msgid)
{
    poe_mgr_taskid = taskId;
     poe_mgr_msgid = msgid;
}
#endif

#if 1 /* Eugene temp */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_NotifyLldpFameReceived_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the PoE task ID
 * INPUT    : lport
 *            info: TLV data for dot3at
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_NotifyLldpFameReceived_Callback(UI32_T unit, UI32_T port, POE_TYPE_Dot3atPowerInfo_T *info)
{
    /* BODY
     */
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    /* refresh the state machine timer */
    POE_ENGINE_RefreshTimer(unit,port);
#endif

    /* handled state control TLV in state machine */
    if (POE_ENGINE_ProcessLLDPInfo(unit,port,info)==TRUE)
        POE_MGR_RETURN_AND_RELEASE_CSC(TRUE)
    else
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE)
}
#endif

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (for UI manual setting)
 * INPUT    : unit
 *            port
 *            mode : 1 - force high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_SetPortManualHighPowerMode(UI32_T unit, UI32_T port, UI32_T mode)
{
    UI32_T current_mode;

    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
    if (mode !=0 && mode !=1)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (POE_OM_UserPortExisting(unit, port) != TRUE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    POE_OM_GetPortManualHighPowerMode(unit, port, &current_mode);

    /* don't need to configure if the same setting */
    if (current_mode == mode)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
    }

    POE_MGR_LOCK();
    /* set to OM */
    if (mode == 1)
    {
        /* Diable LLDP tx/rx per port */
//        LLDP_PMGR_SetPortConfigAdminStatus(msg.lport, LLDP_TYPE_ADMIN_STATUS_DISABLE);

        /* Disabe state machine */
        POE_ENGINE_DISABLE_PORT_DOT3AT(unit, port);

        /* Enable Force mode */
        POEDRV_SetPortForceHighPowerMode(unit, port, 1);

        /* Reset this port */
        POEDRV_ResetPort(unit, port);
    }
    else
    {
        /* Disable Force mode */
        POEDRV_SetPortForceHighPowerMode(unit, port, 0);

        /* Reset this port */
        POEDRV_ResetPort(unit, port);

        /* Enable LLDP tx/rx per port */
//        LLDP_PMGR_SetPortConfigAdminStatus(msg.lport, LLDP_TYPE_ADMIN_STATUS_TX_RX);

        /* Enable state machine */
        POE_ENGINE_ENABLE_PORT_DOT3AT(unit, port);
    }

    POE_OM_SetPortManualHighPowerMode(unit, port, mode);

    POE_MGR_UNLOCK();
#elif (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
    current_mode = mode;
#endif

    POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}/* End of POE_MGR_SetPortManualHighPowerMode */
#endif

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : Process timer event
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void POE_MGR_ProcessTimerEvent(void)
{
    POE_ENGINE_ProcessTimerEvent();
}
#endif
#endif

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_BindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Bind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index, time_range
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_BindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index, UI8_T* time_range)
{
    UI32_T status = POE_TYPE_TIMERANGE_STATUS_INACTIVE, driver_admin = VAL_pethPsePortAdminEnable_false;
    UI8_T  ori_time_range[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH + 1] = {0};
    UI32_T ori_time_range_index, new_time_range_index;

    DBG_PRINT("binding %lu/%lu to %s", group_index, port_index, (char*) time_range);
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /*You must bind the new time-range first to prevent the time-range be deleted*/
    if (TIME_RANGE_PMGR_RequestTimeRangeEntry(time_range, TRUE, &new_time_range_index) == FALSE)
    {
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (POE_OM_GetPsePortTimeRangeIndex(group_index, port_index, &ori_time_range_index) == FALSE)
    {
        /*unbind the new time-range due to fail*/
        TIME_RANGE_PMGR_RequestTimeRangeEntry(time_range, FALSE, &new_time_range_index);
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /* check if time range status is active */
    if (TIME_RANGE_PMGR_IsTimeRangeEntryActive(time_range) == TRUE)
    {
        status = POE_TYPE_TIMERANGE_STATUS_ACTIVE;
        driver_admin = VAL_pethPsePortAdminEnable_true;
    }
    else
    {
        status = POE_TYPE_TIMERANGE_STATUS_INACTIVE;
        driver_admin = VAL_pethPsePortAdminEnable_false;
    }

    if (POEDRV_SetPortAdminStatus(group_index, port_index, driver_admin) == FALSE)
    {
        /*unbind the new time-range due to fail*/
        TIME_RANGE_PMGR_RequestTimeRangeEntry(time_range, FALSE, &new_time_range_index);    
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (ori_time_range_index != TIME_RANGE_TYPE_UNDEF_TIME_RANGE)
    {
        /*unbind the old time range*/
        POE_OM_GetPsePortTimeRangeName(group_index, port_index, ori_time_range);
        TIME_RANGE_PMGR_RequestTimeRangeEntry(ori_time_range, FALSE, &ori_time_range_index);        
    }	

    POE_OM_SetPsePortTimeRangeName(group_index, port_index, time_range);
    POE_OM_SetPsePortTimeRangeIndex(group_index, port_index, new_time_range_index);
    POE_OM_SetPsePortTimeRangeStatus(group_index, port_index, status);
    POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
} /* End of POE_MGR_BindTimeRangeToPsePort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_UnbindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Unbind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_UnbindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index)
{
    UI32_T core_admin = VAL_pethPsePortAdminEnable_false;
    UI8_T  time_range_name[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH + 1] = {0};
    UI32_T time_range_index;

    DBG_PRINT("unbinding %lu/%lu", group_index, port_index);
    POE_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
 	
    POE_OM_GetPsePortTimeRangeName(group_index, port_index, time_range_name);
    POE_OM_GetPsePortAdmin(group_index, port_index, &core_admin);

    /* Need to put string/value pointer to do user request.
     */
    if (POEDRV_SetPortAdminStatus(group_index, port_index, core_admin) == FALSE)
    {
        /* 1. Need to put string/value pointer to do user request.
         */
        POE_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /*unbind time range*/
    TIME_RANGE_PMGR_RequestTimeRangeEntry(time_range_name, FALSE, &time_range_index);

    memset((char *) time_range_name, 0, SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH);
	
    POE_OM_SetPsePortTimeRangeIndex(group_index, port_index, TIME_RANGE_TYPE_UNDEF_TIME_RANGE);
    POE_OM_SetPsePortTimeRangeName(group_index, port_index, time_range_name);
    POE_OM_SetPsePortTimeRangeStatus(group_index, port_index, POE_TYPE_TIMERANGE_STATUS_NONE);
    POE_MGR_RETURN_AND_RELEASE_CSC(TRUE);
} /* End of POE_MGR_UnbindTimeRangeToPsePort */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_TimeRangeStatusChange_callback
 * -------------------------------------------------------------------------
 * FUNCTION: time range status change callback function, register to time_range
 * INPUT   : isChanged_list -- if status changed by time range index list
 *           status_list    -- status by time range index list
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_TimeRangeStatusChange_callback(UI8_T *isChanged_list, UI8_T *status_list)
{
    UI32_T i, unit, port, index, status, driver_admin;

    /* check time range status change list */
    for (i = 0; i < SYS_ADPT_TIME_RANGE_MAX_NBR_OF_ENTRY; ++i)
    {
        if (isChanged_list[(UI32_T)(i>>3)] & (1 << (7 - (i%8))))
        {
            if (status_list[(UI32_T)(i>>3)] & (1 << (7 - (i%8))))
            {
                status = POE_TYPE_TIMERANGE_STATUS_ACTIVE;
                driver_admin = VAL_pethPsePortAdminEnable_true;
            }
            else
            {
                status = POE_TYPE_TIMERANGE_STATUS_INACTIVE;
                driver_admin = VAL_pethPsePortAdminEnable_false;
            }

            for (unit=1; unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
            {
                for (port=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; port<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER; port++)
                {
                    if (POE_OM_UserPortExisting(unit, port) != TRUE)
                        continue;

                    POE_OM_GetPsePortTimeRangeIndex(unit, port, &index);
                    if (index == i)
                    {

                        /* PSE function priority: TIME_RANGE CSC > PoE CSC.
                         */
                        POEDRV_SetPortAdminStatus(unit, port, driver_admin);
                        POE_OM_SetPsePortTimeRangeStatus(unit, port, status);
                    }
                }
            }
        }
    }
} /* End of POE_MGR_TimeRangeStatusChange_callback */
#endif

