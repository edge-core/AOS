/* Module Name: SWDRV.C
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides switch driver interface.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port configuration, VLAN, port mirroring,
 *         trunking, spanning tree, IGMP, broadcast storm control, and
 *         port mapping.
 *        ( 3.  The domain would not be handled by this module. )
 *         But this module doesn't include MAC address manipulation and
 *         port statistics.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier        Reason
 *       2001/6/1    Jimmy Lin       Create this file
 *       2002/9/20   Jeff Kao        Add Stacking & Transition Mode
 *       2002/10/23  Dino King       Change called drv from BCMDRV to DEV_SWDRV
 *       2002/10/24  Jeff Kao        move l3swdrv API. to here
 *       2003/1/24   Charles Cheng   Add MAU MIB code.
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "leaf_1493.h"
#include "leaf_2674p.h"
#include "leaf_2674q.h"
#include "leaf_es3626a.h"
#include "leaf_sys.h"
#include "swdrv.h"
#include "leaf_2863.h"
#include "sys_dflt.h"
#include "sysrsc_mgr.h"
#include "dev_swdrv.h"
#if (SYS_CPNT_CRAFT_PORT == TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
#include "stktplg_board.h"
#endif
#include "stktplg_type.h"
#include "stktplg_om.h"
#include "stktplg_pom.h"
#include "stktplg_pmgr.h"
#include "stktplg_shom.h"
#include "swdrv_type.h"
#include "swdrv_lib.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_MGMT_PORT == TRUE)
#include "adm_nic.h"
#endif

#include "sys_module.h"
#include "swdrv_om.h"
#include "sys_callback_mgr.h"
#include "sys_time.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
#include <math.h>
#include "l_stdlib.h"
#include "sysdrv.h"
#include "l_math.h"
#include "l_pbmp.h"
#if (SYS_CPNT_SYSDRV_USE_ONLP==TRUE)
#include "onlpdrv_sfp.h"
#endif
#endif

#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif


#define SWDRV_PORT_FLOW_CONTROL_OPER_STATUS_CHANGED     0x02

/* This options is because some chip get unexpected mean that speed/duplex will not be monitored
 * when the port is link down.
 * 1. never monitor speed/duplex of a linkdown port.
 * 2. when port is changed from up to down, also change oper speed/duplex
 *    to default (according port type) and notify upper layer.
 */
#define SWDRV_FORCE_DEFAULT_OPER_SPEED_DUPLEX_WHEN_LINK_DOWN    TRUE

/* MACRO FUNCTIONS */

#if (SYS_CPNT_STACKING == TRUE)
#define SWDRV_TIME_OUT                  2400            /* time to wait for ISC reply */
#define SWDRV_TRY_TIMES                 4
#define SWDRV_ALL_UNIT                  255            /* all unit number  */
#define MASTER_UNIT                     1
#define SWDRV_OPTION_MODULE             255
#endif /*SYS_CPNT_STACKING*/

/* TYPE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */
    static void   SWDRV_MONITOR_TASK_Main(int arg);
    static void   SWDRV_MONITOR_SFP_Main(int arg);

    static BOOL_T SWDRV_MONITOR_PortExist(UI32_T port);
    static void   SWDRV_MONITOR_OperSpeedDuplex(UI32_T port);

    static void   SWDRV_MONITOR_Module(UI32_T port);
    static void   SWDRV_MONITOR_SfpForModuleInserted(UI32_T port);

    static void   SWDRV_MONITOR_PortSfpPresent(UI32_T port, BOOL_T force_to_check_present);
    static BOOL_T SWDRV_MONITOR_GetSfpPresentStatus(UI32_T stack_id, UI32_T sfp_index, BOOL_T *is_present_p);
    static void   SWDRV_MONITOR_GetSfpPresentStatusCheckDelay(UI32_T sfp_index, UI8_T *is_present_p);
    static void   SWDRV_MONITOR_UpdatePortSfpPresentStatus(UI32_T port, UI32_T sfp_index, BOOL_T is_present);

    static void   SWDRV_MONITOR_LinkStatus(UI32_T port);
    #if (SYS_CPNT_CRAFT_PORT == TRUE)
    static void   SWDRV_MONITOR_CraftPortLinkStatus();
    #endif
    static void   SWDRV_MONITOR_OperFlowCtrl(UI32_T port);
    //static void SWDRV_MONITOR_AllPortOperSpeedDuplex(UI32_T start_port,UI32_T end_port);
    //static void SWDRV_MONITOR_AllPortLinkStatus(UI32_T start_port,UI32_T end_port );
    //static void SWDRV_MONITOR_AllPortOperFlowCtrl(UI32_T start_port,UI32_T end_port);

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    static void SWDRV_MONITOR_CopperEnergyDetect(UI32_T port);
#endif
#if 0
    static void SWDRV_MONITOR_Trunking(void);
#endif

#if (SWDRV_FORCE_DEFAULT_OPER_SPEED_DUPLEX_WHEN_LINK_DOWN == TRUE)
    static BOOL_T SWDRV_MONITOR_GetDefaultPortSpeedDuplex(UI32_T unit, UI32_T port, UI32_T *default_speed_duplex_p);
#endif

#if (SYS_CPNT_HW_LINKSCAN == TRUE)
    static void SWDRV_MONITOR_HardwareLinkStatusChanged_Callback(UI32_T unit, UI32_T port, UI32_T link_status);
#endif

#if (SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE)
    static void SWDRV_MONITOR_PDPortPowerStatus(UI32_T port);
#endif
    static void   SWDRV_MONITOR_SfpInfoWorkaround(UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH]);
    static BOOL_T SWDRV_MONITOR_SfpInfoReadAndParse(UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p);
    static BOOL_T SWDRV_MONITOR_SfpInfoParse(UI8_T sfp_type, UI8_T *info_ar, SWDRV_TYPE_SfpInfo_T *sfp_info_p);
    static float  SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(UI32_T single_float);
    static double SWDRV_MONITOR_DdmConvertSlope(UI16_T slope);
    static double SWDRV_MONITOR_DdmConvertOffset(UI16_T offset);
    static float  SWDRV_MONITOR_DdmConvertTemperature(UI16_T temperature, double slope, double offset);
    static float  SWDRV_MONITOR_DdmConvertVoltage(UI16_T voltage, double slope, double offset);
    static float  SWDRV_MONITOR_DdmConvertCurrent(UI16_T current, double slope, double offset);
    static double SWDRV_MONITOR_DdmConvertTxPower(UI16_T tx_power, double slope, double offset);
    static double SWDRV_MONITOR_DdmConvertRxPower(UI16_T rx_power, float rx_p_4, float rx_p_3, float rx_p_2, float rx_p_1, float rx_p_0);
    static float  SWDRV_MONITOR_DdmUnitWatt2Dbm(double val);
    static UI16_T SWDRV_MONITOR_DdmGetXFPVoltage(UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH], UI8_T aux_monitoring);
    static void   SWDRV_MONITOR_ConvertSfpDdmInfoRaw(UI32_T sfp_index, UI8_T sfp_type, UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH], UI8_T ext_calibrate_needed, UI8_T aux_monitoring, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);
    static BOOL_T SWDRV_MONITOR_SfpDdmInfoParsing(UI32_T sfp_index, UI8_T sfp_type, BOOL_T ext_calibrate_needed, UI8_T aux_monitoring, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);
    static BOOL_T SWDRV_MONITOR_PortSfpInfo(UI32_T unit, UI32_T sfp_index, UI8_T sfp_type);
    static void   SWDRV_MONITOR_PortSfpDdmInfo(UI32_T port);
    static BOOL_T SWDRV_MONITOR_IsSfpRxLosAsserted(UI32_T unit, UI32_T sfp_index);
#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    static BOOL_T SWDRV_MONITOR_GetSfpRxLosStatus(UI32_T unit, UI32_T sfp_index, UI8_T *rx_los_status_p);
#endif

/* STATIC VARIABLE DEFINITIONS
 */
#if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD != SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
static UI32_T swdrv_sfp_present_status_addr[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
#endif
static UI8_T  swdrv_sfp_present_mask[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY)
static UI8_T  swdrv_sfp_present_bit_shift[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
#endif

#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
static UI32_T swdrv_sfp_rx_los_status_addr_ar[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
static UI8_T  swdrv_sfp_rx_los_mask_ar[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
static UI8_T  swdrv_sfp_rx_los_bit_shift_ar[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
#endif
static UI32_T swdrv_sfp_present_ticks_ar[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
static UI8_T  swdrv_sfp_present_stable_bmp[(SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT+7)/8];
static UI8_T  swdrv_sfp_ddm_read_count= 0;
static UI8_T  swdrv_sfp_ddm_error_count[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];

/* EXPORTED SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWDRV_MONITOR_CreateTask
* -------------------------------------------------------------------------
* FUNCTION: This function will create Switch Driver Task. This function
*           will be called by root.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
* -------------------------------------------------------------------------*/
void SWDRV_MONITOR_CreateTask(void)
{
    UI32_T swdrv_thread_id, swdrv_sfp_thread_id;
    if (SYSFUN_SpawnThread(SYS_BLD_SWDRV_TASK_PRIORITY,
                           SYSFUN_SCHED_RR,
                           SYS_BLD_SWDRV_TASK,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           SWDRV_MONITOR_TASK_Main,
                           0,
                           &swdrv_thread_id))
    {

        return;
    }

    if (SYSFUN_SpawnThread(SYS_BLD_SWDRV_SFP_TASK_PRIORITY,
                           SYSFUN_SCHED_DEFAULT,
                           SYS_BLD_SWDRV_SFP_TASK,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           SWDRV_MONITOR_SFP_Main,
                           0,
                           &swdrv_sfp_thread_id))
    {

        return;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SWDRV, swdrv_thread_id, SYS_ADPT_SWDRV_SW_WATCHDOG_TIMER);
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SWDRV_SFP, swdrv_sfp_thread_id, SYS_ADPT_SWDRV_SW_WATCHDOG_TIMER);
#endif

    SWDRV_OM_SetThreadId(swdrv_thread_id);
    SWDRV_OM_SetSfpThreadId(swdrv_sfp_thread_id);
} /* End of SWDRV_MONITOR_CreateTask() */


void SWDRV_MONITOR_UpdateHgPortLinkState()
{
#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    UI32_T status,old_up_state=0,old_down_state=0;
    UI32_T  unit;
    UI32_T dev_id, uplink_stackingport, downlink_stackingport;
    //UI32_T speed_duplex;
    //UI32_T state,uplinkport, downlinkport;
    //SWDRV_Port_Info_T swdrv_port_info;
    if(FALSE==DEV_SWDRV_GetStackChipDeviceID(&unit))
    {
        return;
    }

    if (STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_UP_LINK, &dev_id, &uplink_stackingport) == FALSE)
    {
        return;
    }
    if (STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_DOWN_LINK, &dev_id, &downlink_stackingport) == FALSE)
    {
        return;
    }

    STKTPLG_SHOM_GetHgUpPortLinkState(&old_up_state);
    if(DEV_SWDRV_GetPortLinkStatusByDeviceId(unit, uplink_stackingport, &status))
    {
        if(old_up_state != status)
        {
            STKTPLG_SHOM_SetHgUpPortLinkState(status);
        }
    }

    STKTPLG_SHOM_GetHgDownPortLinkState(&old_down_state);
    if(DEV_SWDRV_GetPortLinkStatusByDeviceId(unit, downlink_stackingport, &status))
    {
        if(old_down_state != status)
        {
            STKTPLG_SHOM_SetHgDownPortLinkState(status);
        }
    }
#endif /* #if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1) */

}


void SWDRV_Linkscan_Callback_Signal(int signum)
{

}

/****************************************************************************/
/* Local Functions                                                          */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_TASK_Main
 * -------------------------------------------------------------------------
 * FUNCTION: Switch driver main task routine
 * INPUT   : arg -- null argument
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_TASK_Main(int arg)
{
    UI32_T      i;
    BOOL_T      swdrv_provision_complete;
    UI32_T      wait_events, received_events;
    SWDRV_Switch_Info_T swdrv_system_info;
    
    //UI32_T speed_duplex; /*add by michael.wang 2008-8-26 for hg port 26 27 */
    //UI32_T status,old_up_state=0,old_down_state=0;
    /* add by fen.wang 2008-8-14, stktplg will read hg state from dev_swdrv termly,but driver sometimes is
     * so slow to relpy stktplg,so change it to share memory to get hg info
     */
    SWDRV_OM_SetThreadIdle(TRUE);

    wait_events = SWDRV_EVENT_ENTER_TRANSITION_MODE;
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    wait_events |= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
#endif

    /* The main body
     */
#if (SYS_CPNT_BCM_LINKSCAN_ENABLE == TRUE)
    DEV_SWDRV_linkscan_register();
    SYSFUN_RegisterSignal(SYS_BLD_SIGNAL_LINK_SCAN, SWDRV_Linkscan_Callback_Signal);
#endif

#if (SYS_CPNT_HW_LINKSCAN == TRUE)
    DEV_SWDRV_Register_LinkStatusChanged(SWDRV_MONITOR_HardwareLinkStatusChanged_Callback);
#endif

    while(1)
    {
        SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_NOWAIT, &received_events);
        if (received_events & SWDRV_EVENT_ENTER_TRANSITION_MODE)
        {
            SWDRV_OM_SetTaskTransitionDone(TRUE);
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(received_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SWDRV);
            received_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* Charles: Add (master mode && provision_complete)
         */
        SWDRV_OM_GetProvisionComplete(&swdrv_provision_complete);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        /* if we support sw watchdog, swdrv needs to check whether it has get
         * the sw watchdog monitor event. Don't use while-loop there
         */
        if(((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE) && swdrv_provision_complete) ||
           ((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE ) && swdrv_provision_complete) )
#else
        while(((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE) && swdrv_provision_complete) ||
              ((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE ) && swdrv_provision_complete) )
#endif
        {
            memset(&swdrv_system_info, 0, sizeof(SWDRV_Switch_Info_T));
            SWDRV_OM_GetSystemInfo(&swdrv_system_info);

            #if (SYS_CPNT_CRAFT_PORT == TRUE)
            SWDRV_MONITOR_CraftPortLinkStatus();
            #endif
            for(i= swdrv_system_info.base_port_id;
                i<=swdrv_system_info.base_port_id+swdrv_system_info.port_number-1;
                i++)
            {

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
                /* gordon_kao: when port is loopbacking, the polling is nonsense
                 */

                if(!SWDRV_MONITOR_PortExist(i) || DEV_SWDRV_IsPortLoopbacking(i))
                    continue;
#else
                if(!SWDRV_MONITOR_PortExist(i))
                    continue;
#endif

                SWDRV_OM_SetThreadIdle(FALSE);

#if (SYS_CPNT_HW_LINKSCAN != TRUE) || \
    (SYS_CPNT_DEVSWDRV_FORCE_LINK_UP_WHEN_PHY_IS_UP == TRUE) || \
    (SWDRV_FORCE_DEFAULT_OPER_SPEED_DUPLEX_WHEN_LINK_DOWN == TRUE)
                SWDRV_MONITOR_LinkStatus(i);
                //SWDRV_MONITOR_AllPortLinkStatus(i,i);
#endif

                SWDRV_MONITOR_OperFlowCtrl(i);

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
                SWDRV_MONITOR_CopperEnergyDetect(i);
#endif

                SWDRV_MONITOR_Module(i);

#if (SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE)
                SWDRV_MONITOR_PDPortPowerStatus(i);
#endif
            } /* end of for(i= swdrv_system_info.base_port_id; ... */

            //SWDRV_MONITOR_AllPortLinkStatus(swdrv_system_info.base_port_id,swdrv_system_info.base_port_id+swdrv_system_info.port_number-1);
            //SWDRV_MONITOR_AllPortOperSpeedDuplex(swdrv_system_info.base_port_id,swdrv_system_info.base_port_id+swdrv_system_info.port_number-1);
            //SWDRV_MONITOR_AllPortOperFlowCtrl(swdrv_system_info.base_port_id,swdrv_system_info.base_port_id+swdrv_system_info.port_number-1);
            if(SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
            {
                SWDRV_Notify_PortLinkStatus2UpperLayer();
            }
            else if (SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
            {
                SWDRV_Notify_PortLinkStatus2Master();
            }

            /*SWDRV_MONITOR_Trunking();*//* Be deprecated by Distributed Trunk, arthur 2002-11-01 */
            DEV_SWDRV_LinkScan_Update();
            SWDRV_MONITOR_UpdateHgPortLinkState();
            SWDRV_OM_SetThreadIdle(TRUE);
            #if (SYS_CPNT_BCM_LINKSCAN_ENABLE == TRUE)
            SYSFUN_Sleep_Interruptible(SYS_BLD_SWDRV_UPDATE_PORT_STATE_INTERVAL);
            #else
            SYSFUN_Sleep(SYS_BLD_SWDRV_UPDATE_PORT_STATE_TICKS);
            #endif
        }
        SWDRV_MONITOR_UpdateHgPortLinkState();
        SYSFUN_Sleep(SYS_BLD_SWDRV_UPDATE_PORT_STATE_TICKS);
    } /* while(1) */
} /* End of SWDRV_MONITOR_TASK_Main() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SFP_Main
 * -------------------------------------------------------------------------
 * FUNCTION: Switch driver main task routine for SFP eeprom related operation
 * INPUT   : arg -- null argument
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_SFP_Main(int arg)
{
    UI32_T      i;
    BOOL_T      swdrv_provision_complete;
    UI32_T      wait_events, received_events;
    SWDRV_Switch_Info_T swdrv_system_info;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    wait_events = SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
#endif

#if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD != SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
    SWDRV_OM_GetSfpPresentStatusAddr(swdrv_sfp_present_status_addr);
#endif
    SWDRV_OM_GetSfpPresentMask(swdrv_sfp_present_mask);
#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY)
    SWDRV_OM_GetSfpPresentBitShift(swdrv_sfp_present_bit_shift);
#endif

#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    SWDRV_OM_GetSfpRxLosStatusAddr(swdrv_sfp_rx_los_status_addr_ar);
    SWDRV_OM_GetSfpRxLosMask(swdrv_sfp_rx_los_mask_ar);
    SWDRV_OM_GetSfpRxLosBitShift(swdrv_sfp_rx_los_bit_shift_ar);
#endif

    while(1)
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_NOWAIT, &received_events);
        if(received_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SWDRV_SFP);
            received_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* Charles: Add (master mode && provision_complete)
         */
        SWDRV_OM_GetProvisionComplete(&swdrv_provision_complete);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        /* if we support sw watchdog, swdrv needs to check whether it has get
         * the sw watchdog monitor event. Don't use while-loop there
         */
        if(((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE) && swdrv_provision_complete) ||
           ((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE ) && swdrv_provision_complete) )
#else
        while(((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE) && swdrv_provision_complete) ||
              ((SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE ) && swdrv_provision_complete) )
#endif
        {
            memset(&swdrv_system_info, 0, sizeof(SWDRV_Switch_Info_T));
            SWDRV_OM_GetSystemInfo(&swdrv_system_info);

            for(i= swdrv_system_info.base_port_id;
                i<=swdrv_system_info.base_port_id+swdrv_system_info.port_number-1;
                i++)
            {
            #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
                if(!SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_STOP_MONITOR))
                    SWDRV_MONITOR_PortSfpPresent(i, FALSE);
            #endif
            } /* end of for(i= swdrv_system_info.base_port_id; ... */
            swdrv_sfp_ddm_read_count++;

        #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
            SWDRV_Notify_PortSfpPresent();
        #endif
        }
        SYSFUN_Sleep(SYS_BLD_SWDRV_UPDATE_PORT_STATE_TICKS);
    } /* while(1) */
} /* End of SWDRV_MONITOR_TASK_Main() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_PortExist
 * -------------------------------------------------------------------------
 * FUNCTION: Check if this port is existing or not. If changed, call back
 *           (master) or send notification through ISC(slave).
 * INPUT   : port -- the port to check
 * OUTPUT  : None
 * RETURN  : True: If existing; False: If not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRV_MONITOR_PortExist(UI32_T port)
{
    UI32_T port_type;
    UI32_T stack_id;
    UI32_T old_fc_status;
    SWDRV_Port_Info_T swdrv_port_info;

    memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);
    old_fc_status = swdrv_port_info.flow_control_oper;

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (SYS_ADPT_MGMT_PORT == port)
    {
        if(swdrv_port_info.existing == FALSE)
        {
            SWDRV_OM_SetPortInfoExisting(port, TRUE);
            SWDRV_OM_SetPortInfoSpeedDuplexOper(port, 0);
            SWDRV_OM_SetPortInfoFlowControlOper(port, VAL_portFlowCtrlStatus_backPressure);
            if (old_fc_status != VAL_portFlowCtrlStatus_backPressure)
            {
                SWDRV_Notify_PortFlowCtrl(stack_id, port, VAL_portFlowCtrlStatus_backPressure);
            }

            SWDRV_Notify_HotSwapInsert(stack_id, port);
        }
        return TRUE;
    }
#endif

#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    /* If the port is stacking port, no need to be monitored
     */
    if (swdrv_port_info.port_type == STKTPLG_PORT_TYPE_STACKING)
    {
        return FALSE;
    }
#endif

    if(STKTPLG_POM_PortExist(stack_id, port))
    {
        if(swdrv_port_info.existing == FALSE)
        {
            STKTPLG_POM_GetPortType(stack_id, port, &port_type);
            SWDRV_OM_SetPortInfoPortType(port, port_type);
            SWDRV_OM_SetPortInfoLinkStatus(port, FALSE);
            if(port_type == VAL_portType_hundredBaseTX)
            {
                SWDRV_OM_SetPortInfoSpeedDuplexOper(port, 0);
                SWDRV_OM_SetPortInfoFlowControlOper(port, VAL_portFlowCtrlStatus_backPressure);

                if (old_fc_status != VAL_portFlowCtrlStatus_backPressure)
                {
                    SWDRV_Notify_PortFlowCtrl(stack_id, port, VAL_portFlowCtrlStatus_backPressure);
                }
            }
            else
            {
                SWDRV_OM_SetPortInfoSpeedDuplexOper(port, VAL_portSpeedDpxStatus_fullDuplex1000);
                SWDRV_OM_SetPortInfoFlowControlOper(port, VAL_portFlowCtrlStatus_dot3xFlowControl);
                if (old_fc_status != VAL_portFlowCtrlStatus_dot3xFlowControl)
                {
                    SWDRV_Notify_PortFlowCtrl(stack_id, port, VAL_portFlowCtrlStatus_dot3xFlowControl);
                }
            }
            SWDRV_OM_SetPortInfoExisting(port, TRUE);
            SWDRV_Notify_HotSwapInsert(stack_id, port);
        }
        return TRUE;
    }
    else
    {

        if(swdrv_port_info.existing == TRUE)
        {
            SWDRV_OM_SetPortInfoPortType(port, STKTPLG_PORT_TYPE_NOT_EXIST);
            SWDRV_OM_SetPortInfoExisting(port, FALSE);
            SWDRV_Notify_HotSwapRemove(stack_id, port);
        }
        return FALSE;
    }
} /* End of SWDRV_MONITOR_PortExist() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_OperSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: Check if speed or duplex changed. If changed, call back(master)
 *           or send notification through ISC(slave).
 * INPUT   : port -- the port to check
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_OperSpeedDuplex(UI32_T port)
{
    UI32_T speed_duplex;
    BOOL_T retval;
    UI32_T  stack_id;
    SWDRV_Port_Info_T swdrv_port_info;

    memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

    SWDRV_OM_GetSystemInfoStackId(&stack_id);
#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (SYS_ADPT_MGMT_PORT == port)
    {
        if (VAL_portSpeedDpxStatus_fullDuplex100 != (UI32_T )swdrv_port_info.speed_duplex_oper)
        {
            SWDRV_OM_SetPortInfoSpeedDuplexOper(port, VAL_portSpeedDpxStatus_fullDuplex100);
            SWDRV_Notify_PortSpeedDuplex(stack_id, port, VAL_portSpeedDpxStatus_fullDuplex100);
        }
        return;
    }
#endif

    retval = DEV_SWDRV_GetPortOperSpeedDuplex(stack_id, port, &speed_duplex);

    if (retval)
    {
        if(speed_duplex != swdrv_port_info.speed_duplex_oper)
        {
            SWDRV_OM_SetPortInfoSpeedDuplexOper(port, speed_duplex);
            SWDRV_Notify_PortSpeedDuplex(stack_id, port, speed_duplex);
        }
    }
} /* End of SWDRV_MONITOR_OperSpeedDuplex() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_Module
 * -------------------------------------------------------------------------
 * FUNCTION: Monitor Module
 * INPUT   : port -- the port to check
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_MONITOR_Module(UI32_T port)
{
    SWDRV_Port_Info_T swdrv_port_info;
    UI32_T stack_id, port_type;
    UI8_T module_id = 0;

    SWDRV_OM_GetSystemInfoStackId(&stack_id);
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

    if (!STKTPLG_POM_GetModuleID(stack_id, port, &module_id))
    {
        module_id = 0;
    }

    if (swdrv_port_info.module_id != module_id)
    {
        SWDRV_OM_SetPortInfoModuleId(port, module_id);

        if (module_id != 0)
        {
            if (!STKTPLG_POM_GetPortType(stack_id, port, &port_type))
            {
                return;
            }

            SWDRV_MONITOR_SfpForModuleInserted(port);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SfpForModuleInserted
 * -------------------------------------------------------------------------
 * FUNCTION: Monitor sfp transceiver of Module
 * INPUT   : port -- the port to check
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_MONITOR_SfpForModuleInserted(UI32_T port)
{
    /* reset media-type/port-type info
     * and trigger SWDRV_MonitorSfpTransceive to
     * detect new port type.
     */

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
    UI32_T stack_id;
    UI32_T media_cap;

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

    if (!STKTPLG_POM_GetPortMediaCapability(stack_id, port, &media_cap))
    {
        media_cap = 0;
    }

    if ((media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER) && (media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER))
    {
        SWDRV_OM_SetComboForceMode(port, SYS_DFLT_COMBO_PORT_FORCED_MODE);
    }
    else if ((media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER))
    {
        SWDRV_OM_SetComboForceMode(port, VAL_portComboForcedMode_sfpForced);
    }
    else
    {
        SWDRV_OM_SetComboForceMode(port, VAL_portComboForcedMode_copperForced);
    }

#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
    SWDRV_OM_SetComboForceSpeed(port, VAL_portType_other);
#endif
#endif

    SWDRV_OM_SetPortInfoPortType(port, VAL_portType_other);
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    SWDRV_MONITOR_PortSfpPresent(port, TRUE);
#endif
}

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_PortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Check if sfp present status changed. If changed, call back or send
 *           notification through ISC(slave).
 * INPUT   : port -- the port to check
 *           force_to_check_present
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_MONITOR_PortSfpPresent(UI32_T port, BOOL_T force_to_check_present)
{
    UI32_T stack_id, sfp_index;
    BOOL_T is_present_new = FALSE;
    BOOL_T is_present_old = FALSE;
    BOOL_T sfp_info_valid = FALSE;
    UI8_T  sfp_type;
    BOOL_T is_break_out;

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

    if(FALSE == STKTPLG_OM_UserPortToSfpIndexAndType(stack_id, port, &sfp_index, &sfp_type, &is_break_out))
    {
        return;
    }
    if (is_break_out == TRUE)
    {
        sfp_type = SWDRV_TYPE_GBIC_ID_QSFP;
    }

    if(SWDRV_MONITOR_GetSfpPresentStatus(stack_id, sfp_index, &is_present_new))
    {
    #if (SYS_CPNT_SWDRV_COMBO_PORT_MEDIA_PREFERRED_AUTO_BY_HW == TRUE)
        /* HW medium detect
         *
         * fiber detected if
         *     1) sfp inserted and
         *     2) phy fiber medium is active
         */
        {
            UI32_T port_media;

            if (!DEV_SWDRV_PMGR_GetPortMediaActive(stack_id, port, &port_media))
                port_media = DEV_SWDRV_PORT_MEDIA_UNKNOWN;

            is_present_new = is_present_new && (port_media == DEV_SWDRV_PORT_MEDIA_FIBER);
        }
    #else
        /* SW medium detect
         *
         * fiber detected if
         *     1) sfp inserted and
         *     2) rx_loss not asserted (optional)
         */

        #if (SYS_CPNT_SWDRV_COMBO_PORT_SWITCH_TO_FIBER_WHEN_RX_LOS_DEASSERT==TRUE)
        if(is_present_new)
        {
            BOOL_T rx_los_status;

            rx_los_status = SWDRV_MONITOR_IsSfpRxLosAsserted(SYS_VAL_LOCAL_UNIT_ID, sfp_index);
            is_present_new = !rx_los_status;
        }
        #endif
    #endif /* (SYS_CPNT_SWDRV_COMBO_PORT_MEDIA_PREFERRED_AUTO_BY_HW == TRUE) */

        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
        {
            if(sfp_index == 1)
                BACKDOOR_MGR_Printf("\r\n");
            if(is_present_new)
                BACKDOOR_MGR_Printf("%s(%d): unit:%lu, port:%2lu: %s\r\n", __FUNCTION__, __LINE__, stack_id, port, "present");
        }

        SWDRV_OM_GetPortSfpPresent(sfp_index, &is_present_old);

        if(is_present_new != is_present_old || force_to_check_present)
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
                BACKDOOR_MGR_Printf("%s-%d unit:%ld, port:%2ld: status changes to %s\r\n", __FUNCTION__, __LINE__, stack_id, port, is_present_new?"present":"not present");
            SWDRV_MONITOR_UpdatePortSfpPresentStatus(port, sfp_index, is_present_new);

        /* change from not present to present */
            if(is_present_new)
            {
                if(TRUE == SWDRV_MONITOR_PortSfpInfo(stack_id, sfp_index, sfp_type))
                    sfp_info_valid = TRUE;
                else
                    sfp_info_valid = FALSE;
                SWDRV_OM_SetPortSfpInfoValid(sfp_index, sfp_info_valid);
            }
        /* SWDRV_SfpInserted will use bitrate which is read from EEPROM info */
            if (is_present_new && sfp_info_valid)
            {
                SWDRV_SfpInserted(stack_id, sfp_index);
            }
            else
            {
                SWDRV_SfpRemoved(stack_id, sfp_index);
            }
        }
        else
        {
            if(is_present_new)
            {
                SWDRV_OM_GetPortSfpInfoValid(sfp_index, &sfp_info_valid);
                if(sfp_info_valid == FALSE)
                {
                    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
                        BACKDOOR_MGR_Printf("%s-%d read SFP%ld EEPROM again for previous fail\r\n", __FUNCTION__, __LINE__, sfp_index);
                /* sfp info is not valid due to fail to read sfp when first present */
                    if(TRUE == SWDRV_MONITOR_PortSfpInfo(stack_id, sfp_index, sfp_type))
                        sfp_info_valid = TRUE;
                    else
                        sfp_info_valid = FALSE;

                /* SWDRV_SfpInserted will use bitrate which is read from EEPROM info */
                    if (sfp_info_valid)
                    {
                        SWDRV_SfpInserted(stack_id, sfp_index);
                    }

                    SWDRV_OM_SetPortSfpInfoValid(sfp_index, sfp_info_valid);
                }
                else if((swdrv_sfp_ddm_read_count%6) == 0)
                {
                /* Update DDM every SYS_BLD_SWDRV_UPDATE_PORT_STATE_TICKS * 6
                 * = 50 * 6 ticks = 3000 ms = 3 second
                 */
                    SWDRV_MONITOR_PortSfpDdmInfo(port);
                }
            }
        }
    }
} /* End of SWDRV_MONITOR_PortSfpPresent() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_GetSfpPresentStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Check if sfp is present or not
 * INPUT   : stack_id
 *           sfp_index
 * OUTPUT  : is_present_p
 * RETURN  : None
 * NOTE    : porting from STKTPLG_OM_GetSFPPresentStatus()
 * -------------------------------------------------------------------------
 */
static BOOL_T SWDRV_MONITOR_GetSfpPresentStatus(UI32_T stack_id, UI32_T sfp_index, UI8_T *is_present_p)
{
#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE)
    UI32_T board_id = 0;
    UI8_T  value_info;

    /* if SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD is defined
     *     SYS_HWCFG_REG_ACCESS_METHOD_I2C:
     *        es3528mv2_flf_38
     *        es4627mb_flf_ec
     *        ecs4620_28t
     *     SYS_HWCFG_REG_ACCESS_METHOD_SYS_HWCFG_API:
     *        ecs3510_28t
     *        ecs4910_28f
     *     SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL:
     *        ecs5610_52s
     *     SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR:
     *        ecs3580_52t
     *        ecs4510_12pd
     *        es3510ma_flf_38
     *        es3628bt_flf_zz
     *        es4624h_sfp_flf_38
     *        es4626f_flf_38
     *        es4650ba_flf_ec
     */
    if(sfp_index < 1 || sfp_index > SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT)
    {
        return FALSE;
    }

    if(STKTPLG_OM_GetUnitBoardID(stack_id , &board_id) == FALSE)
    {
        printf("%s(%d): Failed to get my board id.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

#if defined(SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD)
#if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_I2C)
    #if defined(ES4627MB)
    if(board_id <= 1 || board_id == 5)
    {
        if(I2CDRV_GetI2CInfo(SYS_HWCFG_SFP_PRESENT_ASIC_I2C_ADDRESS, swdrv_sfp_present_status_addr[sfp_index-1], 1, &value_info)==FALSE)
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
                BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
            return FALSE;
        }
    }
    else if(board_id == 2 || board_id == 6)
    {
        if(I2CDRV_TwsiDataReadWithBusIdx(1, SYS_HWCFG_SFP_PRESENT_ASIC_I2C_ADDRESS,
            I2C_7BIT_ACCESS_MODE, TRUE, swdrv_sfp_present_status_addr[sfp_index-1],
            FALSE, 1, &value_info)==FALSE)
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
                BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
            return FALSE;
        }
    }
    else if(board_id == 3 || board_id == 4)
    {
        if(I2CDRV_GetI2CInfo(SYS_HWCFG_SFP_PRESENT_ASIC_I2C_ADDRESS, swdrv_sfp_present_status_addr[sfp_index-1], 1, &value_info)==FALSE)
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
                BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
            return FALSE;
        }
    }
    #elif defined(ASF4512MP) /* #if defined(ES4627MB) */
    if(board_id == 0)
    {
        if(I2CDRV_GetI2CInfo(SYS_HWCFG_SFP_PRESENT_ASIC_I2C_ADDRESS, swdrv_sfp_present_status_addr[sfp_index-1], 1, &value_info)==FALSE)
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
                BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
            return FALSE;
        }
    }
    else if(board_id == 1)
    {
        if(!SYS_HWCFG_GetSFPPresentStatus(board_id, sfp_index, &value_info))
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
                BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
            return FALSE;
        }
    }
    #else /* #if defined(ASF4512MP) */
    if(I2CDRV_GetI2CInfo(SYS_HWCFG_SFP_PRESENT_ASIC_I2C_ADDRESS, swdrv_sfp_present_status_addr[sfp_index-1], 1, &value_info)==FALSE)
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
            BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }
    #endif /* end of #if defined(ES4627MB) */
#elif (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_SYS_HWCFG_API)
    if(!SYS_HWCFG_GetSFPPresentStatus(board_id, sfp_index, &value_info))
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
            BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }
#elif (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
    SYS_HWCFG_i2cRegAndChannelInfo_T reg_info;
    BOOL_T                           rc;
    BOOL_T                           get_val_ok=FALSE;

    if(SYS_HWCFG_GetSFPPresentStatusRegInfo(board_id, sfp_index, &reg_info) == FALSE)
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
            BACKDOOR_MGR_Printf("%s(%d): Access sfp present status reg info for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }

    /* Set and lock Mux if required
     */
    if(reg_info.channel_val)
    {
        rc = I2CDRV_SetAndLockMux(reg_info.i2c_mux_index, reg_info.channel_val);
        if(rc==FALSE)
        {
            printf("%s(%d):Failed to set and lock mux index %u, channel_bmp=0x%02X\r\n",
                __FUNCTION__, __LINE__, reg_info.i2c_mux_index, reg_info.channel_val);
            return FALSE;
        }
    }

    /* Get value from the register
     */
    get_val_ok=I2CDRV_TwsiDataReadWithBusIdx(reg_info.i2c_reg_info.bus_idx,
        reg_info.i2c_reg_info.dev_addr, reg_info.i2c_reg_info.op_type,
        reg_info.i2c_reg_info.validOffset, reg_info.i2c_reg_info.offset,
        reg_info.i2c_reg_info.moreThen256, reg_info.i2c_reg_info.data_len,
        &value_info);

    /* Unlock Mux if required
     */
    if(reg_info.channel_val)
    {
        rc = I2CDRV_UnLockMux(reg_info.i2c_mux_index);
        if(rc == FALSE)
        {
            printf("%s(%d):Failed to unlock mux index %u\r\n", __FUNCTION__, __LINE__, reg_info.i2c_mux_index);
        }
    }

    if(get_val_ok == FALSE)
    {
        /*printf("%s(%d): Failed to get present status from the register for sfp index %lu\r\n",
            __FUNCTION__, __LINE__, sfp_index);*/
        return FALSE;
    }
#elif (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR)
    if(FALSE == PHYSICAL_ADDR_ACCESS_Read(swdrv_sfp_present_status_addr[sfp_index-1], 1, 1, &value_info))
    {
            BACKDOOR_MGR_Printf("%s(%d): Access sfp present status for sfp %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }
#else
    #error "Not impelmented. (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD)"
#endif /* end of #if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_I2C) */
#endif /* end of #if defined(SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD) */

#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY)
    value_info = (value_info & swdrv_sfp_present_mask[sfp_index-1])
                 >> swdrv_sfp_present_bit_shift[sfp_index-1];
#else
    value_info &= swdrv_sfp_present_mask[sfp_index-1];
#endif
    *is_present_p = (value_info == SYS_HWCFG_GBIC_PRESENT_BIT)?TRUE:FALSE;
#else /* #if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE) */
    UI32_T port=0;

    if (STKTPLG_POM_SfpIndexToUserPort(stack_id, sfp_index, &port)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d):STKTPLG_POM_SfpIndexToUserPort error.(Unit %lu Sfp_index %lu)\r\n",
            __FUNCTION__, __LINE__, stack_id, sfp_index);
        return FALSE;
    }
    if (ONLPDRV_SFP_GetSfpPresentStatus(stack_id, port, is_present_p)==FALSE)
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_PRESENT))
        {
            BACKDOOR_MGR_Printf("%s(%d):ONLPDRV_SFP_GetSfpPresentStatus error.(Unit %lu Sfp_index %lu)\r\n",
                __FUNCTION__, __LINE__, stack_id, sfp_index);
        }
        return FALSE;
    }
#endif /* end of #if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE) */
    SWDRV_MONITOR_GetSfpPresentStatusCheckDelay(sfp_index, is_present_p);

    return TRUE;
} /* End of SWDRV_MONITOR_GetSfpPresentStatus() */
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_LinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Check if link state changed. If changed, call back or send
 *           notification through ISC(slave).
 * INPUT   : port -- the port to check
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_LinkStatus(UI32_T port)
{
    UI32_T status;
    UI32_T stack_id;
    SWDRV_Port_Info_T swdrv_port_info;

    memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

#ifdef PEGASUS
    UI32_T sfp_index;/*Wingson 2004-09-21,added for checking sfp existence*/
#endif

#if (SYS_CPNT_MGMT_PORT == TRUE)
    BOOL_T is_link_up;
    if (SYS_ADPT_MGMT_PORT == port)
    {
        is_link_up = ADM_NIC_IsLinkUp();

        /* 1. monitor speed duplex is only performed when port is up
         * 2. shall notify speed duplex changed before link status changed
         */
        if (is_link_up)
        {
            SWDRV_MONITOR_OperSpeedDuplex(port);
        }

        if (is_link_up)
        {
            status = (UI8_T)(DEV_SWDRV_NORMAL_PORT_TYPE|DEV_SWDRV_PORT_LINK_UP);
        }
        else
        {
            status = (UI8_T)DEV_SWDRV_PORT_LINK_DOWN;
        }

        if (status != swdrv_port_info.link_status)
        {
            SWDRV_LocalUpdatePortLinkStatus(port, (UI8_T)status);
        }
        return;
    }
#endif

    /*Wingson 2004-09-21, not check link status if sfp is not exist*/
#ifdef PEGASUS
    if (TRUE == STKTPLG_POM_UserPortToSfpIndex(stack_id, port, &sfp_index))
    {
        if (swdrv_port_info.sfp_present == FALSE)
        {
            return;
        }
    }
#endif
    /*Wingson 2004-09-21, end of new code*/

    /* never check port type and notify port type change any more,
     * by new spec, port type changing base on transceiver insetion/removal
     */
    if(DEV_SWDRV_GetPortLinkStatus(stack_id, port, &status))
    {
        /* 1. monitor speed duplex is only performed when port is up
         * 2. shall notify speed duplex changed before link status changed
         */
        if (status == DEV_SWDRV_PORT_LINK_UP)
        {
            SWDRV_MONITOR_OperSpeedDuplex(port);
        }

        if(status != swdrv_port_info.link_status)
        {
            SWDRV_LocalUpdatePortLinkStatus(port, (UI8_T)status);
        }

#if (SWDRV_FORCE_DEFAULT_OPER_SPEED_DUPLEX_WHEN_LINK_DOWN == TRUE)
        if (status == DEV_SWDRV_PORT_LINK_DOWN)
        {
            /* if link status is changed from up to down,
             * set oper speedDuplex to default.
             */
            UI32_T default_speed_duplex;

            SWDRV_MONITOR_GetDefaultPortSpeedDuplex(stack_id, port, &default_speed_duplex);

            if (swdrv_port_info.speed_duplex_oper != default_speed_duplex)
            {
                SWDRV_OM_SetPortInfoSpeedDuplexOper(port, default_speed_duplex);
                SWDRV_Notify_PortSpeedDuplex(stack_id, port, default_speed_duplex);
            }
        }
#endif
    } /* End of if(DEV_SWDRV_GetPortLinkStatus(stack_id, port, &status)) */
} /* End of SWDRV_MONITOR_LinkStatus() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_CraftPortLinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Check if link state changed. If changed, call back or send
 *           notification through ISC(slave).
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
#if (SYS_CPNT_CRAFT_PORT == TRUE)
static void SWDRV_MONITOR_CraftPortLinkStatus()
{
#if(SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
    SWDRV_CraftPort_Info_T swdrv_craftport_info;
    UI32_T status;
    UI32_T stack_id;

    memset(&swdrv_craftport_info, 0, sizeof(SWDRV_CraftPort_Info_T));
    SWDRV_OM_GetCraftPortInfo(&swdrv_craftport_info);

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

    if(DEV_SWDRV_GetCraftPortLinkStatus(stack_id, &status))
    {
        if(status != swdrv_craftport_info.link_status)
        {
            SWDRV_OM_SetCraftPortInfoLinkStatus(status);

            if(status == DEV_SWDRV_PORT_LINK_UP)
            {
                SWDRV_Notify_CraftPortLinkUp(stack_id);
            }
            else if(status == DEV_SWDRV_PORT_LINK_DOWN)
            {
                SWDRV_Notify_CraftPortLinkDown(stack_id);
            }
        }

    }
#endif
    return;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_OperFlowCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Check if flow control state changed. If changed, call back or
 *           send notification through ISC(slave).
 * INPUT   : port -- the port to check
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_OperFlowCtrl(UI32_T port)
{
    UI32_T flow_control;
    BOOL_T retval;
    UI32_T stack_id;

    SWDRV_Port_Info_T swdrv_port_info;

#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (SYS_ADPT_MGMT_PORT == port)
        return;
#endif

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

    memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

    retval = DEV_SWDRV_GetPortFlowCtrl(stack_id, port, &flow_control);

    if(retval)
    {
        if(flow_control != swdrv_port_info.flow_control_oper)
        {
            swdrv_port_info.flow_control_oper = (UI8_T )flow_control;
#if 0
            switch(flow_control)
            {
                case VAL_portFlowCtrlCfg_dot3xFlowControl:
                    f_c = VAL_portFlowCtrlStatus_dot3xFlowControl;
                    break;
                case VAL_portFlowCtrlCfg_backPressure:
                    f_c = VAL_portFlowCtrlStatus_backPressure;
                    break;
                case VAL_portFlowCtrlCfg_disabled:
                    f_c = VAL_portFlowCtrlStatus_none;
                    break;
                default:
                    f_c = 0;
            }
            if (f_c == 0)
                return;
#endif
            SWDRV_OM_SetPortInfoFlowControlOper(port, flow_control);
            SWDRV_Notify_PortFlowCtrl(stack_id, port, flow_control);
        }
    }
} /* End of SWDRV_MONITOR_OperFlowCtrl() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_Trunking
 * -------------------------------------------------------------------------
 * FUNCTION: Check the link status of trunk members and set to bcmdrv
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
#if 0
static void SWDRV_MONITOR_Trunking(void)
{
#define BCM_LIMITATION_PORTLIST_BYTE  8
    UI8_T  link_status_org;
    int    i, j;
    UI32_T port, port_count, broadcast_port;
    UI8_T port_bit_map[BCM_LIMITATION_PORTLIST_BYTE]; /* broadcom limitation 64 bits */

    for(i=1; i<=SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; i++)
    {
        if(!(shmem_data_p->swdrv_trunk_info[i].used))
            continue;

        /* get the link status of trunk members
         */
        link_status_org = shmem_data_p->swdrv_trunk_info[i].link_status;
        SWDRV_EnterCriticalSection();
        shmem_data_p->swdrv_trunk_info[i].link_status = 0;
        SWDRV_LeaveCriticalSection();
        for(j=0; j<shmem_data_p->swdrv_trunk_info[i].member_number; j++)
        {
            port = shmem_data_p->swdrv_trunk_info[i].member_list[j];
            if(shmem_data_p->swdrv_port_info[port].link_status)
            {
                SWDRV_EnterCriticalSection();
                (shmem_data_p->swdrv_trunk_info[i].link_status) |= 0x1 << j;
                SWDRV_LeaveCriticalSection();
            }
        }

        /* if not the same as before, set again
         */
        if(shmem_data_p->swdrv_trunk_info[i].link_status != link_status_org)
        {
            port_count = 0;
            memset(port_bit_map, 0, BCM_LIMITATION_PORTLIST_BYTE);
            broadcast_port = 0;
            for(j=0; j<shmem_data_p->swdrv_trunk_info[i].member_number; j++)
            {
                if(shmem_data_p->swdrv_trunk_info[i].link_status & (0x1 << j))
                {
                    port_count++;
                    port_bit_map[((shmem_data_p->swdrv_trunk_info[i].member_list[j]+7)/8)-1] |=
                        0x80 >> ((shmem_data_p->swdrv_trunk_info[i].member_list[j] - 1) % 8);
                    if((broadcast_port == 0) ||
                       (broadcast_port > shmem_data_p->swdrv_trunk_info[i].member_list[j]))
                        broadcast_port = shmem_data_p->swdrv_trunk_info[i].member_list[j];
                }
            }
            BCMDRV_SetTrunkPorts(shmem_data_p->swdrv_trunk_info[i].bcmdrv_trunk_id,
                                 port_count,
                                 port_bit_map,
                                 broadcast_port);
        }
    }
} /* End of SWDRV_MONITOR_Trunking() */
#endif


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_AllPortStatus    zhenhua_wu add
 * -------------------------------------------------------------------------
 * FUNCTION: Check if link state changed. If changed, call back or send
 *           notification through ISC(slave).
 * INPUT
 *           UI32_T start_port      -   start port number
 *           UI32_T end_port        -   end port number
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
#if 0
static void SWDRV_MONITOR_AllPortStatus(UI32_T start_port,UI32_T end_port )
{
    UI32_T status;
    UI32_T stack_id;
    SWDRV_Port_Info_T swdrv_port_info;
    int port;
    DEV_SWDRV_PortTableStats_T PortStats;


    SWDRV_OM_GetSystemInfoStackId(&stack_id);
    if(DEV_SWDRV_PMGR_GetAllPortStatus(stack_id, start_port,end_port, &PortStats)==FALSE)
     {
         printf("%s(fail):line = %d\n", __FUNCTION__,__LINE__);
     }


   for(port=start_port;port<=end_port; port++)
    {


    #if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
                    /* gordon_kao: when port is loopbacking, the polling is nonsense
                     */
            if(!SWDRV_MONITOR_PortExist(port) || DEV_SWDRV_PMGR_IsPortLoopbacking(port))
                  continue;
    #else
            if(!SWDRV_MONITOR_PortExist(port))
                  continue;
    #endif

    SWDRV_MonitorSfpTransceiver(port);

    #if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
             SWDRV_MONITOR_CopperEnergyDetect(port);
    #endif

     memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
     SWDRV_OM_GetPortInfo(port, &swdrv_port_info);


     if(PortStats.portLinkStatus[port-1] != swdrv_port_info.link_status)
        {

        SWDRV_LocalUpdatePortLinkStatus(port, (UI8_T)PortStats.portLinkStatus[port-1]);
            if(PortStats.portLinkStatus[port-1])
            {
               SWDRV_Notify_PortLinkUp(stack_id, port);
            }
            else
            {
               SWDRV_Notify_PortLinkDown(stack_id, port);
            }
        }
        if(PortStats.portSpeedDuplex[port-1] != 0)
        {
        if(PortStats.portSpeedDuplex[port-1] != swdrv_port_info.speed_duplex_oper)
        {

            SWDRV_OM_SetPortInfoSpeedDuplexOper(port, PortStats.portSpeedDuplex[port-1]);
            SWDRV_Notify_PortSpeedDuplex(stack_id, port, PortStats.portSpeedDuplex[port-1]);
          }
        }
         if(PortStats.portFlowCtrl[port-1] != swdrv_port_info.flow_control_oper)
        {
            swdrv_port_info.flow_control_oper = (UI8_T )PortStats.portFlowCtrl[port-1];
        #if 0
            switch(flow_control)
            {
                case VAL_portFlowCtrlCfg_dot3xFlowControl:
                    f_c = VAL_portFlowCtrlStatus_dot3xFlowControl;
                    break;
                case VAL_portFlowCtrlCfg_backPressure:
                    f_c = VAL_portFlowCtrlStatus_backPressure;
                    break;
                case VAL_portFlowCtrlCfg_disabled:
                    f_c = VAL_portFlowCtrlStatus_none;
                    break;
                default:
                    f_c = 0;
            }
            if (f_c == 0)
                return;
        #endif
            SWDRV_OM_SetPortInfoFlowControlOper(port, PortStats.portFlowCtrl[port-1]);
            SWDRV_Notify_PortFlowCtrl(stack_id, port, PortStats.portFlowCtrl[port-1]);
        }
       }
}

static void SWDRV_MONITOR_AllPortLinkStatus(UI32_T start_port,UI32_T end_port )
{
    UI32_T status;
    UI32_T stack_id;
    SWDRV_Port_Info_T swdrv_port_info;
    int port;
    DEV_SWDRV_PortTableStats_T PortStats;
    SWDRV_OM_GetSystemInfoStackId(&stack_id);

     if(DEV_SWDRV_PMGR_GetAllPortLinkStatus(stack_id, start_port,end_port, &PortStats)==FALSE)
     {
         printf("%s(fail):line = %d\n", __FUNCTION__,__LINE__);

     }


    for(port=start_port;port<=end_port; port++)
    {
    memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

     if(PortStats.portLinkStatus[port-1] != swdrv_port_info.link_status)
        {

            SWDRV_LocalUpdatePortLinkStatus(port, (UI8_T)PortStats.portLinkStatus[port-1]);


            if(PortStats.portLinkStatus[port-1])
            {
                SWDRV_Notify_PortLinkUp(stack_id, port);
            }
            else
            {
               SWDRV_Notify_PortLinkDown(stack_id, port);
            }
        }

    }
} /* End of SWDRV_MONITOR_LinkStatus() */

static void SWDRV_MONITOR_AllPortOperSpeedDuplex(UI32_T start_port,UI32_T end_port)
{
    UI32_T speed_duplex;
    BOOL_T retval;
    UI32_T  stack_id;
    SWDRV_Port_Info_T swdrv_port_info;
    int port;

     DEV_SWDRV_PortTableStats_T PortStats;


    SWDRV_OM_GetSystemInfoStackId(&stack_id);


    if(DEV_SWDRV_PMGR_GetAllPortOperSpeedDuplex(stack_id, start_port,end_port, &PortStats)==FALSE)
        {
        //    printf("%s(fail):line = %d\n", __FUNCTION__,__LINE__);

        }



      for(port=start_port;port<=end_port; port++)
      {
      memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
      SWDRV_OM_GetPortInfo(port, &swdrv_port_info);


#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            if (VAL_portSpeedDpxStatus_fullDuplex100 != (UI32_T )swdrv_port_info.speed_duplex_oper)
            {
                SWDRV_OM_SetPortInfoSpeedDuplexOper(port, VAL_portSpeedDpxStatus_fullDuplex100);
                SWDRV_Notify_PortSpeedDuplex(stack_id, port, VAL_portSpeedDpxStatus_fullDuplex100);
            }
            return;
        }
#endif
        if(PortStats.portSpeedDuplex[port-1] != 0)
        {
        if(PortStats.portSpeedDuplex[port-1] != swdrv_port_info.speed_duplex_oper)
        {
            SWDRV_OM_SetPortInfoSpeedDuplexOper(port, PortStats.portSpeedDuplex[port-1]);
            SWDRV_Notify_PortSpeedDuplex(stack_id, port, PortStats.portSpeedDuplex[port-1]);
          }
        }

   }
} /* End of SWDRV_MONITOR_OperSpeedDuplex() */

static void SWDRV_MONITOR_AllPortOperFlowCtrl(UI32_T start_port,UI32_T end_port)
{
    UI32_T flow_control;
    BOOL_T retval;
    UI32_T stack_id;

    SWDRV_Port_Info_T swdrv_port_info;

    int port;

    DEV_SWDRV_PortTableStats_T PortStats;

#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
            return;
#endif

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

     if(DEV_SWDRV_PMGR_GetAllPortFlowCtrl(stack_id, start_port,end_port, &PortStats)==FALSE)
        {
            printf("%s(fail):line = %d\n", __FUNCTION__,__LINE__);

        }



      for(port=start_port;port<=end_port; port++)
     {

    memset(&swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

        if(PortStats.portFlowCtrl[port-1] != swdrv_port_info.flow_control_oper)
        {
            swdrv_port_info.flow_control_oper = (UI8_T )PortStats.portFlowCtrl[port-1];
#if 0
            switch(flow_control)
            {
                case VAL_portFlowCtrlCfg_dot3xFlowControl:
                    f_c = VAL_portFlowCtrlStatus_dot3xFlowControl;
                    break;
                case VAL_portFlowCtrlCfg_backPressure:
                    f_c = VAL_portFlowCtrlStatus_backPressure;
                    break;
                case VAL_portFlowCtrlCfg_disabled:
                    f_c = VAL_portFlowCtrlStatus_none;
                    break;
                default:
                    f_c = 0;
            }
            if (f_c == 0)
                return;
#endif
            SWDRV_OM_SetPortInfoFlowControlOper(port, PortStats.portFlowCtrl[port-1]);
            SWDRV_Notify_PortFlowCtrl(stack_id, port, PortStats.portFlowCtrl[port-1]);
        }

    }
} /* End of SWDRV_MONITOR_OperFlowCtrl() */

#endif

#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_PDPortPowerStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Monitor the power good status of the specified port. If the
 *           specified port does not support PD, the function will return
 *           directly. If the given port support PD, it will power down PHY
 *           when PowerGood status is FALSE and power up PHY when PowerGood
 *           status is TRUE.
 *
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_PDPortPowerStatus(UI32_T port)
{
    static UI8_T  prev_power_good_status[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]={0xFF};
    UI32_T my_unit_id, my_board_id;
    UI32_T dev_addr, reg_addr, reg_mask, pg_val;
    BOOL_T local_admin_status, local_admin_status_changed;
    BOOL_T current_pg_status, prev_pg_status, pg_status_changed;
    UI8_T  reg_val;

    if(STKTPLG_OM_GetMyUnitID(&my_unit_id)==FALSE)
    {
        printf("%s(%d): Failed to get my unit id.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    if(STKTPLG_OM_GetUnitBoardID(my_unit_id, &my_board_id)==FALSE)
    {
        printf("%s(%d): Failed to get my board id.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    /* check whether the port supports PD
     */
    if(STKTPLG_OM_IsPoEPDPort(my_unit_id, port)==FALSE)
    {
        /* for port not support PD, just return */
        return;
    }

    if(SWDRV_OM_GetPSECheckStatus()==FALSE)
    {
        /* always treat power good status as TRUE if
         * pse check status is disabled
         */
        current_pg_status = TRUE;
    }
    else
    {
        /* check the Power status of the port
         */
        if(SYS_HWCFG_GetPDPortPowerStatusRegInfo(my_board_id, port, &dev_addr, &reg_addr, &reg_mask, &pg_val)==FALSE)
        {
            printf("%s(%d): Failed to get Power Good Status Register info.\r\n", __FUNCTION__, __LINE__);
            return;
        }

        if(DEV_SWDRV_TwsiDataRead(dev_addr, 0, 1, reg_addr, 0, 1, &reg_val)==FALSE)
        {
            printf("%s(%d): I2C read error.\r\n", __FUNCTION__, __LINE__);
            return;
        }

        current_pg_status = ((reg_val & reg_mask)==pg_val)?TRUE:FALSE;
    }

    if((prev_power_good_status[(port-1)/8] & (0x80>>((port-1)%8))))
        prev_pg_status=TRUE;
    else
        prev_pg_status=FALSE;

    SWDRV_OM_GetLocalPortAdminStatus(port, &local_admin_status, &local_admin_status_changed);

    if((current_pg_status == TRUE) && (prev_pg_status == FALSE))
    {
        /* PD power status change to good -> set PHY power up if
         *                                   current port admin status is TRUE
         */
        if(local_admin_status)
        {
            DEV_SWDRV_EnablePortAdmin(my_unit_id, port);
        }

        pg_status_changed=TRUE;
        prev_power_good_status[(port-1)/8] |= (0x80>>((port-1)%8));
    }
    else if((current_pg_status==FALSE) && (prev_pg_status==TRUE))
    {
        /* PD power status change to no good -> set PHY power down
         */
        DEV_SWDRV_DisablePortAdmin(my_unit_id, port);
        pg_status_changed=TRUE;
        prev_power_good_status[(port-1)/8] ^= (0x80>>((port-1)%8));
    }
    else
        pg_status_changed=FALSE;

    /* need to apply the new setting of local_admin_status
     * to set PHY admin status properly if power good status does
     * not change in this iteration.
     */
    if(pg_status_changed==FALSE && local_admin_status_changed==TRUE)
    {
        /* Always shutdown PHY when power good status is FALSE
         * Thus only need to check when power good status is TRUE
         */
        if(current_pg_status==TRUE)
        {
            if(local_admin_status==TRUE)
            {
                DEV_SWDRV_EnablePortAdmin(my_unit_id, port);
            }
            else
            {
                DEV_SWDRV_DisablePortAdmin(my_unit_id, port);
            }
        }
    }
}
#endif

#if (SWDRV_FORCE_DEFAULT_OPER_SPEED_DUPLEX_WHEN_LINK_DOWN == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_GetDefaultPortSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port link status is changed
 *           from up to down
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRV_MONITOR_GetDefaultPortSpeedDuplex(UI32_T unit, UI32_T port, UI32_T *default_speed_duplex_p)
{
    SWDRV_Port_Info_T swdrv_port_info;
    UI32_T default_speed_duplex;

    SWDRV_OM_GetPortInfo(port, &swdrv_port_info);

    switch (swdrv_port_info.port_type)
    {
        case VAL_portType_hundredBaseTX:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex100;
            break;
        case VAL_portType_hundredBaseFX:
        case VAL_portType_hundredBaseFxScSingleMode:
        case VAL_portType_hundredBaseFxScMultiMode:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex100;
            break;
        case VAL_portType_thousandBaseT:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex1000;
            break;
        case VAL_portType_tenG:
        case VAL_portType_tenGBaseT:
        case VAL_portType_tenGBaseXFP:
        case VAL_portType_tenGBaseSFP:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex10g;
            break;
        case VAL_portType_twentyFiveGBaseSFP:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex25g;
            break;
        case VAL_portType_fortyGBaseQSFP:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex40g;
            break;
        case VAL_portType_hundredGBaseQSFP:
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex100g;
            break;
        default: /* GE Fiber */
            default_speed_duplex = VAL_portSpeedDpxStatus_fullDuplex1000;
    }

    *default_speed_duplex_p = default_speed_duplex;

    return TRUE;
}
#endif /* (SWDRV_FORCE_DEFAULT_OPER_SPEED_DUPLEX_WHEN_LINK_DOWN == TRUE) */

#if (SYS_CPNT_HW_LINKSCAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_MONITOR_HardwareLinkStatusChanged_Callback
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function is callback function to handle link status
 *              changed event from HW linkscan
 * INPUT    :   unit
 *              port
 *              link_status
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
static void SWDRV_MONITOR_HardwareLinkStatusChanged_Callback(UI32_T unit, UI32_T port, UI32_T link_status)
{
    SWDRV_MONITOR_LinkStatus(port);
}
#endif /* (SYS_CPNT_HW_LINKSCAN == TRUE) */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
static void SWDRV_MONITOR_UpdatePortSfpPresentStatus(UI32_T port, UI32_T sfp_index, BOOL_T is_present)
{
    UI8_T array_index;
    UI8_T bit_index;
    UI32_T stack_id;
    SWDRV_TYPE_SfpPresentStatus_T swdrv_port_sfp_present_status;

    array_index = SWDRV_LPORT_INDEX(port);
    bit_index = SWDRV_LPORT_BIT_IN_UI8_T(port);
    SWDRV_OM_GetSystemInfoStackId(&stack_id);

    /* Need to update om first, fot it's used latter in
     * SWDRV_SfpTransceiverInserted() and 
     * SWDRV_SfpTransceiverRemoved().
     */
    SWDRV_OM_SetPortSfpPresent(sfp_index, is_present);

    SWDRV_OM_GetPortSfpPresentStatusBitmaps(stack_id, &swdrv_port_sfp_present_status);
    if(is_present)
    {
        /* Turn ON the specific bit.
         */
        swdrv_port_sfp_present_status.sfp_present_st_bitmap[array_index] |= bit_index;
    }
    else
    {
        /* Turn OFF the specific bit.
         */
        swdrv_port_sfp_present_status.sfp_present_st_bitmap[array_index] &= (~bit_index);
    }
    SWDRV_OM_SetPortSfpPresentStatusBitmaps(stack_id, &swdrv_port_sfp_present_status, SWDRV_OM_F_REALTIME);

    if(SWDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* trigger Master to notify upper layer
         */
        UI32_T thread_id;

        SWDRV_OM_GetSfpThreadId(&thread_id);
        SYSFUN_SendEvent(thread_id, SWDRV_EVENT_UPDATE_PORT_SFP_PRESENT_STATUS);
    }
}/* End of SWDRV_MONITOR_UpdatePortSfpPresentStatus */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SfpInfoWorkaround
 * -------------------------------------------------------------------------
 * FUNCTION: EEPROM info workaround for specified project or transceiver
 * INPUT   : info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH]
 * OUTPUT  : info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH]
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_MONITOR_SfpInfoWorkaround(UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH])
{
#if (SYS_CPNT_SFP_INFO_FOR_BROCADE == TRUE)
    /* for Brocade
     *  Vendor name [20:35]    "FOUNDRY NETWORKS" -> "BROCADE"
     */
    if (strncmp(&info_ar[20], "FOUNDRY NETWORKS", sizeof("FOUNDRY NETWORKS")-1) == 0)
    {
        strncpy(&info_ar[20], "BROCADE", 16);
    }
#endif
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SfpInfoReadAndParse
 * -------------------------------------------------------------------------
 * FUNCTION: Read and parse the sfp EEPROM info.
 * INPUT   : sfp_index
 * OUTPUT  : sfp_info_p -- sfp info
 * RETURN  : TRUE/FALSE
 *           TRUE: Can get sfp info
 *           FALSE: Can't get sfp info
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SWDRV_MONITOR_SfpInfoReadAndParse(UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
    UI8_T  qsfp_page = 0/* write dflt page 0 */;
    UI8_T  sfp_type = sfp_info_p->identifier;
    static UI8_T  tx_disable;
    static UI8_T  info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH];
    BOOL_T ret=TRUE;
    UI8_T  sum_cc_base = 0, sum_cc_base_real = 0;
    UI8_T  sum_cc_ext = 0, sum_cc_ext_real = 0;

    /* Read A0h */
    switch (sfp_type)
    {
        case SWDRV_TYPE_GBIC_ID_SFP:
            sfp_info_p->support_tx_disable = TRUE;
            if ((FALSE == SYSDRV_GetSfpInfo(sfp_index, 0, 128, info_ar)) ||
                (FALSE == SYSDRV_GetSfpInfo(sfp_index, 128, 128, &(info_ar[128]))))
            {
                if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
                    BACKDOOR_MGR_Printf("%s(%d): Failed to read SFP %lu info from I2C.\r\n", __FUNCTION__, __LINE__, sfp_index);
                sfp_info_p->is_invalid |= SWDRV_TYPE_GBIC_INVALID_INFO_READ_ERROR;
                return FALSE;
            }
            break;

        case SWDRV_TYPE_GBIC_ID_XFP:
            sfp_info_p->support_ddm = TRUE;
            if (FALSE == SYSDRV_GetSfpInfo(sfp_index, 128, 96, info_ar))
            {
                if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
                    BACKDOOR_MGR_Printf("%s(%d): Failed to read XFP %lu info from I2C.\r\n", __FUNCTION__, __LINE__, sfp_index);
                sfp_info_p->is_invalid |= SWDRV_TYPE_GBIC_INVALID_INFO_READ_ERROR;
				return FALSE;
            }
            break;

        case SWDRV_TYPE_GBIC_ID_QSFP:
        case SWDRV_TYPE_GBIC_ID_QSFP_PLUS:
        case SWDRV_TYPE_GBIC_ID_QSFP28:
            sfp_info_p->support_ddm = TRUE;
            if ((FALSE == SYSDRV_SetSfpInfo(sfp_index, 127, 1, &qsfp_page)) ||
                (FALSE == SYSDRV_GetSfpInfo(sfp_index, 128, 96, info_ar)))
            {
                if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
                    BACKDOOR_MGR_Printf("%s(%d): Failed to read QSFP(+) %lu info from I2C.\r\n", __FUNCTION__, __LINE__, sfp_index);
                sfp_info_p->is_invalid |= SWDRV_TYPE_GBIC_INVALID_INFO_READ_ERROR;
                return FALSE;
            }
            break;

       default: /*unknow identifier*/
	        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
	        {
                BACKDOOR_MGR_Printf("%s(%d): Unknow Identifier %u.\r\n", __FUNCTION__, __LINE__, sfp_type);
            }
	        sfp_info_p->is_invalid |= SWDRV_TYPE_GBIC_INVALID_INFO_READ_ERROR;
            return TRUE;
    }

    /* Verify Checksum: CC_BASE, CC_EXT */
    if(FALSE == SWDRV_LIB_CalcSFPChecksum(SWDRV_TYPE_SFP_CHECKSUM_CC_BASE, info_ar, &sum_cc_base, &sum_cc_base_real))
        BACKDOOR_MGR_Printf("%s(%d): Failed to calculate CC_BASE of SFP %lu.\r\n", __FUNCTION__, __LINE__, sfp_index);
    if(FALSE == SWDRV_LIB_CalcSFPChecksum(SWDRV_TYPE_SFP_CHECKSUM_CC_EXT, info_ar, &sum_cc_ext, &sum_cc_ext_real))
        BACKDOOR_MGR_Printf("%s(%d): Failed to calculate CC_EXT of SFP %lu.\r\n", __FUNCTION__, __LINE__, sfp_index);

    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
    {
        BACKDOOR_MGR_Printf("CC_BASE:0x%x, calculated:0x%x\r\n", sum_cc_base, sum_cc_base_real);
        BACKDOOR_MGR_Printf("CC_EXT :0x%x, calculated:0x%x\r\n", sum_cc_ext, sum_cc_ext_real);
    }

    if(sum_cc_base != sum_cc_base_real)
    {
        sfp_info_p->is_invalid |= SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_BASE_ERROR;
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        {
            BACKDOOR_MGR_Printf("%s %d sfp_type:0x%x 0x%x, bitrate:0x%x is_invalid_flag:0x%x\r\n", __FUNCTION__, sfp_index, info_ar[0], sfp_type, info_ar[12], sfp_info_p->is_invalid);
            BACKDOOR_MGR_DumpHex(NULL, 256, (void *)&info_ar);
        }
    }

    if(sum_cc_ext != sum_cc_ext_real)
    {
        sfp_info_p->is_invalid |= SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_EXT_ERROR;
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        {
            BACKDOOR_MGR_Printf("%s %d sfp_type:0x%x 0x%x, bitrate:0x%x is_invalid_flag:0x%x\r\n", __FUNCTION__, sfp_index, info_ar[0], sfp_type, info_ar[12], sfp_info_p->is_invalid);
            BACKDOOR_MGR_DumpHex(NULL, 256, (void *)&info_ar);
        }
    }

    /* Parsing:
     * Return FALSE when following occurs:
     *  SWDRV_TYPE_GBIC_INVALID_INFO_READ_ERROR
     *  SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_BASE_ERROR
     * Note:
     *  when SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_EXT_ERROR, we will not
     *  return FALSE in order to config speed to chip.
     */
    if (sfp_info_p->is_invalid & (~SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_EXT_ERROR))
    {
        return TRUE;
    }
    else
    {
        SWDRV_MONITOR_SfpInfoParse(sfp_type, info_ar, sfp_info_p);

        /* Revision compliance
         */
        if (sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
        {
            sfp_info_p->rev_comp = info_ar[94];
        }
        else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
                 sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
                 sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
        {
            if (!SYSDRV_GetSfpInfo(sfp_index, 0, 1, &sfp_info_p->rev_comp))
            {
                sfp_info_p->rev_comp = 0;
            }
        }
        else
        {
            sfp_info_p->rev_comp = 0;
        }
    }

    return TRUE;
}/* End of SWDRV_MONITOR_SfpInfoReadAndParse */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SfpInfoParse
 * -------------------------------------------------------------------------
 * FUNCTION: Parse the sfp EEPROM info.
 * INPUT   : sfp_type
 *           info_ar
 * OUTPUT  : sfp_info_p -- sfp info
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SWDRV_MONITOR_SfpInfoParse(UI8_T sfp_type, UI8_T *info_ar, SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
    if(info_ar == NULL || sfp_info_p == NULL)
    {
        BACKDOOR_MGR_Printf("%s-%d NULL pointer\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    SWDRV_MONITOR_SfpInfoWorkaround(info_ar);

    if(sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
        sfp_info_p->aux_monitoring = info_ar[94];/*222*/

    /* Connector [2]
     */
    sfp_info_p->connector = info_ar[2];

    /* Transceiver [3:10]
     */
    memcpy((UI8_T *)&(sfp_info_p->transceiver), &info_ar[3], SWDRV_TYPE_GBIC_TRANSCEIVER_FIELD_LENGTH);

    /* Bit rate [12], in unit of 100 MBd (Baud)
     */
    sfp_info_p->bitrate = info_ar[12];

    /* Link Length Support in km [14]
     */
    sfp_info_p->link_length_support_km = info_ar[14];

    /* Link Length Support in 100m [15]
     */
    sfp_info_p->link_length_support_100m = info_ar[15];

    /* Vendor name [20:35]
     */
    memcpy(sfp_info_p->vendor_name, &info_ar[20], SWDRV_TYPE_GBIC_VENDOR_NAME_FIELD_LENGTH);
    sfp_info_p->vendor_name[SWDRV_TYPE_GBIC_VENDOR_NAME_FIELD_LENGTH] = 0;

    /* Transceiver [3:10]
     */
    if (sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
    {
        sfp_info_p->transceiver_1 = info_ar[36];
    }
    else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_info_p->transceiver_1 = info_ar[64];
    }
    else
    {
        sfp_info_p->transceiver_1 = 0;
    }

    /* Vendor OUI [37:39]
     */
    memcpy(sfp_info_p->vendor_oui, &info_ar[37], SWDRV_TYPE_GBIC_VENDOR_OUI_FIELD_LENGTH);

    /* Vendor PN [40:55]
     */
    memcpy(sfp_info_p->vendor_pn, &info_ar[40], SWDRV_TYPE_GBIC_VENDOR_PN_FIELD_LENGTH);
    sfp_info_p->vendor_pn[SWDRV_TYPE_GBIC_VENDOR_PN_FIELD_LENGTH] = 0;

    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
    {
        /* Vendor rev [56:59]
         */
        memcpy(sfp_info_p->vendor_rev, &info_ar[56], SWDRV_TYPE_GBIC_VENDOR_REV_FIELD_LENGTH);
        sfp_info_p->vendor_rev[SWDRV_TYPE_GBIC_VENDOR_REV_FIELD_LENGTH] = 0;
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_XFP ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        /* Vendor rev [56:57]
         */
        memcpy(sfp_info_p->vendor_rev, &info_ar[56], 2);
        sfp_info_p->vendor_rev[2] = 0;
    }

    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
    {
        /* Laser Wavelength [60:61]
         */
        if ((info_ar[8] & 0x0c) == 0)
        {
            sfp_info_p->wavelength = (info_ar[60] << 8) | info_ar[61];
        }
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_XFP || sfp_type == SWDRV_TYPE_GBIC_ID_QSFP || sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS)
    {
        /* Laser Wavelength [58:59]
         */
        sfp_info_p->wavelength = (info_ar[58] << 8) | info_ar[59];
    }

    /* Bit rate [66], in unit of 250 MBd (Baud)
     */
    if (sfp_info_p->bitrate == 0xff)
    {
        if (sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
        {
            sfp_info_p->bitrate_in_250mbd = info_ar[66];
        }
        else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
                 sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
                 sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
        {
            sfp_info_p->bitrate_in_250mbd = info_ar[94];
        }
        else
        {
            sfp_info_p->bitrate_in_250mbd = 0;
        }
    }

    /* Vendor SN [68:83]
     */
    memcpy(sfp_info_p->vendor_sn, &info_ar[68], SWDRV_TYPE_GBIC_VENDOR_SN_FIELD_LENGTH);
    sfp_info_p->vendor_sn[SWDRV_TYPE_GBIC_VENDOR_SN_FIELD_LENGTH] = 0;

    /* Date code [84:91]
     */
    memcpy(sfp_info_p->date_code, &info_ar[84], SWDRV_TYPE_GBIC_DATE_CODE_FIELD_LENGTH);

    /* Tx-disable [195]
     */
    if (sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
    {
        sfp_info_p->support_tx_disable = TRUE;
    }
    else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_info_p->support_tx_disable = info_ar[67] >> 4;  /*get bit 4*/
    }
    else
    {
        sfp_info_p->support_tx_disable = FALSE;
    }

#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    /* query sfp eeprom to know whether RX_LOS is implemented
     * Reference "SFF Committee INF-8074i Specification for SFP
     * Transceiver Rev 1.0"
     * Excerpted from Appendix B. Table 3.6 Option Values
     * Offset 65, bit 1: Loss of signal implemented, signal as defined in
     * Table 1 (signal high as LOS, signal low as normal).
     * Offset 65, bit 2: Loss of signal implemented, signal inverted from
     * definition in Table 1.
     */
    sfp_info_p->is_rx_los_implemented = (info_ar[65] & BIT_1) ?TRUE:FALSE;
    sfp_info_p->is_rx_los_inverted = (info_ar[65] & BIT_2) ?TRUE:FALSE;
#endif /* end of #if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE) */

    if((SWDRV_TYPE_GBIC_DIAG_MONITOR_TYPE_DDM_IMPLEMENTED & info_ar[92]) ||
        sfp_type == SWDRV_TYPE_GBIC_ID_XFP || sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
        sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
        sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_info_p->support_ddm = TRUE;
        if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
        {
            if (SWDRV_TYPE_GBIC_DIAG_MONITOR_TYPE_EXTERNELLY_CALIBRATE & info_ar[92])
                sfp_info_p->ext_calibrate_needed = TRUE;
            else
                sfp_info_p->ext_calibrate_needed = FALSE;
        }
        else
            sfp_info_p->ext_calibrate_needed = FALSE;
    }

    return TRUE;
}/* End of SWDRV_MONITOR_SfpInfoParse */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM float.
 * INPUT   : single_float
 * OUTPUT  : None.
 * RETURN  : result(float)
 * NOTE    : 1.http://www.psc.edu/general/software/packages/ieee/ieee.php
 * -------------------------------------------------------------------------
 */
static float SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(UI32_T single_float)
{
    float  result = 0.0, tmp = 0.0;
    UI32_T i;
    UI32_T fraction;
    UI8_T  exponent;

    exponent = (single_float >> 23) & 0xff;
    fraction = (single_float << 9) & 0xfffffe00;

    if (exponent > 0 && exponent < 255)
    {
        for (i = 31; i >= 9; i--)
            if (fraction & (1 << i))
                tmp += pow((float)0.5, (float)(32-i));
        result = pow((float)2, (float)(exponent - 127)) * (1 + tmp);
    }
    else if (exponent == 0)
    {
        if (fraction == 0)
             result = 0;
        else
        {
            for (i = 31; i >= 9; i--)
                if (fraction & (1 << i))
                    tmp += pow((float)0.5, (float)32-i);
            result = pow((float)2, (float)-126) * tmp;
        }
    }/*
    else if (exponent == 255 && fraction == 0)
    {
        result = INFINITY; //(or if (single_float & (1 << 31)), it will be -INFINITY)
    }
    else //(exponent == 255, but franction != 0)
    {
        result = Nan; //(Nan: Not a number even if (single_float & (1 << 31)))
    }
    */

    if (single_float & (1 << 31))
        result = -result;

    return result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertSlope
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM slope
 * INPUT   : slope
 * OUTPUT  : None.
 * RETURN  : result(double)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static double SWDRV_MONITOR_DdmConvertSlope(UI16_T slope)
{
    double result = 0.0, temp = 0.00390625; /* slope unit: 1/256 */
    UI32_T i;

    for (i = 0; i < 16; i++)
    {
        if (slope & (1 << i))
            result += temp;
        temp *= 2;
    }
    if (slope & (1 << 15))
        result = -result;

    return result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertOffset
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM offset.
 * INPUT   : offset
 * OUTPUT  : None.
 * RETURN  : result(double)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static double SWDRV_MONITOR_DdmConvertOffset(UI16_T offset)
{
    double result = 0.0;

    result = (double)((I16_T)offset);

    return result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertTemperature
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM temperature
 * INPUT   : temperature
 *           slope
 *           offset
 * OUTPUT  : None.
 * RETURN  : result(float)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static float SWDRV_MONITOR_DdmConvertTemperature(UI16_T temperature, double slope, double offset)
{
    double result = 0.0, temp = 0.00390625; /* temperature unit: 1/256 degree */
    UI32_T i;
    UI16_T absolute=temperature;
    BOOL_T neg=FALSE;

    if (temperature & (1 << 15))
    {
        neg = TRUE;
        absolute = ~temperature + 1;
    }

    for (i = 0; i < 15; i++)
    {
        if (absolute & (1 << i))
            result += temp;
        temp *= 2;
    }
    if (neg)
        result = -result;

    result = result * slope + offset;

    return (float)result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertVoltage
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM voltage
 * INPUT   : voltage
 *           slope
 *           offset
 * OUTPUT  : None.
 * RETURN  : result(float)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static float SWDRV_MONITOR_DdmConvertVoltage(UI16_T voltage, double slope, double offset)
{
    double result = 0.0; /* voltage unit: 100 uVolt */

    result = ((double)voltage * slope + offset) / 10000;

    return (float)result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertCurrent
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM current
 * INPUT   : current
 *           slope
 *           offset
 * OUTPUT  : None.
 * RETURN  : result(float)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static float SWDRV_MONITOR_DdmConvertCurrent(UI16_T current, double slope, double offset)
{
    double result = 0.0; /* current unit: 2 uA */

    result = ((double)current * slope + offset) / 500;

    return (float)result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertTxPower
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM tx power
 * INPUT   : tx_power
 *           slope
 *           offset
 * OUTPUT  : None.
 * RETURN  : result(double)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static double SWDRV_MONITOR_DdmConvertTxPower(UI16_T tx_power, double slope, double offset)
{
    double result = 0.0; /* power unit: 0.1 uW */

    result = ((double)tx_power * slope + offset) / 10000000;

    return result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmConvertRxPower
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM rx power
 * INPUT   : rx_power
 *           slope
 *           offset
 * OUTPUT  : None.
 * RETURN  : result(double)
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static double SWDRV_MONITOR_DdmConvertRxPower(UI16_T rx_power, float rx_p_4, float rx_p_3, float rx_p_2, float rx_p_1, float rx_p_0)
{
    double result = 0.0; /* power unit: 0.1 uW */

    result = ((double)rx_p_4 * (double)pow((float)rx_power, 4.0f) +
              (double)rx_p_3 * (double)pow((float)rx_power, 3.0f) +
              (double)rx_p_2 * (double)pow((float)rx_power, 2.0f) +
              (double)rx_p_1 * (double)rx_power +
              (double)rx_p_0) / 10000000;

    return result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmUnitWatt2Dbm
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM power of unit watt to dBm
 * INPUT   : val
 * OUTPUT  : float
 * RETURN  : None.
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static float SWDRV_MONITOR_DdmUnitWatt2Dbm(double val)
{
    if ((val >= 0.0000001) && (val <= 0.0065535))
        return (float)((double)log10(val) * 10 + 30);
    else
        return -40;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_DdmGetXFPVoltage
 * -------------------------------------------------------------------------
 * FUNCTION: Get voltage from AUX 1/2 depends on address 222 for XFP
 * INPUT   : info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH] -- raw data read from
 * GBIC EEPROM
 *           aux_monitoring
 * OUTPUT  : None.
 * RETURN  : voltage in UI16_T
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static UI16_T SWDRV_MONITOR_DdmGetXFPVoltage(UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH], UI8_T aux_monitoring)
{
    UI32_T aux_input_offset[16];
    UI32_T voltage_offset = 0;
    UI8_T  aux_type = 0;

    aux_type = aux_monitoring;

    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        BACKDOOR_MGR_Printf("%s(%d): aux_type:0x%x\r\n", __FUNCTION__, __LINE__, aux_type);

    memset(aux_input_offset, 0, sizeof(aux_input_offset));
    aux_input_offset[((aux_type >> 4) & 0xf)] = 106;/* AUX 1 MSB */
    aux_input_offset[(aux_type & 0xf)] = 108;       /* AUX 2 MSB */
    voltage_offset = aux_input_offset[SWDRV_TYPE_AUX_INPUT_TYPE_SUPPLY_VOLTAGE_1];

    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        BACKDOOR_MGR_Printf("%s(%d): voltage_offset:0x%lx\r\n", __FUNCTION__, __LINE__, voltage_offset);

    /* Voltage
     */
    if(aux_input_offset[SWDRV_TYPE_AUX_INPUT_TYPE_SUPPLY_VOLTAGE_1])
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): val:0x%x\r\n", __FUNCTION__, __LINE__, *((UI16_T*)&info_ar[aux_input_offset[SWDRV_TYPE_AUX_INPUT_TYPE_SUPPLY_VOLTAGE_1]]));
        return *((UI16_T*)&info_ar[aux_input_offset[SWDRV_TYPE_AUX_INPUT_TYPE_SUPPLY_VOLTAGE_1]]);
    }
    else
        return 0;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_ConvertSfpDdmInfoRaw
 * -------------------------------------------------------------------------
 * FUNCTION: Convert sfp DDM EEPROM information to readable.
 * INPUT   : sfp_index -- sfp index.
 *           sfp_type -- can be SFP/XFP/QSFP/QSFP_PLUS.
 *           info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH] -- ddm all info.
 *           ext_calibrate_needed    -- need external calibrate.
 * OUTPUT  : sfp_ddm_info           -- ddm readable info
 * RETUEN  : None.
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static void SWDRV_MONITOR_ConvertSfpDdmInfoRaw(UI32_T sfp_index, UI8_T sfp_type, UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH], UI8_T ext_calibrate_needed, UI8_T aux_monitoring, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    double  i_slope = 1.0, p_slope = 1.0, t_slope = 1.0, v_slope = 1.0;
    double  i_offset = 0.0, p_offset = 0.0, t_offset = 0.0, v_offset = 0.0;
    double  tmp_power;
    float   rx_p_4 = 0.0, rx_p_3 = 0.0, rx_p_2 = 0.0, rx_p_1 = 1.0, rx_p_0 = 0.0;
    SWDRV_TYPE_SfpDdmInfoRaw_T       *sfp_ddm_raw_p = (SWDRV_TYPE_SfpDdmInfoRaw_T *)info_ar;
    SWDRV_TYPE_QsfpDdmInfoRaw_T       *qsfp_ddm_raw_p = (SWDRV_TYPE_QsfpDdmInfoRaw_T *)info_ar;

    /* 1-1.Convert the thresholds to human readable data */
    /* Convert the calibrate argument if the external calibrate bit is set
     * where XFP, QSFP, QSFP+ only support internal calibration.
     */
    if(ext_calibrate_needed && sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
    {
        rx_p_4 = SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(L_STDLIB_Ntoh32(*((UI32_T*)&(sfp_ddm_raw_p->rx_pwr_4[0]))));
        rx_p_3 = SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(L_STDLIB_Ntoh32(*((UI32_T*)&(sfp_ddm_raw_p->rx_pwr_3[0]))));
        rx_p_2 = SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(L_STDLIB_Ntoh32(*((UI32_T*)&(sfp_ddm_raw_p->rx_pwr_2[0]))));
        rx_p_1 = SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(L_STDLIB_Ntoh32(*((UI32_T*)&(sfp_ddm_raw_p->rx_pwr_1[0]))));
        rx_p_0 = SWDRV_MONITOR_DdmConvertIeee754SingleFloat2Float(L_STDLIB_Ntoh32(*((UI32_T*)&(sfp_ddm_raw_p->rx_pwr_0[0]))));
        i_slope = SWDRV_MONITOR_DdmConvertSlope(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_i_slope[0]))));
        i_offset = SWDRV_MONITOR_DdmConvertOffset(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_i_offset[0]))));
        p_slope = SWDRV_MONITOR_DdmConvertSlope(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_pwr_slope[0]))));
        p_offset = SWDRV_MONITOR_DdmConvertOffset(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_pwr_offset[0]))));
        t_slope = SWDRV_MONITOR_DdmConvertSlope(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->t_slope[0]))));
        t_offset = SWDRV_MONITOR_DdmConvertOffset(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->t_offset[0]))));
        v_slope = SWDRV_MONITOR_DdmConvertSlope(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->v_slope[0]))));
        v_offset = SWDRV_MONITOR_DdmConvertOffset(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->v_offset[0]))));
    }/* End of if (ext_calibrate_needed) */

    /* 1-2.Read threshold from transceiver directly for use in auto mode*/
    /* temperature */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        sfp_ddm_info_p->threshold.temp_high_alarm = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->temp_high_alarm[0]))), t_slope, t_offset);
        sfp_ddm_info_p->threshold.temp_low_alarm = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->temp_low_alarm[0]))), t_slope, t_offset);
        sfp_ddm_info_p->threshold.temp_high_warning = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->temp_high_warning[0]))), t_slope, t_offset);
        sfp_ddm_info_p->threshold.temp_low_warning = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->temp_low_warning[0]))), t_slope, t_offset);
    }
    else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_ddm_info_p->threshold.temp_high_alarm = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->temp_high_alarm[0]))), t_slope, t_offset);
        sfp_ddm_info_p->threshold.temp_low_alarm = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->temp_low_alarm[0]))), t_slope, t_offset);
        sfp_ddm_info_p->threshold.temp_high_warning = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->temp_high_warning[0]))), t_slope, t_offset);
        sfp_ddm_info_p->threshold.temp_low_warning = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->temp_low_warning[0]))), t_slope, t_offset);
    }


    /* voltage */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        sfp_ddm_info_p->threshold.voltage_high_alarm = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->voltage_high_alarm[0]))), v_slope, v_offset);
        sfp_ddm_info_p->threshold.voltage_low_alarm = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->voltage_low_alarm[0]))), v_slope, v_offset);
        sfp_ddm_info_p->threshold.voltage_high_warning = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->voltage_high_warning[0]))), v_slope, v_offset);
        sfp_ddm_info_p->threshold.voltage_low_warning = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->voltage_low_warning[0]))), v_slope, v_offset);
    }
    else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_ddm_info_p->threshold.voltage_high_alarm = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->voltage_high_alarm[0]))), v_slope, v_offset);
        sfp_ddm_info_p->threshold.voltage_low_alarm = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->voltage_low_alarm[0]))), v_slope, v_offset);
        sfp_ddm_info_p->threshold.voltage_high_warning = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->voltage_high_warning[0]))), v_slope, v_offset);
        sfp_ddm_info_p->threshold.voltage_low_warning = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->voltage_low_warning[0]))), v_slope, v_offset);
    }

    /* current */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        sfp_ddm_info_p->threshold.bias_high_alarm = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->bias_high_alarm[0]))), i_slope, i_offset);
        sfp_ddm_info_p->threshold.bias_low_alarm = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->bias_low_alarm[0]))), i_slope, i_offset);
        sfp_ddm_info_p->threshold.bias_high_warning = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->bias_high_warning[0]))), i_slope, i_offset);
        sfp_ddm_info_p->threshold.bias_low_warning = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->bias_low_warning[0]))), i_slope, i_offset);
    }
    else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_ddm_info_p->threshold.bias_high_alarm = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->bias_high_alarm[0]))), i_slope, i_offset);
        sfp_ddm_info_p->threshold.bias_low_alarm = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->bias_low_alarm[0]))), i_slope, i_offset);
        sfp_ddm_info_p->threshold.bias_high_warning = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->bias_high_warning[0]))), i_slope, i_offset);
        sfp_ddm_info_p->threshold.bias_low_warning = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->bias_low_warning[0]))), i_slope, i_offset);
    }

    /* QSFP does not have tx power */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        /* tx power */
        tmp_power = SWDRV_MONITOR_DdmConvertTxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_power_high_alarm[0]))), p_slope, p_offset);
        sfp_ddm_info_p->threshold.tx_power_high_alarm = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertTxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_power_low_alarm[0]))), p_slope, p_offset);
        sfp_ddm_info_p->threshold.tx_power_low_alarm = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertTxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_power_high_warning[0]))), p_slope, p_offset);
        sfp_ddm_info_p->threshold.tx_power_high_warning = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertTxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_power_low_warning[0]))), p_slope, p_offset);
        sfp_ddm_info_p->threshold.tx_power_low_warning = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
    }

    /* rx power */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->rx_power_high_alarm[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_high_alarm = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->rx_power_low_alarm[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_low_alarm = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->rx_power_high_warning[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_high_warning = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->rx_power_low_warning[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_low_warning = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
    }
    else if (sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
             sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_high_alarm[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_high_alarm = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_low_alarm[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_low_alarm = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_high_warning[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_high_warning = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_low_warning[0]))), rx_p_4, rx_p_3, rx_p_2, rx_p_1, rx_p_0);
        sfp_ddm_info_p->threshold.rx_power_low_warning = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
    }

    /* 2.Convert the real time A/D value to human readable data */
    /* Temperature, in unit of 1/256 degrees Celsius.
     * SFP, XFP   : [96:97]
     * QSFP, QSFP+: [22:23]
     */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        sfp_ddm_info_p->measured.temperature = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->temperature[0]))), t_slope, t_offset);
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_ddm_info_p->measured.temperature = SWDRV_MONITOR_DdmConvertTemperature(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->temperature[0]))), t_slope, t_offset);
    }

    /* Voltage
     * SFP        : [98:99]
     * QSFP, QSFP+: [26:27]
     */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP)
    {
        sfp_ddm_info_p->measured.voltage = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->voltage[0]))), v_slope, v_offset);
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        sfp_ddm_info_p->measured.voltage = SWDRV_MONITOR_DdmConvertVoltage(SWDRV_MONITOR_DdmGetXFPVoltage(info_ar, aux_monitoring), v_slope, v_offset);
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_ddm_info_p->measured.voltage = SWDRV_MONITOR_DdmConvertVoltage(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->voltage[0]))), v_slope, v_offset);
    }

    /* TX Bias Current
     * SFP, XFP   : [100:101]
     * QSFP, QSFP+: [42:43] channel 1
     *              [44:45] channel 2
     *              [46:47] channel 3
     *              [48:49] channel 4
     */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        sfp_ddm_info_p->measured.tx_bias_current = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_bias_current[0]))),i_slope,i_offset);
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        sfp_ddm_info_p->measured.tx_bias_current = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->tx_bias_current_1[0]))),i_slope,i_offset);
        sfp_ddm_info_p->measured.tx_bias_current_2 = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->tx_bias_current_2[0]))),i_slope,i_offset);
        sfp_ddm_info_p->measured.tx_bias_current_3 = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->tx_bias_current_3[0]))),i_slope,i_offset);
        sfp_ddm_info_p->measured.tx_bias_current_4 = SWDRV_MONITOR_DdmConvertCurrent(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->tx_bias_current_4[0]))),i_slope,i_offset);
    }

    /* Tx power
     * SFP, XFP   : [102:103]
     * QSFP, QSFP+: None
     */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        tmp_power = SWDRV_MONITOR_DdmConvertTxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->tx_power[0]))),p_slope,p_offset);
        /* Convert tx power unit from watt to dbm */
        sfp_ddm_info_p->measured.tx_power = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
    }

    /* Rx power [104:105]
     * SFP, XFP   : [104:105]
     * QSFP, QSFP+: [34:35] channel 1
     *              [36:37] channel 2
     *              [38:39] channel 3
     *              [40:41] channel 4
     */
    if(sfp_type == SWDRV_TYPE_GBIC_ID_SFP || sfp_type == SWDRV_TYPE_GBIC_ID_XFP)
    {
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(sfp_ddm_raw_p->rx_power[0]))),rx_p_4,rx_p_3,rx_p_2,rx_p_1,rx_p_0);
        /* Convert rx power unit from watt to dbm */
        sfp_ddm_info_p->measured.rx_power = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
    }
    else if(sfp_type == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP_PLUS ||
            sfp_type == SWDRV_TYPE_GBIC_ID_QSFP28)
    {
        /* Convert rx power unit from watt to dbm */
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_1[0]))),rx_p_4,rx_p_3,rx_p_2,rx_p_1,rx_p_0);
        sfp_ddm_info_p->measured.rx_power = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_2[0]))),rx_p_4,rx_p_3,rx_p_2,rx_p_1,rx_p_0);
        sfp_ddm_info_p->measured.rx_power_2 = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_3[0]))),rx_p_4,rx_p_3,rx_p_2,rx_p_1,rx_p_0);
        sfp_ddm_info_p->measured.rx_power_3 = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
        tmp_power = SWDRV_MONITOR_DdmConvertRxPower(L_STDLIB_Ntoh16(*((UI16_T*)&(qsfp_ddm_raw_p->rx_power_4[0]))),rx_p_4,rx_p_3,rx_p_2,rx_p_1,rx_p_0);
        sfp_ddm_info_p->measured.rx_power_4 = SWDRV_MONITOR_DdmUnitWatt2Dbm(tmp_power);
    }

#if 0
    /* convert the alarm/warning status */
    sfp_ddm_info_p->alarm_warning_status =
        ((UI32_T)((*((UI16_T*)&(sfp_all_ddm_info.alarm_flags[0]))) << 16) +
         (UI32_T)(*((UI16_T*)&(sfp_all_ddm_info.warning_flags[0])))) & 0xFFC0FFC0; /* don't care about some reserve bit */
#endif

    return;
} /* end of SWDRV_MONITOR_ConvertSfpDdmInfoRaw */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SfpDdmInfoParsing
 * -------------------------------------------------------------------------
 * FUNCTION: Read and parse the sfp DDM EEPROM info.
 * INPUT   : sfp_index
 *           sfp_type
 *           ext_calibrate_needed
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRV_MONITOR_SfpDdmInfoParsing(UI32_T sfp_index, UI8_T sfp_type, BOOL_T ext_calibrate_needed, UI8_T aux_monitoring, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    SWDRV_TYPE_GetSfpDdmInfoFunc_T  read_fn_p;
    UI8_T  qsfp_page = 3;/* change to Page 03 */
    static UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH];

    memset(info_ar, 0, sizeof(info_ar));

    switch(sfp_type)
    {
        case SWDRV_TYPE_GBIC_ID_SFP:
            read_fn_p = &SYSDRV_GetSfpDdmInfo;
            break;
        case SWDRV_TYPE_GBIC_ID_XFP:
            read_fn_p = &SYSDRV_GetSfpInfo;
            break;
        case SWDRV_TYPE_GBIC_ID_QSFP:
        case SWDRV_TYPE_GBIC_ID_QSFP_PLUS:
        case SWDRV_TYPE_GBIC_ID_QSFP28:
            read_fn_p = &SYSDRV_GetSfpInfo;
            if(FALSE == SYSDRV_SetSfpInfo(sfp_index, 127, 1, &qsfp_page))
            {
            /* SYSDRV_SetSfpInfo(sfp_index, 127, 1, &qsfp_page):
             * write 0x3 to page select byte(address: 127) in order to read
             * Free Side Device Threshold/Channel Threshold (Page 03).
             */
                if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
                    BACKDOOR_MGR_Printf("%s(%d): Failed to change QSFP(+)%lu to Page03.\r\n", __FUNCTION__, __LINE__, sfp_index);
                return FALSE;
            }

            break;
        default:
            return FALSE;
    }/* End of switch(sfp_type) */
    if((FALSE == read_fn_p(sfp_index, 0, 128, info_ar)) ||
       (FALSE == read_fn_p(sfp_index, 128, 128, &(info_ar[128]))))
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): Failed to read SFP %lu DDM info from I2C.\r\n", __FUNCTION__, __LINE__, sfp_index);
        SWDRV_OM_SetPortSfpDdmInfoRaw(sfp_index, info_ar);
        return FALSE;
    }
    else
    {
        if (SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        {
            BACKDOOR_MGR_Printf("SfpDdmInfo sfp_index:%lu\n", (unsigned long)sfp_index);
            BACKDOOR_MGR_DumpHex(NULL, sizeof(info_ar), info_ar);
        }

        SWDRV_OM_SetPortSfpDdmInfoRaw(sfp_index, info_ar);
        SWDRV_MONITOR_ConvertSfpDdmInfoRaw(sfp_index, sfp_type, info_ar, ext_calibrate_needed, aux_monitoring, sfp_ddm_info_p);
        sfp_ddm_info_p->measured.rx_los_asserted = SWDRV_MONITOR_IsSfpRxLosAsserted(SYS_VAL_LOCAL_UNIT_ID, sfp_index);
    }

    return TRUE;
}/* End of SWDRV_MONITOR_SfpDdmInfoParsing */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_SfpDdmInfoMeasuredParsing
 * -------------------------------------------------------------------------
 * FUNCTION: Read and parse the sfp DDM EEPROM info.
 * INPUT   : sfp_index
 *           sfp_type
 *           ext_calibrate_needed
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T SWDRV_MONITOR_SfpDdmInfoMeasuredParsing(UI32_T sfp_index, UI8_T sfp_type, BOOL_T ext_calibrate_needed, UI8_T aux_monitoring, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    SWDRV_TYPE_GetSfpDdmInfoFunc_T  read_fn_p;
    UI16_T  offset;
    UI8_T   size;
    static  UI8_T  info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH];

    memset(info_ar, 0, sizeof(info_ar));
    SWDRV_OM_GetPortSfpDdmInfoRaw(sfp_index, info_ar);

    /* is_xfp=FALSE
     *     read from SYS_HWCFG_I2C_SLAVE_EEPROM+1 at offset 0 with 256 bytes
     * is_xfp=TRUE
     *     read from SYS_HWCFG_I2C_SLAVE_EEPROM at offset 96 with 14 bytes
     */
    switch(sfp_type)
    {
        case SWDRV_TYPE_GBIC_ID_SFP:
            offset = SWDRV_TYPE_DDM_DIAG_OFFSET;
            size = SWDRV_TYPE_DDM_DIAG_SIZE;
            read_fn_p = &SYSDRV_GetSfpDdmInfo;
            break;
        case SWDRV_TYPE_GBIC_ID_XFP:
            offset = SWDRV_TYPE_DDM_DIAG_OFFSET;
            size = SWDRV_TYPE_DDM_DIAG_SIZE;
            read_fn_p = &SYSDRV_GetSfpInfo;
            break;
        case SWDRV_TYPE_GBIC_ID_QSFP:
        case SWDRV_TYPE_GBIC_ID_QSFP_PLUS:
        case SWDRV_TYPE_GBIC_ID_QSFP28:
            offset = SWDRV_TYPE_DDM_DIAG_OFFSET_QSFP;
            size = SWDRV_TYPE_DDM_DIAG_SIZE_QSFP;
            read_fn_p = &SYSDRV_GetSfpInfo;
            break;
        default:
            return FALSE;
    }/* End of switch(sfp_type) */


    if(FALSE == read_fn_p(sfp_index, offset, size, &(info_ar[offset])))
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): Failed to read GBIC %lu DDM info from I2C.\r\n", __FUNCTION__, __LINE__, sfp_index);
        /* Sometimes the i2c read might fail, record the fail counts.
         * If the fail count is less than 10 times, treat it as unchanged.
         */
        swdrv_sfp_ddm_error_count[sfp_index-1]++;
        if(swdrv_sfp_ddm_error_count[sfp_index-1] >= 10)
        {
            memset(&(info_ar[offset]), 0, size);
            SWDRV_OM_SetPortSfpDdmInfoRaw(sfp_index, info_ar);
            memset(&sfp_ddm_info_p->measured, 0, sizeof(SWDRV_TYPE_SfpDdmInfoMeasured_T));
        }
        return FALSE;
    }
    else
    {
        /* Clear the fail count once it succeed. */
        swdrv_sfp_ddm_error_count[sfp_index-1] = 0;
        SWDRV_OM_SetPortSfpDdmInfoRaw(sfp_index, info_ar);
        SWDRV_MONITOR_ConvertSfpDdmInfoRaw(sfp_index, sfp_type, info_ar, ext_calibrate_needed, aux_monitoring, sfp_ddm_info_p);
    }

    return TRUE;
}/* End of SWDRV_MONITOR_SfpDdmInfoMeasuredParsing */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_PortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Update sfp EEPROM info when present. If changed, call back or
 * send
 *           notification through ISC(slave).
 * INPUT   : unit
 *           sfp_index 
 *           sfp_type
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SWDRV_MONITOR_PortSfpInfo(UI32_T unit, UI32_T sfp_index, UI8_T sfp_type)
{
    /* local_sfp_info requires large buffer
     * so declare it as static to avoid consuming too much stack size
     */
    static SWDRV_TYPE_SfpInfo_T local_sfp_info;
    static SWDRV_TYPE_SfpDdmInfo_T local_sfp_ddm_info;

    if(sfp_index < 1 || sfp_index > SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT)
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): Invalid argument(sfp_index):%lu\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }

    memset(&local_sfp_info, 0, sizeof(SWDRV_TYPE_SfpInfo_T));
    local_sfp_info.identifier = sfp_type;

    if(TRUE == SWDRV_MONITOR_SfpInfoReadAndParse(sfp_index, &local_sfp_info))
    {
        SWDRV_OM_SetPortSfpInfo(sfp_index, &local_sfp_info);
        if(local_sfp_info.support_ddm == TRUE)
        {
            memset(&local_sfp_ddm_info, 0, sizeof(SWDRV_TYPE_SfpDdmInfo_T));
            SWDRV_MONITOR_SfpDdmInfoParsing(sfp_index, local_sfp_info.identifier, local_sfp_info.ext_calibrate_needed, local_sfp_info.aux_monitoring, &local_sfp_ddm_info);
            SWDRV_OM_SetPortSfpDdmInfo(sfp_index, &local_sfp_ddm_info);
        }
    }
    else
    {
        SWDRV_OM_SetPortSfpInfo(sfp_index, &local_sfp_info);
        return FALSE;
    }

    SWDRV_Notify_PortSfpInfo(unit, sfp_index, &local_sfp_info);
    if(local_sfp_info.support_ddm == TRUE)
    {
        SWDRV_Notify_PortSfpDdmInfo(unit, sfp_index, &local_sfp_ddm_info);
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_PortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Update sfp DDM EEPROM info when present. If changed, call back
 *           or send notification through ISC(slave).
 * INPUT   : port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : SWDRV_TYPE_DDM_DIAG_SIZE:
 *           For SFP, read from 96~105 (size:10)
 *           For XFP, read from 96~109 (size:14)
 *           For QSFP, read from 22~49 (size:28)
 * -------------------------------------------------------------------------*/
static void SWDRV_MONITOR_PortSfpDdmInfo(UI32_T port)
{
    UI32_T stack_id, sfp_index;
    UI32_T ticks;
    static SWDRV_TYPE_SfpInfo_T local_sfp_info;
    static SWDRV_TYPE_SfpDdmInfo_T local_sfp_ddm_info;

    SWDRV_OM_GetSystemInfoStackId(&stack_id);
    if(FALSE == STKTPLG_POM_UserPortToSfpIndex(stack_id, port, &sfp_index))
    {
        return;
    }

    if(sfp_index < 1 || sfp_index > SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT)
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): Invalid argument(sfp_index):%lu\r\n", __FUNCTION__, __LINE__, sfp_index);
        return;
    }

    memset(&local_sfp_info, 0, sizeof(SWDRV_TYPE_SfpInfo_T));
    SWDRV_OM_GetPortSfpInfo(sfp_index, &local_sfp_info);

    if(local_sfp_info.support_ddm != TRUE)
    {
        return;
    }

    SWDRV_OM_GetSystemInfoStackId(&stack_id);

    memset(&local_sfp_ddm_info, 0, sizeof(SWDRV_TYPE_SfpDdmInfo_T));
    SWDRV_OM_GetPortSfpDdmInfo(sfp_index, &local_sfp_ddm_info);

    SWDRV_MONITOR_SfpDdmInfoMeasuredParsing(sfp_index, local_sfp_info.identifier, local_sfp_info.ext_calibrate_needed, local_sfp_info.aux_monitoring, &local_sfp_ddm_info);

    local_sfp_ddm_info.measured.rx_los_asserted = SWDRV_MONITOR_IsSfpRxLosAsserted(SYS_VAL_LOCAL_UNIT_ID, sfp_index);

    SWDRV_OM_SetPortSfpDdmInfo(sfp_index, &local_sfp_ddm_info);

    SWDRV_Notify_PortSfpDdmInfoMeasured(stack_id, sfp_index, &local_sfp_ddm_info.measured);

    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_SHOW_DDM_TEMP))
    {

        SYS_TIME_GetSystemUpTimeByTick(&ticks);
        BACKDOOR_MGR_Printf("%s(%d): SFP %2lu temperature: %f degree C(up_time=%lu secs)\r\n", __FUNCTION__, __LINE__, sfp_index, local_sfp_ddm_info.measured.temperature, ticks/SYS_BLD_TICKS_PER_SECOND);
    }
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_MONITOR_GetSfpPresentStatusCheckDelay
 * -------------------------------------------------------------------------
 * FUNCTION: Check if sfp is present or not due to present time and delay
 * INPUT   : sfp_index
 *           is_present_p
 * OUTPUT  : is_present_p
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
static void   SWDRV_MONITOR_GetSfpPresentStatusCheckDelay(UI32_T sfp_index, UI8_T *is_present_p)
{
    UI32_T current_tick, timeout_tick, sfp_present_delay_ticks;

    if(*is_present_p)/* sfp is present */
    {
        /* update present tick(timestamp) if present ticks is zero
         */
        if(swdrv_sfp_present_ticks_ar[sfp_index-1] == 0)
        {
            swdrv_sfp_present_ticks_ar[sfp_index-1] = SYSFUN_GetSysTick();
            *is_present_p = FALSE;
        }
        else
        {
            /* check if present time is long enough if it is not in present
             * stable bitmap
             */
            if(L_PBMP_GET_PORT_IN_PBMP_ARRAY(swdrv_sfp_present_stable_bmp, sfp_index) == 0)
            {
                current_tick = SYSFUN_GetSysTick();
                sfp_present_delay_ticks = SWDRV_OM_GetSfpPresentDelay();
                timeout_tick = swdrv_sfp_present_ticks_ar[sfp_index-1] + sfp_present_delay_ticks;

                if (!L_MATH_TimeOut32(current_tick, timeout_tick))
                {
                    /* present time is not long enough,
                     * take it as 'not present'.
                     */
                    *is_present_p = FALSE;
                }
                else
                {
                    /* present time is long enough,
                     * set the corresponding bit in sfp present stable bitmap
                     */
                    L_PBMP_SET_PORT_IN_PBMP_ARRAY(swdrv_sfp_present_stable_bmp, sfp_index);
                }
            }
        }
    }
    else /* sfp is not present */
    {
        /* reset present tick(timestamp) as 0
         */ 
        swdrv_sfp_present_ticks_ar[sfp_index-1] = 0;
        /* unset the corresponding bit in sfp present stable bitmap
         */
        L_PBMP_UNSET_PORT_IN_PBMP_ARRAY(swdrv_sfp_present_stable_bmp, sfp_index);
    }
}
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

/* FUNCTION NAME: SWDRV_MONITOR_IsSfpRxLosAsserted
 * PURPOSE: This function is used to check SFP transceiver's rx-loss.
 * INPUT:   unit      -- unit id, must be SYS_VAL_LOCAL_UNIT_ID now
 *          sfp_index -- SFP index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- Asserted.
 *          FALSE -- Not asserted.
 * NOTES:   1. This API must be called when sfp is present.
 *          2. This API will check the SFP EEPROM to know whether the inserted
 *             SFP has rx_los signal implemented or has rx_los signal
 *             inverted.
 *          3. If SFP EEPROM indicates that rx_los signal is not implemented,
 *             the return value will always be FALSE.
 *          4. If this API is called when sfp is not present,
 *             the result is always FALSE.
 *          5. This API does not support to use on non-local unit.
 */
static BOOL_T SWDRV_MONITOR_IsSfpRxLosAsserted(UI32_T unit, UI32_T sfp_index)
{
    BOOL_T rx_los_asserted = FALSE;
#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    UI8_T rx_los_status;
    BOOL_T is_rx_los_implemented, is_rx_los_inverted;
    UI32_T my_unit_id;
#endif
    static SWDRV_TYPE_SfpInfo_T sfp_info;//check if create another api

    memset(&sfp_info, 0, sizeof(SWDRV_TYPE_SfpInfo_T));
    SWDRV_OM_GetPortSfpInfo(sfp_index, &sfp_info);

    if(unit != SYS_VAL_LOCAL_UNIT_ID)
    {
        printf("%s(%d): this api does not support on non-local unit(%lu).\r\n", __FUNCTION__, __LINE__, unit);
        return FALSE;
    }

    if(sfp_index < 1 || sfp_index > SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT)
    {
        printf("%s(%d): Invalid sfp_index: %lu\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }

#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    STKTPLG_OM_GetMyUnitID(&my_unit_id);

    is_rx_los_implemented = sfp_info.is_rx_los_implemented;
    is_rx_los_inverted = sfp_info.is_rx_los_inverted;

    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        BACKDOOR_MGR_Printf("%s(%d): sfp_index=%lu rx_los_imp=%u rx_los_inv=%u\r\n",
        __FUNCTION__, __LINE__, sfp_index, is_rx_los_implemented, is_rx_los_inverted);

    if(is_rx_los_implemented == TRUE)
    {
        if(SWDRV_MONITOR_GetSfpRxLosStatus(my_unit_id, sfp_index, &rx_los_status) == FALSE)
        {
            if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
                BACKDOOR_MGR_Printf("%s(%d): Access SFP Rx LOS status on SFP %lu failed.\r\n", __FUNCTION__, __LINE__, sfp_index);
            return FALSE;
        }

        if(is_rx_los_inverted == TRUE)
            rx_los_asserted = (rx_los_status == (~SYS_HWCFG_GBIC_RX_LOS_BIT & (SYS_HWCFG_GBIC_RX_LOS_BIT))) ? TRUE:FALSE;
        else
            rx_los_asserted = (rx_los_status == SYS_HWCFG_GBIC_RX_LOS_BIT) ?  TRUE:FALSE;
    }
    else
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): RX_LOS signal not implemented on SFP %lu\r\n",
            __FUNCTION__, __LINE__, sfp_index);

        /* ignore rx_los info if the SFP module doesn't implement
         * just treat rx_los as not asserted.
         */
        rx_los_asserted = FALSE;
    }
#endif

    if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
        BACKDOOR_MGR_Printf("%s(%d): rx_los_asserted=%u\r\n", __FUNCTION__, __LINE__, rx_los_asserted);

    return rx_los_asserted;
}

#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
/* FUNCTION NAME: SWDRV_MONITOR_GetSfpRxLosStatus
 * PURPOSE: This function is used to get SFP RX LOS status of the specified
 *          sfp index.
 * INPUT:   sfp_index    -- SFP index. (1 based)
 * OUTPUT:  rx_los_status_p -- 1 : rx los signal status is high
 *                             0 : rx los signal status is low
 * RETUEN:  TRUE  -- Get SFP RX LOS status successfully.
 *          FALSE -- Failed to get SFP RX LOS status.
 * NOTES:   Need to define SYS_HWCFG_SFP_RX_LOS_STATUS_ACCESS_METHOD if
 *          its access method are not through PHYSICAL_ADDR_ACCESS.
 */
static BOOL_T SWDRV_MONITOR_GetSfpRxLosStatus(UI32_T unit, UI32_T sfp_index, UI8_T *rx_los_status_p)
{
    UI32_T board_id = 0;
    UI8_T rx_los_info;

#if (SYS_HWCFG_SFP_RX_LOS_STATUS_ACCESS_METHOD==SYS_HWCFG_REG_ACCESS_METHOD_I2C)
    if(I2CDRV_GetI2CInfo(SYS_HWCFG_SFP_RX_LOS_ASIC_I2C_ADDRESS, swdrv_sfp_rx_los_status_addr_ar[sfp_index-1], 1, &rx_los_info)==FALSE)
#elif (SYS_HWCFG_SFP_RX_LOS_STATUS_ACCESS_METHOD==SYS_HWCFG_REG_ACCESS_METHOD_SYS_HWCFG_API)
    if(STKTPLG_OM_GetUnitBoardID(unit, &board_id) == FALSE)
    {
        printf("%s(%d): Failed to get my board id.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(!SYS_HWCFG_GetSFPRXLOSStatus(board_id, sfp_index, &rx_los_info))
#else
    if (FALSE == PHYSICAL_ADDR_ACCESS_Read(swdrv_sfp_rx_los_status_addr_ar[sfp_index-1], 1, 1, &rx_los_info))
#endif
    {
        if(SWDRV_OM_GetDebugFlag(SWDRV_TYPE_DEBUG_FLAG_GBIC_INFO))
            BACKDOOR_MGR_Printf("%s(%d): Access SFP rx_los status for SFP %lu failed\r\n", __FUNCTION__, __LINE__, sfp_index);
        return FALSE;
    }

    *rx_los_status_p = (rx_los_info & swdrv_sfp_rx_los_mask_ar[sfp_index-1])
                        >> swdrv_sfp_rx_los_bit_shift_ar[sfp_index-1];

    return TRUE;
}
#endif /* #if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE) */
