/*--------------------------------------------------------------------------+ 
 * FILE NAME - nmtr_mgr.c                                                   +
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file defines Network Monitor APIs and Task                +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *   Arthur Wu      24/07/2001       creation                               +
 *   Arthur Wu      18/10/2002       add Evnet Handle feature               +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * -------------------------------------------------------------------------*/


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "stktplg_pom.h"
#include "nmtrdrv.h" 
#include "nmtr_mgr.h"
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
#include "nmtr_hist.h"
#endif
//#include "nmtrdrv_om.h" /*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not included,changed makefile.am*/
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "trk_pmgr.h"
#include "syslog_type.h"
//#include "syslog_mgr.h"
//#include "syslog_task.h"
#include "leaf_es3626a.h"
#include "leaf_3635.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "sys_module.h"
//#include "mib2_mgr.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "sys_time.h"

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
#include "vlan_lib.h"
#include "vlan_om.h"
#endif

/* CONSTANT DECLARATIONS
 */
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
#define CHIP_SUPPORT_VLAN_COUNTER
#endif

#define NMTR_MGR_FUNCTION_NUMBER 0xffffffff
#define NMTR_MGR_TIME_PERIOD     300
#define NMTR_MGR_TIME_UNIT       5
#define NMTR_MGR_SLICE_NUMBER ((NMTR_MGR_TIME_PERIOD) / (NMTR_MGR_TIME_UNIT))   /*300 seconds, one unit is 5 seconds*/
#define MAX_OCTETS_NUMBER_OF_SPEED_1000 ((1000 * SYS_TYPE_1M_BYTES)/8)
#define MAX_OCTETS_NUMBER_OF_SPEED_100  ((100 * SYS_TYPE_1M_BYTES)/8)
#define MAX_OCTETS_NUMBER_OF_SPEED_10   ((10 * SYS_TYPE_1M_BYTES)/8)
#define MAX_BITS_NUMBER_OF_SPEED_1000 (1000 * 1000000)
#define MAX_BITS_NUMBER_OF_SPEED_100  (100  * 1000000)
#define MAX_BITS_NUMBER_OF_SPEED_10   (10   * 1000000)

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
#define NMTR_MGR_HIST_UTIL_BASE_BANDWIDTH_KBPS_FOR_XMIT_BYTES_STATISTIC      1000
#endif

/*
##1 : NMTR_EH_NOT_MASTER
##2 : NMTR_EH_ERROR_MEMORY
##3 : NMTR_EH_PORT_NOT_EXIST
*/

/* LOCAL TYPES
 */


/* MACRO DECLARATIONS
 */
#ifndef SYSFUN_USE_CSC
#define SYSFUN_USE_CSC(r)
#define SYSFUN_RELEASE_CSC()
#endif

/* kh_shi remove the definition */
#define NMTR_MGR_ENTER_CRITICAL()    
#define NMTR_MGR_EXIT_CRITICAL()     

/* define the macros about the port duplex
 */
#define NMTR_MGR_IS_FULL_DUPLEX()   ( port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex10    ||  \
                                      port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex100   ||  \
                                      port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000  ||  \
                                      port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex10g )
                                
#define NMTR_MGR_IS_HALF_DUPLEX()   ( port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10    ||  \
                                      port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex100   ||  \
                                      port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex1000  ||  \
                                      port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10g )
                                     
/* define the macros about the port type
 */ 
#define NMTR_MGR_IS_USER_PORT_TYPE(port_type)       ( port_type == SWCTRL_LPORT_NORMAL_PORT         ||  \
                                                      port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER )
                                                   
#define NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type)      ( port_type == SWCTRL_LPORT_TRUNK_PORT )
                                         
#define NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type)    ( port_type == SWCTRL_LPORT_UNKNOWN_PORT )

/* define the macros about ifindex type
 */
#define NMTR_MGR_IS_VLAN(ifindex)                   ( ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER )
                                          
/* define the macro for more painless use of UI64 manipulation
 */
#define NMTR_MGR_UI64_Add(v1_p,v2) \
    L_STDLIB_UI64_Add(&(L_STDLIB_UI64_H32(*(v1_p))), &(L_STDLIB_UI64_L32(*(v1_p))), L_STDLIB_UI64_H32(v2), L_STDLIB_UI64_L32(v2));

#define NMTR_MGR_UI64_Sub(v1_p,v2) \
    L_STDLIB_UI64_Sub(&(L_STDLIB_UI64_H32(*(v1_p))), &(L_STDLIB_UI64_L32(*(v1_p))), L_STDLIB_UI64_H32(v2), L_STDLIB_UI64_L32(v2));

#define NMTR_MGR_UI64_Multi(v1_p,v2) \
    L_STDLIB_UI64_Multi(&(L_STDLIB_UI64_H32(*(v1_p))), &(L_STDLIB_UI64_L32(*(v1_p))), (UI32_T)(v2));

#define NMTR_MGR_UI64_Div(v1_p,v2) \
    L_STDLIB_UI64_Div(&(L_STDLIB_UI64_H32(*(v1_p))), &(L_STDLIB_UI64_L32(*(v1_p))), (I32_T)(v2));
                                          
/*------------------------------------------------------------------------
 * MACRO NAME - NMTR_MGR_CMP_UI64T
 *------------------------------------------------------------------------
 * FUNCTION: 64 bit unsigned binary comparison
 * INPUT   : __high_1, __low_1, __high_2, __low_2
 * OUTPUT  : __result <  0:  variable_1 < variable_2
 *         : __result >  0:  variable_1 > variable_2
 *         : __result == 0:  variable_1 = variable_2
 * NOTES   : (__high_1, __low_1) -> variable_1
 *           (__high_2, __low_2) -> variable_2
 *------------------------------------------------------------------------
*/
#define  NMTR_MGR_CMP_UI64T(__result, __high_1, __low_1, __high_2, __low_2)     \
{                                                                               \
    (__result) = ((__high_1) - (__high_2));                                           \
    if ( (__result) == 0 )                                                        \
    {                                                                           \
        (__result) = ((__low_1) - (__low_2));                                         \
    }                                                                           \
}

/* define the macros about different counter update
 */
#define NMTR_MGR_UpdateDiffCounter(old_counter,new_counter,different)   \
    different = ((UI32_T)(new_counter) - (UI32_T)(old_counter))

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NMTR_MGR_InitStatistics(void);

static void NMTR_MGR_SumOfIfTableStats              (SWDRV_IfTableStats_T       *destination_if_table_stats, 
                                                    SWDRV_IfTableStats_T        source_if_table_stats );
static void NMTR_MGR_SumOfIfXTableStats             (SWDRV_IfXTableStats_T      *destination_if_xtable_stats, 
                                                    SWDRV_IfXTableStats_T       source_if_xtable_stats );
static void NMTR_MGR_SumOfEtherLikeStats            (SWDRV_EtherlikeStats_T     *destination_ether_like_stats, 
                                                    SWDRV_EtherlikeStats_T      source_ether_like_stats );
static void NMTR_MGR_SumOfEtherLikePause            (SWDRV_EtherlikePause_T     *destination_ether_like_pause, 
                                                    SWDRV_EtherlikePause_T      source_ether_like_pause );
static void NMTR_MGR_SumOfRmonStats                 (SWDRV_RmonStats_T          *destination_rmon_stats, 
                                                    SWDRV_RmonStats_T           source_rmon_stats );
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void NMTR_MGR_SumOfIfPerQStats               (SWDRV_IfPerQStats_T        *destination_stats, 
                                                    SWDRV_IfPerQStats_T         source_stats );
#endif
#if (SYS_CPNT_PFC == TRUE)
static void NMTR_MGR_SumOfPfcStats                  (SWDRV_PfcStats_T           *destination_stats, 
                                                    SWDRV_PfcStats_T            source_stats );
#endif
#if (SYS_CPNT_CN == TRUE)
static void NMTR_MGR_SumOfQcnStats                  (SWDRV_QcnStats_T           *destination_stats, 
                                                    SWDRV_QcnStats_T            source_stats );
#endif
static void NMTR_MGR_SumOfUtilizationStats          (NMTR_MGR_Utilization_T     *destination_utilization_stats, 
                                                    NMTR_MGR_Utilization_T      source_utilization_stats );

static BOOL_T NMTR_MGR_GetNextIndexFromPortList (UI32_T *index,
                                                 UI8_T  *port_list,
                                                 UI32_T max_port_num);

static void NMTR_MGR_GetDeltaIfCounter(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats);
static void NMTR_MGR_GetDeltaIfXCounter(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats);
static void NMTR_MGR_GetDeltaEtherLikeCounter(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats);
static void NMTR_MGR_GetDeltaEtherLikePauseCounter(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause);
static void NMTR_MGR_GetDeltaRmonCounter(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void NMTR_MGR_GetDeltaIfPerQCounter(UI32_T ifindex, SWDRV_IfPerQStats_T *stats);
#endif
#if (SYS_CPNT_PFC == TRUE)
static void NMTR_MGR_GetDeltaPfcCounter(UI32_T ifindex, SWDRV_PfcStats_T *stats);
#endif
#if (SYS_CPNT_CN == TRUE)
static void NMTR_MGR_GetDeltaQcnCounter(UI32_T ifindex, SWDRV_QcnStats_T *stats);
#endif
static void NMTR_MGR_SumOfUtilizationStats300secs(NMTR_MGR_Utilization_300_SECS_T *destination_utilization_stats,
                                                  NMTR_MGR_Utilization_300_SECS_T source_utilization_stats );

static BOOL_T NMTR_MGR_UpdateIFTableStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num);
static BOOL_T NMTR_MGR_UpdateIFXTableStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num);
static BOOL_T NMTR_MGR_UpdateRmonStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num);
static BOOL_T NMTR_MGR_UpdateEtherLikeStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num);     
static BOOL_T NMTR_MGR_UpdateEtherLikePause_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static BOOL_T NMTR_MGR_UpdateIfPerQStats_CallBack(UI32_T unit, 
                                               UI32_T start_port, 
                                               UI32_T updated_port_num);
#endif
#if (SYS_CPNT_PFC == TRUE)
static BOOL_T NMTR_MGR_UpdatePfcStats_CallBack(UI32_T unit, 
                                               UI32_T start_port, 
                                               UI32_T updated_port_num);
#endif
#if (SYS_CPNT_CN == TRUE)
static BOOL_T NMTR_MGR_UpdateQcnStats_CallBack(UI32_T unit, 
                                               UI32_T start_port, 
                                               UI32_T updated_port_num);
#endif
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
static BOOL_T NMTR_MGR_UpdateHistory_CallBack(UI32_T unit,
                                                 UI32_T start_port,
                                                 UI32_T updated_port_num);
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static BOOL_T NMTR_MGR_UpdateIFXTableStatsForVlan_CallBack(UI32_T unit,
                                                 UI32_T start_vid,
                                                 UI32_T updated_num);
#endif
static BOOL_T NMTR_MGR_Update300sUtilization_CallBack(void);
static void NMTR_MGR_DiffTwoUI64Var(UI64_T old_var, UI64_T new_var, UI64_T *diff_var);
static void NMTR_MGR_SumThreeUI64Var(UI64_T var1, UI64_T var2, UI64_T var3, UI64_T *total_var);
static void NMTR_MGR_Backdoor_Menu(void);
static void NMTR_MGR_BD_ShowUnitPortIFInfo(void);
static void NMTR_MGR_BD_ShowUnitPortIFXInfo(void);
static void NMTR_MGR_BD_ShowUnitPortRmonInfo(void);
static void NMTR_MGR_BD_ShowUnitPortEtherLikeInfo(void);
static void NMTR_MGR_BD_ShowUnitPortEtherLikePauseInfo(void);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void NMTR_MGR_BD_ShowUnitPortIfPerQInfo(void);
#endif
#if (SYS_CPNT_PFC == TRUE)
static void NMTR_MGR_BD_ShowUnitPortPfcInfo(void);
#endif
#if (SYS_CPNT_CN == TRUE)
static void NMTR_MGR_BD_ShowUnitPortQcnInfo(void);
#endif
static void NMTR_MGR_BD_ShowUnitPortUtilization300secs(void);
static void NMTR_MGR_BD_ShowUnitPortUtilization(void);
static void NMTR_MGR_BD_ShowSystemwideIfXTableStats(void);

/*tempate solution*/
extern void NMTRDRV_OM_GetIfStats(UI32_T lunit,UI32_T lport, SWDRV_IfTableStats_T *if_stats_p);
extern void NMTRDRV_OM_GetIfXStats(UI32_T lunit,UI32_T lport, SWDRV_IfXTableStats_T *ifx_stats_p);
extern void NMTRDRV_OM_GetRmonStats(UI32_T lunit,UI32_T lport, SWDRV_RmonStats_T *rmon_stats_p);
extern void NMTRDRV_OM_GetEtherlikeStats(UI32_T lunit,UI32_T lport, SWDRV_EtherlikeStats_T *etherlike_stats_p);
extern void NMTRDRV_OM_GetEtherlikePause(UI32_T lunit,UI32_T lport, SWDRV_EtherlikePause_T *etherlike_pause_p);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
extern void NMTRDRV_OM_GetIfPerQStats(UI32_T lunit,UI32_T lport, SWDRV_IfPerQStats_T *stats_p);
#endif
#if (SYS_CPNT_PFC == TRUE)
extern void NMTRDRV_OM_GetPfcStats(UI32_T lunit,UI32_T lport, SWDRV_PfcStats_T *stats_p);
#endif
#if (SYS_CPNT_CN == TRUE)
extern void NMTRDRV_OM_GetQcnStats(UI32_T lunit,UI32_T lport, SWDRV_QcnStats_T *stats_p);
#endif


/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T                   units_in_stack;
static UI32_T                   my_unit_id;
static BOOL_T                   nmtr_mgr_provision_complete;
static SWDRV_IfTableStats_T     nmtr_mgr_if_table_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static SWDRV_IfXTableStats_T    nmtr_mgr_if_xtable_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];                           
static SWDRV_EtherlikeStats_T   nmtr_mgr_ether_like_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static SWDRV_EtherlikePause_T   nmtr_mgr_ether_like_pause[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static SWDRV_RmonStats_T        nmtr_mgr_rmon_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];                                                                  
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static SWDRV_IfPerQStats_T      nmtr_mgr_ifperq_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_PFC == TRUE)
static SWDRV_PfcStats_T         nmtr_mgr_pfc_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_CN == TRUE)
static SWDRV_QcnStats_T         nmtr_mgr_qcn_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static SWDRV_IfXTableStats_T    nmtr_mgr_if_xtable_stats_for_vlan[SYS_ADPT_MAX_VLAN_ID];
#endif
static NMTR_MGR_DiffCounter_T   nmtr_mgr_different_counter[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static NMTR_MGR_Utilization_T   nmtr_mgr_utilization_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static NMTR_MGR_Utilization_300_SECS_T	           nmtr_mgr_utilization_300_secs[SYS_ADPT_TOTAL_NBR_OF_LPORT][NMTR_MGR_SLICE_NUMBER];
static NMTR_MGR_Utilization_300_SECS_LAST_TIME_T   nmtr_mgr_utilization_300_secs_last_time[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static SWDRV_IfTableStats_T     nmtr_mgr_if_table_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
static SWDRV_IfXTableStats_T    nmtr_mgr_if_xtable_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
static SWDRV_EtherlikeStats_T   nmtr_mgr_ether_like_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
static SWDRV_EtherlikePause_T   nmtr_mgr_ether_like_pause_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
static SWDRV_RmonStats_T        nmtr_mgr_rmon_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static SWDRV_IfPerQStats_T      nmtr_mgr_ifperq_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
#endif
#if (SYS_CPNT_PFC == TRUE)
static SWDRV_PfcStats_T         nmtr_mgr_pfc_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
#endif
#if (SYS_CPNT_CN == TRUE)
static SWDRV_QcnStats_T         nmtr_mgr_qcn_stats_base[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
#endif
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
static NMTR_TYPE_HistCounterInfo_T nmtr_mgr_last_hist_counter[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static NMTR_TYPE_HistCounterInfo_T nmtr_mgr_diff_hist_counter[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];   /* last one for temp use */
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static SWDRV_IfXTableStats_T    nmtr_mgr_if_xtable_stats_base_for_vlan[SYS_ADPT_MAX_VLAN_ID];
#endif

#if (SYS_CPNT_NMTR_SYNC_NDEV == TRUE)
static void NMTR_MGR_UpdateNetDevStatistics_CallBack(void);
static void NMTR_MGR_ResetNetDevStatistics(UI32_T ifidx);
#endif /* #if (SYS_CPNT_NMTR_SYNC_NDEV == TRUE) */

static BOOL_T in_first_300_secs = TRUE;
static UI32_T sysuptime_base    = 0;
static UI32_T nmtr_mgr_slice_num       = 0;

SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_Init                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize kernel resources               
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : Invoked by root.c()                                                        
 *------------------------------------------------------------------------*/
void NMTR_MGR_Init(void)
{
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    NMTR_HIST_Init();
#endif
    return;
} 
    
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_Create_InterCSC_Relation                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : Invoked by NMTR_INIT_Initiate_System_Resources()                                                        
 *------------------------------------------------------------------------*/
void NMTR_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("nmtr_mgr", 
                                                      SYS_BLD_NETACCESS_NMTR_IPCMSGQ_KEY,
                                                      NMTR_MGR_Backdoor_Menu);
    return;
}
    
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_EnterTransitionMode                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize counters                
 * INPUT   : None                                                  
 * OUTPUT  : None                                                  
 * RETURN  : None                                                  
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTR_MGR_EnterTransitionMode(void)
{
    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE(); 

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    NMTR_HIST_Reset();
#endif
}

/*------------------------------------------------------------------------------
 * Function : NMTR_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTR_MGR_SetTransitionMode(void)
{  
    nmtr_mgr_provision_complete = FALSE;

    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}	/*	end of NMTR_MGR_SetTransitionMode	*/

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_EnterMasterMode                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will enable network monitor services
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTR_MGR_EnterMasterMode(void)
{             
    STKTPLG_POM_GetNumberOfUnit(&units_in_stack);
    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    /* initial statistic counters
     */
    NMTR_MGR_InitStatistics();

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    {
        UI32_T ifindex;

        for (ifindex = 1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++)
        {
            if (!SWCTRL_LogicalPortExisting(ifindex))
            {
                continue;
            }
            NMTR_HIST_SetDefaultCtrlEntry(ifindex);
        }
    }
#endif

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE(); 
}


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_EnterSlaveMode                                   
 *------------------------------------------------------------------------|
 * FUNCTION: This function will disable network monitor services          
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
void NMTR_MGR_EnterSlaveMode(void)
{   
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_MGR_HandleHotInsertion
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
void NMTR_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    UI32_T  ifindex;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;
    UI32_T  port_type;
    
    /* In this event, because this user port is new coming,
     * need to clean up the database, just link enter master mode.
     */
    for(ifindex = starting_port_ifindex;
        ifindex<= starting_port_ifindex+number_of_port-1;
        ifindex++ )
    {
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
        if (SWCTRL_LPORT_UNKNOWN_PORT == port_type ||
            SWCTRL_LPORT_TRUNK_PORT   == port_type )
        {
            continue;
        }
        
        NMTRDRV_ClearPortCounter (unit, port);
        NMTR_MGR_ClearPortCounter(ifindex);

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
        NMTR_HIST_SetDefaultCtrlEntry(ifindex);
#endif

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
        if (port == 1)
        {
            NMTRDRV_ClearVlanCounter(unit, 0);
        }
#endif
    }
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_MGR_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void NMTR_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T  ifindex;

    /* In this event, because this user port is no more present,
     * need to clean up the database.
     */
    for(ifindex = starting_port_ifindex;
        ifindex<= starting_port_ifindex+number_of_port-1;
        ifindex++ )
    {
        NMTR_MGR_ClearPortCounter(ifindex);

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
        NMTR_HIST_DestroyCtrlEntryByIfindex(ifindex);
#endif
    }
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetOperationMode                                   
 *------------------------------------------------------------------------|
 * FUNCTION: This function will return present opertaion mode         
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
UI32_T NMTR_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Net monitor module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTR_MGR_ProvisionComplete(void)
{
    //NMTRDRV_ProvisionComplete();
    nmtr_mgr_provision_complete = TRUE;
}


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearPortCounter                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the port conuter                 
 * INPUT   : UI32_T ifindex - interface index
 * OUTPUT  : None                                                  
 * RETURN  : Boolean        - TRUE : success , FALSE: failed
 * NOTE    : For clearing the port counter after system start up
 *------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_ClearPortCounter(UI32_T ifindex)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;
    BOOL_T                  retval;
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   
    
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {    
        if (NMTR_MGR_IS_VLAN(ifindex))  
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            UI32_T unit, vid;

            VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);

            if (VLAN_OM_IsVlanExisted(vid))
            {
                NMTR_MGR_ENTER_CRITICAL ();    

                /* clear the statistics in NMTR_MGR 
                 */
                memset (&nmtr_mgr_if_xtable_stats_for_vlan[vid-1],  0, sizeof (SWDRV_IfXTableStats_T));

                /* clear the chip statistics 
                */
                for (unit = 0; STKTPLG_OM_GetNextUnit(&unit); )
                {
                    retval = retval &&
                        NMTRDRV_ClearVlanCounter (unit, port);
                }

                NMTR_MGR_EXIT_CRITICAL ();
                
                //SYSFUN_RELEASE_CSC();
                return retval;
            }
        #else   /* the chip does not support vlan counter */ 
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }
    
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
    
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {        
            NMTR_MGR_ENTER_CRITICAL ();    
            /* clear the statistics in NMTR_MGR 
             */
            memset (&nmtr_mgr_if_table_stats[ifindex-1],   0, sizeof (SWDRV_IfTableStats_T));
            memset (&nmtr_mgr_if_xtable_stats[ifindex-1],  0, sizeof (SWDRV_IfXTableStats_T));
            memset (&nmtr_mgr_ether_like_stats[ifindex-1], 0, sizeof (SWDRV_EtherlikeStats_T));
            memset (&nmtr_mgr_ether_like_pause[ifindex-1], 0, sizeof (SWDRV_EtherlikePause_T));
            memset (&nmtr_mgr_rmon_stats[ifindex-1],       0, sizeof (SWDRV_RmonStats_T));
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
            memset (&nmtr_mgr_ifperq_stats[ifindex-1], 0, sizeof (SWDRV_IfPerQStats_T));
#endif
#if (SYS_CPNT_PFC == TRUE)
            memset (&nmtr_mgr_pfc_stats[ifindex-1], 0, sizeof (SWDRV_PfcStats_T));
#endif
#if (SYS_CPNT_CN == TRUE)
            memset (&nmtr_mgr_qcn_stats[ifindex-1], 0, sizeof (SWDRV_QcnStats_T));
#endif
            memset (&nmtr_mgr_utilization_stats[ifindex-1], 0, sizeof(NMTR_MGR_Utilization_T));
            memset (&nmtr_mgr_utilization_300_secs[ifindex-1], 0, sizeof(NMTR_MGR_Utilization_300_SECS_T));
            memset (&nmtr_mgr_utilization_300_secs_last_time[ifindex-1], 0, sizeof(NMTR_MGR_Utilization_300_SECS_LAST_TIME_T));

            /* clear the chip statistics 
            */
            retval = NMTRDRV_ClearPortCounter (unit, port);
            NMTR_MGR_EXIT_CRITICAL ();
            
            //SYSFUN_RELEASE_CSC();
            return retval;
        }
            
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))       
        {     
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* the chip does not support the trunk counters, need to clear all of 
             * trunk member counters 
             */
            trunk_entry.trunk_index = trunk_id;   
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();                
                return FALSE;
            }    
                
            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                if (!NMTR_MGR_ClearPortCounter(trunk_member_l_port))
                {
                    //SYSFUN_RELEASE_CSC();
                    return FALSE;
                }   
            }
            
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
               
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    } /* End of if (operation mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }
    
    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* End of NMTR_MGR_ClearPortCounter () */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearAllCounters
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the all counters in whole system
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_ClearAllCounters(void)
{
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   
    
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {      
        NMTR_MGR_InitStatistics();
        //SYSFUN_RELEASE_CSC();
        return TRUE;
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }
    
    //SYSFUN_RELEASE_CSC();
    return FALSE;    
}


/*-----------------
 * MIB2 Statistics 
 *-----------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the iftable of a interface                
 * INPUT   : UI32_T ifindex                         - interface index 
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;    

    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   
    
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {       
        if (if_table_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }    
            
        memset (if_table_stats, 0, sizeof (SWDRV_IfTableStats_T));
           
        if (NMTR_MGR_IS_VLAN(ifindex))  
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */ 
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }
            
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
        
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {        
            memcpy (if_table_stats, &nmtr_mgr_if_table_stats [ifindex-1], sizeof (SWDRV_IfTableStats_T));
            
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
            
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))       
        {     
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* the chip does not support the trunk counters, need to calculate the 
             * trunk counters 
             */
            trunk_entry.trunk_index = trunk_id;   
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
                
            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfIfTableStats (if_table_stats, nmtr_mgr_if_table_stats [trunk_member_l_port-1]);
            }
            
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
       
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }        
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }    

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetIfTableStats () */

        
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {    
        /* get the next ifindex (normal, trunk and trunk member)*/   
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
            
        retval = NMTR_MGR_GetIfTableStats (*ifindex, if_table_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
       
   //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwideStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide counters of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
void NMTR_MGR_ClearSystemwideStats(UI32_T ifindex)
{
    UI32_T                  unit = 0;
    UI32_T                  port = 0;
    UI32_T                  trunk_id = 0;
    UI32_T                  trunk_member_l_port = 0;
    SWCTRL_Lport_Type_T     port_type = 0;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {    
        if (NMTR_MGR_IS_VLAN(ifindex))  
        {
#ifdef CHIP_SUPPORT_VLAN_COUNTER
            UI32_T vid = 0;

            VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);

            if (VLAN_OM_IsVlanExisted(vid))
            {
                NMTR_MGR_ENTER_CRITICAL ();

                /* clear the statistics in NMTR_MGR
                 */
                memcpy(&nmtr_mgr_if_xtable_stats_base_for_vlan[vid-1],  &nmtr_mgr_if_xtable_stats_for_vlan[vid-1], sizeof(SWDRV_IfXTableStats_T));

                NMTR_MGR_EXIT_CRITICAL();
                
                //SYSFUN_RELEASE_CSC();
                return;
            }
#else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return;
#endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }
    
        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
    
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {        
            NMTR_MGR_ENTER_CRITICAL ();    

            /* clear the statistics in NMTR_MGR 
             */
            memcpy(&nmtr_mgr_if_table_stats_base[ifindex-1],   &nmtr_mgr_if_table_stats[ifindex-1], sizeof(SWDRV_IfTableStats_T));
            nmtr_mgr_if_table_stats_base[ifindex-1].ifOutQLen = 0; /* always set base to 0 */

            memcpy(&nmtr_mgr_if_xtable_stats_base[ifindex-1],  &nmtr_mgr_if_xtable_stats[ifindex-1], sizeof(SWDRV_IfXTableStats_T));

            /* Record port counter clear time
             */
            SYS_TIME_GetSystemUpTimeByTick(&(nmtr_mgr_if_xtable_stats_base[ifindex-1].ifCounterDiscontinuityTime));

            memcpy(&nmtr_mgr_ether_like_stats_base[ifindex-1], &nmtr_mgr_ether_like_stats[ifindex-1], sizeof(SWDRV_EtherlikeStats_T));
            memcpy(&nmtr_mgr_ether_like_pause_base[ifindex-1], &nmtr_mgr_ether_like_pause[ifindex-1], sizeof(SWDRV_EtherlikePause_T));
            memcpy(&nmtr_mgr_rmon_stats_base[ifindex-1],       &nmtr_mgr_rmon_stats[ifindex-1], sizeof(SWDRV_RmonStats_T));
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
            memcpy (&nmtr_mgr_ifperq_stats_base[ifindex-1],   &nmtr_mgr_ifperq_stats[ifindex-1], sizeof (SWDRV_IfPerQStats_T));
#endif
#if (SYS_CPNT_PFC == TRUE) && 0 /* move to NMTR_MGR_ClearSystemwidePfcStats */
            memcpy (&nmtr_mgr_pfc_stats_base[ifindex-1],   &nmtr_mgr_pfc_stats[ifindex-1], sizeof (SWDRV_PfcStats_T));
#endif
#if (SYS_CPNT_CN == TRUE)
            memcpy (&nmtr_mgr_qcn_stats_base[ifindex-1],   &nmtr_mgr_qcn_stats[ifindex-1], sizeof (SWDRV_QcnStats_T));
#endif

            NMTR_MGR_EXIT_CRITICAL();
            
            //SYSFUN_RELEASE_CSC();
            return;
        }
            
        else if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))       
        {     
#ifdef CHIP_SUPPORT_TRUNK_COUNTER
#else  
            /* the chip does not support the trunk counters, need to clear all of 
             * trunk member counters 
             */
            trunk_entry.trunk_index = trunk_id;

            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();                
                return;
            }    

            /* Record trunk counter clear time
             */
            SYS_TIME_GetSystemUpTimeByTick(&(nmtr_mgr_if_xtable_stats_base[ifindex-1].ifCounterDiscontinuityTime));

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_ClearSystemwideStats(trunk_member_l_port);
            }
			
           
            //SYSFUN_RELEASE_CSC();
            return;
#endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
        else if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    } /* End of if (operation mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }
    
    //SYSFUN_RELEASE_CSC();
}

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_ClearSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will clear the systemwide PFC counters of a interface
 * INPUT   : ifindex - interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTR_MGR_ClearSystemwidePfcStats(UI32_T ifindex)
{
    UI32_T                  unit = 0;
    UI32_T                  port = 0;
    UI32_T                  trunk_id = 0;
    UI32_T                  trunk_member_l_port = 0;
    SWCTRL_Lport_Type_T     port_type = 0;
    TRK_MGR_TrunkEntry_T    trunk_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_ENTER_CRITICAL ();

            memcpy (&nmtr_mgr_pfc_stats_base[ifindex-1],   &nmtr_mgr_pfc_stats[ifindex-1], sizeof (SWDRV_PfcStats_T));

            NMTR_MGR_EXIT_CRITICAL();

            return;
        }

        else if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
#ifdef CHIP_SUPPORT_TRUNK_COUNTER
#else
            /* the chip does not support the trunk counters, need to clear all of
             * trunk member counters
             */
            trunk_entry.trunk_index = trunk_id;

            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                return;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_ClearSystemwidePfcStats(trunk_member_l_port);
            }

            return;
#endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */
        else if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    } /* End of if (operation mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }
}
#endif /* (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaIfCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide iftable of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaIfCounter(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats)
{
    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
        return;
    if_table_stats->ifInOctets        = nmtr_mgr_if_table_stats[ifindex-1].ifInOctets - nmtr_mgr_if_table_stats_base[ifindex-1].ifInOctets;
    if_table_stats->ifInUcastPkts     = nmtr_mgr_if_table_stats[ifindex-1].ifInUcastPkts - nmtr_mgr_if_table_stats_base[ifindex-1].ifInUcastPkts;
    if_table_stats->ifInNUcastPkts    = nmtr_mgr_if_table_stats[ifindex-1].ifInNUcastPkts - nmtr_mgr_if_table_stats_base[ifindex-1].ifInNUcastPkts;
    if_table_stats->ifInDiscards      = nmtr_mgr_if_table_stats[ifindex-1].ifInDiscards - nmtr_mgr_if_table_stats_base[ifindex-1].ifInDiscards;
    if_table_stats->ifInErrors        = nmtr_mgr_if_table_stats[ifindex-1].ifInErrors - nmtr_mgr_if_table_stats_base[ifindex-1].ifInErrors;
    if_table_stats->ifInUnknownProtos = nmtr_mgr_if_table_stats[ifindex-1].ifInUnknownProtos - nmtr_mgr_if_table_stats_base[ifindex-1].ifInUnknownProtos;
    if_table_stats->ifOutOctets       = nmtr_mgr_if_table_stats[ifindex-1].ifOutOctets - nmtr_mgr_if_table_stats_base[ifindex-1].ifOutOctets;
    if_table_stats->ifOutUcastPkts    = nmtr_mgr_if_table_stats[ifindex-1].ifOutUcastPkts - nmtr_mgr_if_table_stats_base[ifindex-1].ifOutUcastPkts;
    if_table_stats->ifOutNUcastPkts   = nmtr_mgr_if_table_stats[ifindex-1].ifOutNUcastPkts - nmtr_mgr_if_table_stats_base[ifindex-1].ifOutNUcastPkts;
    if_table_stats->ifOutDiscards     = nmtr_mgr_if_table_stats[ifindex-1].ifOutDiscards - nmtr_mgr_if_table_stats_base[ifindex-1].ifOutDiscards;
    if_table_stats->ifOutErrors       = nmtr_mgr_if_table_stats[ifindex-1].ifOutErrors - nmtr_mgr_if_table_stats_base[ifindex-1].ifOutErrors;
    if_table_stats->ifOutQLen         = nmtr_mgr_if_table_stats[ifindex-1].ifOutQLen - nmtr_mgr_if_table_stats_base[ifindex-1].ifOutQLen;
    strncpy(if_table_stats->ifDateTime, nmtr_mgr_if_table_stats[ifindex-1].ifDateTime, sizeof(if_table_stats->ifDateTime));

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide iftable of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideIfTableStats(UI32_T ifindex, SWDRV_IfTableStats_T *if_table_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    SWDRV_IfTableStats_T    if_table_stats_trunk;

    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (if_table_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (if_table_stats, 0, sizeof(SWDRV_IfTableStats_T));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_GetDeltaIfCounter(ifindex, if_table_stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaIfCounter(trunk_member_l_port, &if_table_stats_trunk);
                NMTR_MGR_SumOfIfTableStats(if_table_stats, if_table_stats_trunk);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetIfTableStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           SWDRV_IfTableStats_T *if_table_stats   - iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideIfTableStats(UI32_T *ifindex, SWDRV_IfTableStats_T *if_table_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwideIfTableStats(*ifindex, if_table_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}


/*--------------------------
 * Extended MIB2 Statistics                                 
 *--------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfXTableStats                                 
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended iftable a interface           
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;    
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (if_xtable_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
    
        memset (if_xtable_stats, 0, sizeof (SWDRV_IfXTableStats_T));
        
        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            UI32_T vid;

            VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);

            if (VLAN_OM_IsVlanExisted(vid))
            {
                memcpy (if_xtable_stats, &nmtr_mgr_if_xtable_stats_for_vlan[vid-1], sizeof (SWDRV_IfXTableStats_T));

                //SYSFUN_RELEASE_CSC();
                return TRUE;
            }
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }
           
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
    
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        { 
            memcpy (if_xtable_stats, &nmtr_mgr_if_xtable_stats [ifindex-1], sizeof (SWDRV_IfXTableStats_T));    
            
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
            
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {   
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* the chip does not support the trunk counters, need to calculate the 
               trunk counters 
             */  
            trunk_entry.trunk_index = trunk_id;            
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
                
            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfIfXTableStats (if_xtable_stats, nmtr_mgr_if_xtable_stats [trunk_member_l_port-1]);  
            }    
            
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
               
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }        
    }/* if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }     
    
    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetIfXTableStats () */



/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfXTableStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {       
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
       
        retval = NMTR_MGR_GetIfXTableStats (*ifindex, if_xtable_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }     
    
    //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaIfXCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide ifXtable of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port, trunk port or vid
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaIfXCounter(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats)
{
    SWDRV_IfXTableStats_T *current_stats_p, *base_stats_p;
    UI64_T temp;

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
    if (NMTR_MGR_IS_VLAN(ifindex))
    {
        UI32_T vid;

        VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);
        current_stats_p = &nmtr_mgr_if_xtable_stats_for_vlan[vid-1];
        base_stats_p = &nmtr_mgr_if_xtable_stats_base_for_vlan[vid-1];
    }
    else
#endif
    if (ifindex >= 1 && ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        current_stats_p = &nmtr_mgr_if_xtable_stats[ifindex-1];
        base_stats_p = &nmtr_mgr_if_xtable_stats_base[ifindex-1];
    }
    else
    {
        return;
    }

    if_xtable_stats->ifInMulticastPkts  = current_stats_p->ifInMulticastPkts  - base_stats_p->ifInMulticastPkts;
    if_xtable_stats->ifInBroadcastPkts  = current_stats_p->ifInBroadcastPkts  - base_stats_p->ifInBroadcastPkts;
    if_xtable_stats->ifOutMulticastPkts = current_stats_p->ifOutMulticastPkts - base_stats_p->ifOutMulticastPkts;
    if_xtable_stats->ifOutBroadcastPkts = current_stats_p->ifOutBroadcastPkts - base_stats_p->ifOutBroadcastPkts;

    if_xtable_stats->ifCounterDiscontinuityTime = base_stats_p->ifCounterDiscontinuityTime;

    /* Remark:  (h1, l1) -= (h2, l2);
     *
     * void L_STDLIB_UI64_Sub ( UI32_T *h1, UI32_T *l1,  UI32_T h2, UI32_T l2);
     */
    #define SUB_UI64_T(FIELD)                          \
    temp = current_stats_p->FIELD;  \
    NMTR_MGR_UI64_Sub(&temp, base_stats_p->FIELD); \
    if_xtable_stats->FIELD = temp;

    SUB_UI64_T(ifHCInOctets);
    SUB_UI64_T(ifHCInUcastPkts);
    SUB_UI64_T(ifHCInMulticastPkts);
    SUB_UI64_T(ifHCInBroadcastPkts);

    SUB_UI64_T(ifHCOutOctets);
    SUB_UI64_T(ifHCOutUcastPkts);
    SUB_UI64_T(ifHCOutMulticastPkts);
    SUB_UI64_T(ifHCOutBroadcastPkts);

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the extended systemwide iftable a interface
 * INPUT   : UI32_T ifindex                          - interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideIfXTableStats(UI32_T ifindex, SWDRV_IfXTableStats_T *if_xtable_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;
    SWDRV_IfXTableStats_T   if_xtable_stats_trunk;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (if_xtable_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset(if_xtable_stats, 0, sizeof (SWDRV_IfXTableStats_T));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
#ifdef CHIP_SUPPORT_VLAN_COUNTER
            NMTR_MGR_GetDeltaIfXCounter(ifindex, if_xtable_stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
#else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
#endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_GetDeltaIfXCounter(ifindex, if_xtable_stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
        else if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
#ifdef CHIP_SUPPORT_TRUNK_COUNTER
#else
            SWDRV_IfXTableStats_T temp;

            memset(&temp, 0, sizeof(SWDRV_IfXTableStats_T));

            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;

            if (FALSE == TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_GetDeltaIfXCounter(trunk_member_l_port, &if_xtable_stats_trunk);
                NMTR_MGR_SumOfIfXTableStats (if_xtable_stats, if_xtable_stats_trunk);
            }

			NMTR_MGR_GetDeltaIfXCounter(ifindex, &temp);

			if_xtable_stats->ifCounterDiscontinuityTime = temp.ifCounterDiscontinuityTime;

            //SYSFUN_RELEASE_CSC();
            return TRUE;
#endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetIfXTableStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemIfXTableStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next extended systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                         - interface index
 *           UI32_T *ifindex                         - the next interface index
 * OUTPUT  : SWDRV_IfXTableStats_T *if_xtable_stats  - extended iftable structure
 * RETURN  : BOOL_T                                  - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideIfXTableStats(UI32_T *ifindex, SWDRV_IfXTableStats_T *if_xtable_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwideIfXTableStats(*ifindex, if_xtable_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}


/*---------------------------
 * Ether-like MIB Statistics 
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    Port_Info_T             port_info;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;    
    UI32_T                  is_giga = 0;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {       
        if (ether_like_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
    
        memset (ether_like_stats, 0, sizeof (SWDRV_EtherlikeStats_T));
                    
        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }    
        
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
    
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        { 
            memcpy (ether_like_stats, &nmtr_mgr_ether_like_stats [ifindex-1], sizeof (SWDRV_EtherlikeStats_T));
            
            if (SWCTRL_GetPortInfo(ifindex, &port_info))
            {
                if (NMTR_MGR_IS_HALF_DUPLEX ())
                {
                   (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_halfDuplex;
                }
                else if (NMTR_MGR_IS_FULL_DUPLEX ()) 
                {
                    (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_fullDuplex;
                }
                else
                {
                    (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_unknown;
                }

                /* Added for RFC 3635
                 * All MAC in current project support rate limit therefore
                 * rate limit ability depend on whether rate limit sys_cpnt is enabled.
                 * If either ingress or egree rate limit is enabled, status => TRUE
                 * tc_wang 050819
                 */
                if(port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex1000 ||
                port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000 ||
                port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10g ||
                port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex10g)
                    is_giga = 1;
                if((SYS_CPNT_INGRESS_RATE_LIMIT || SYS_CPNT_EGRESS_RATE_LIMIT) && is_giga)
                    (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_true;
                else
                    (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_false;

                if ( (*ether_like_stats).dot3StatsRateControlAbility == VAL_dot3StatsRateControlAbility_true)
                {
                    if(((port_info.ingress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_INGRESS_RATE_LIMIT)||
                       ((port_info.egress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_EGRESS_RATE_LIMIT))
                        (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOn;
                    else
                        (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
                }
                else
                {
                    (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
                }
            }
    
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
              
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {     
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* the chip does not support the trunk counters, need to calculate the 
               trunk counters 
             */  
            trunk_entry.trunk_index = trunk_id;            
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
                
            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfEtherLikeStats (ether_like_stats, nmtr_mgr_ether_like_stats [trunk_member_l_port-1]);  
            }    
            
            if (SWCTRL_GetPortInfo(ifindex, &port_info))
            {
                if (NMTR_MGR_IS_HALF_DUPLEX ())
                {
                    (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_halfDuplex;
                }
                else if (NMTR_MGR_IS_FULL_DUPLEX ()) 
                {
                    (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_fullDuplex;
                }
                else
                {
                    (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_unknown;
                }

                /* Added for RFC 3635
                 * All MAC in current project support rate limit therefore
                 * rate limit ability depend on whether rate limit sys_cpnt is enabled.
                 * If either ingress or egree rate limit is enabled, status => TRUE
                 * tc_wang 050819
                 */
                if(port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex1000 ||
                port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000 ||
                port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10g ||
                port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex10g)
                    is_giga = 1;

                if((SYS_CPNT_INGRESS_RATE_LIMIT || SYS_CPNT_EGRESS_RATE_LIMIT) && is_giga)
                    (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_true;
                else
                    (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_false;
 
                if ( (*ether_like_stats).dot3StatsRateControlAbility == VAL_dot3StatsRateControlAbility_true)
                {
                    if(((port_info.ingress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_INGRESS_RATE_LIMIT)||
                       ((port_info.egress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_EGRESS_RATE_LIMIT))
                        (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOn;
                    else
                        (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
                }
                else
                {
                    (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
                }
            }
    
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
               
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
    
    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetEtherLikeStats () */

        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextEtherLikeStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
       
        retval = NMTR_MGR_GetEtherLikeStats (*ifindex, ether_like_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
        
    //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaEtherLikeCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide Ether Like counter of a interface
 * INPUT   : UI32_T ifindex                         - interface index
* OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaEtherLikeCounter(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats)
{
    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
    	return;

    ether_like_stats->dot3StatsAlignmentErrors           = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsAlignmentErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsAlignmentErrors;
    ether_like_stats->dot3StatsFCSErrors                 = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsFCSErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsFCSErrors;
    ether_like_stats->dot3StatsSingleCollisionFrames     = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsSingleCollisionFrames - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsSingleCollisionFrames;
    ether_like_stats->dot3StatsMultipleCollisionFrames   = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsMultipleCollisionFrames - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsMultipleCollisionFrames;
    ether_like_stats->dot3StatsSQETestErrors             = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsSQETestErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsSQETestErrors;
    ether_like_stats->dot3StatsDeferredTransmissions     = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsDeferredTransmissions - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsDeferredTransmissions;
    ether_like_stats->dot3StatsLateCollisions            = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsLateCollisions - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsLateCollisions;
    ether_like_stats->dot3StatsExcessiveCollisions       = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsExcessiveCollisions - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsExcessiveCollisions;
    ether_like_stats->dot3StatsInternalMacTransmitErrors = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsInternalMacTransmitErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsInternalMacTransmitErrors;
    ether_like_stats->dot3StatsCarrierSenseErrors        = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsCarrierSenseErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsCarrierSenseErrors;
    ether_like_stats->dot3StatsFrameTooLongs             = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsFrameTooLongs - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsFrameTooLongs;
    ether_like_stats->dot3StatsInternalMacReceiveErrors  = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsInternalMacReceiveErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsInternalMacReceiveErrors;
    ether_like_stats->dot3StatsSymbolErrors              = nmtr_mgr_ether_like_stats[ifindex-1].dot3StatsSymbolErrors - nmtr_mgr_ether_like_stats_base[ifindex-1].dot3StatsSymbolErrors;

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideEtherLikeStats(UI32_T ifindex, SWDRV_EtherlikeStats_T *ether_like_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    Port_Info_T             port_info;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;
    SWDRV_EtherlikeStats_T  ether_like_stats_trunk;
    UI32_T                  is_giga = 0;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (ether_like_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset(ether_like_stats, 0, sizeof(SWDRV_EtherlikeStats_T));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
#ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
#else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
#endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {

            NMTR_MGR_GetDeltaEtherLikeCounter(ifindex, ether_like_stats);

            if (!SWCTRL_GetPortInfo(ifindex, &port_info))
            {
                return FALSE;
            }

            if (NMTR_MGR_IS_HALF_DUPLEX ())
            {
               (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_halfDuplex;
            }
            else if (NMTR_MGR_IS_FULL_DUPLEX ())
            {
                (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_fullDuplex;
            }
            else
            {
                (*ether_like_stats).dot3StatsDuplexStatus=VAL_dot3StatsDuplexStatus_unknown;
            }

            /* Added for RFC 3635
             * All MAC in current project support rate limit therefore
             * rate limit ability depend on whether rate limit sys_cpnt is enabled.
             * If either ingress or egree rate limit is enabled, status => TRUE
             * tc_wang 050819
             */
            if(port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex1000 ||
            port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000 ||
            port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10g ||
            port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex10g)
                is_giga = 1;
            if((SYS_CPNT_INGRESS_RATE_LIMIT || SYS_CPNT_EGRESS_RATE_LIMIT) && is_giga)
                (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_true;
            else
                (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_false;

            if ( (*ether_like_stats).dot3StatsRateControlAbility == VAL_dot3StatsRateControlAbility_true)
            {
                if(((port_info.ingress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_INGRESS_RATE_LIMIT)||
                   ((port_info.egress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_EGRESS_RATE_LIMIT))
                    (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOn;
                else
                    (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
            }
            else
            {
                (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
        else if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
#ifdef CHIP_SUPPORT_TRUNK_COUNTER
#else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;

            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaEtherLikeCounter(trunk_member_l_port, &ether_like_stats_trunk);
                NMTR_MGR_SumOfEtherLikeStats(ether_like_stats, ether_like_stats_trunk);
            }

            if (!SWCTRL_GetPortInfo(ifindex, &port_info))
            {
                return FALSE;
            }

            if (NMTR_MGR_IS_HALF_DUPLEX())
            {
                (*ether_like_stats).dot3StatsDuplexStatus = VAL_dot3StatsDuplexStatus_halfDuplex;
            }
            else if (NMTR_MGR_IS_FULL_DUPLEX())
            {
                (*ether_like_stats).dot3StatsDuplexStatus = VAL_dot3StatsDuplexStatus_fullDuplex;
            }
            else
            {
                (*ether_like_stats).dot3StatsDuplexStatus = VAL_dot3StatsDuplexStatus_unknown;
            }

            /* Added for RFC 3635
             * All MAC in current project support rate limit therefore
             * rate limit ability depend on whether rate limit sys_cpnt is enabled.
             * If either ingress or egree rate limit is enabled, status => TRUE
             * tc_wang 050819
             */
            if(port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex1000 ||
            port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000 ||
            port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10g ||
            port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex10g)
                is_giga = 1;

            if((SYS_CPNT_INGRESS_RATE_LIMIT || SYS_CPNT_EGRESS_RATE_LIMIT) && is_giga)
                (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_true;
            else
                (*ether_like_stats).dot3StatsRateControlAbility = VAL_dot3StatsRateControlAbility_false;

            if ( (*ether_like_stats).dot3StatsRateControlAbility == VAL_dot3StatsRateControlAbility_true)
            {
                if(((port_info.ingress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_INGRESS_RATE_LIMIT)||
                   ((port_info.egress_rate_limit_status == VAL_rlPortInputStatus_enabled) && SYS_CPNT_EGRESS_RATE_LIMIT))
                    (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOn;
                else
                    (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
            }
            else
            {
                (*ether_like_stats).dot3StatsRateControlStatus = VAL_dot3StatsRateControlStatus_rateControlOff;
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
#endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */
        else if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetEtherLikeStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideEtherLikeStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikeStats_T *ether_like_stats  - ether-like structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideEtherLikeStats(UI32_T *ifindex, SWDRV_EtherlikeStats_T *ether_like_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwideEtherLikeStats (*ifindex, ether_like_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}


/*---------------------------
 * Ether-like MIB Pause Stats
 *---------------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikePauseMode
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like pause admin/oper mode
 * INPUT   : UI32_T ifindex                            - interface index
 *           SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_GetEtherLikePauseMode(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause)
{
    Port_Info_T port_info;
    UI32_T expected_oper_mode = 0;

    if (!SWCTRL_GetPortInfo(ifindex, &port_info))
        return FALSE;

    switch (port_info.flow_control_cfg)
    {
        case VAL_portFlowCtrlCfg_enabled:
            ether_like_pause->dot3PauseAdminMode = VAL_dot3PauseAdminMode_enabledXmitAndRcv;
            expected_oper_mode = VAL_dot3PauseOperMode_enabledXmitAndRcv;
            break;
        case VAL_portFlowCtrlCfg_disabled:
            ether_like_pause->dot3PauseAdminMode = VAL_dot3PauseAdminMode_disabled;
            expected_oper_mode = VAL_dot3PauseOperMode_disabled;
            break;
        case VAL_portFlowCtrlCfg_tx:
            ether_like_pause->dot3PauseAdminMode = VAL_dot3PauseAdminMode_enabledXmit;
            expected_oper_mode = VAL_dot3PauseOperMode_enabledXmit;
            break;
        case VAL_portFlowCtrlCfg_rx:
            ether_like_pause->dot3PauseAdminMode = VAL_dot3PauseAdminMode_enabledRcv;
            expected_oper_mode = VAL_dot3PauseOperMode_enabledRcv;
            break;
        default:
            ether_like_pause->dot3PauseAdminMode = VAL_dot3PauseAdminMode_disabled;
    }


    if (NMTR_MGR_IS_HALF_DUPLEX ())
    {
        expected_oper_mode = VAL_dot3PauseOperMode_disabled;
    }

    switch (port_info.flow_control_oper)
    {
        case VAL_portFlowCtrlStatus_dot3xFlowControl:
            ether_like_pause->dot3PauseOperMode = expected_oper_mode;
            break;
        case VAL_portFlowCtrlStatus_error:
        case VAL_portFlowCtrlStatus_backPressure:
        case VAL_portFlowCtrlStatus_none:
        default:
            ether_like_pause->dot3PauseOperMode = VAL_dot3PauseOperMode_disabled;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SetEtherLikePauseAdminMode
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will set the ether-like pause function
 * INPUT   : UI32_T ifindex                            - interface index
 *           mode                                      - VAL_dot3PauseAdminMode_disabled
 *                                                       VAL_dot3PauseAdminMode_enabledXmit
 *                                                       VAL_dot3PauseAdminMode_enabledRcv
 *                                                       VAL_dot3PauseAdminMode_enabledXmitAndRcv
 * OUTPUT  : None
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetEtherLikePauseAdminMode(UI32_T ifindex, UI32_T mode)
{
    UI32_T flow_control_cfg;
    BOOL_T ret;

    SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    } 

    if (NMTR_MGR_IS_VLAN(ifindex))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }    

    switch (mode)
    {
        case VAL_dot3PauseAdminMode_disabled:
            flow_control_cfg = VAL_portFlowCtrlCfg_disabled;
            break;
        case VAL_dot3PauseAdminMode_enabledXmit:
            flow_control_cfg = VAL_portFlowCtrlCfg_tx;
            break;
        case VAL_dot3PauseAdminMode_enabledRcv:
            flow_control_cfg = VAL_portFlowCtrlCfg_rx;
            break;
        case VAL_dot3PauseAdminMode_enabledXmitAndRcv:
            flow_control_cfg = VAL_portFlowCtrlCfg_enabled;
            break;
        default:
            return FALSE;
    }

    ret = SWCTRL_PMGR_SetPortCfgFlowCtrlEnable(ifindex, flow_control_cfg);

    SYSFUN_RELEASE_CSC();
    return ret;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like pause statistics of a interface                
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;    
    
    /* BODY 
     */
    SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {       
        if (ether_like_pause == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
    
        memset (ether_like_pause, 0, sizeof (SWDRV_EtherlikePause_T));
                    
        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }    
        
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
    
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        { 
            memcpy (ether_like_pause, &nmtr_mgr_ether_like_pause [ifindex-1], sizeof (SWDRV_EtherlikePause_T));

            NMTR_MGR_GetEtherLikePauseMode(ifindex, ether_like_pause);
    
            SYSFUN_RELEASE_CSC();
            return TRUE;
        }
              
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {     
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* the chip does not support the trunk counters, need to calculate the 
               trunk counters 
             */  
            trunk_entry.trunk_index = trunk_id;            
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
                
            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfEtherLikePause (ether_like_pause, nmtr_mgr_ether_like_pause [trunk_member_l_port-1]);  
            }    
            
            NMTR_MGR_GetEtherLikePauseMode(ifindex, ether_like_pause);
   
            SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
               
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
    
    SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetEtherLikePause () */

        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextEtherLikePause                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index 
 *           UI32_T *ifindex                           - the next interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;
    
    /* BODY 
     */
    SYSFUN_USE_CSC(FALSE);   

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
       
        retval = NMTR_MGR_GetEtherLikePause (*ifindex, ether_like_pause);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
        
    SYSFUN_RELEASE_CSC();
    return retval;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaEtherLikePauseCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide Ether Like counter of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaEtherLikePauseCounter(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause)
{
    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
    	return;

    ether_like_pause->dot3InPauseFrames                  = nmtr_mgr_ether_like_pause[ifindex-1].dot3InPauseFrames - nmtr_mgr_ether_like_pause_base[ifindex-1].dot3InPauseFrames;
    ether_like_pause->dot3OutPauseFrames                 = nmtr_mgr_ether_like_pause[ifindex-1].dot3OutPauseFrames - nmtr_mgr_ether_like_pause_base[ifindex-1].dot3OutPauseFrames;

    return;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the ether-like systemwide pause statistics of a interface
 * INPUT   : UI32_T ifindex                            - interface index
 * OUTPUT  : SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideEtherLikePause(UI32_T ifindex, SWDRV_EtherlikePause_T *ether_like_pause)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;
    SWDRV_EtherlikePause_T  ether_like_pause_trunk;

    /* BODY
     */
    SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (ether_like_pause == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (ether_like_pause, 0, sizeof(SWDRV_EtherlikePause_T));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {

            NMTR_MGR_GetDeltaEtherLikePauseCounter(ifindex, ether_like_pause);

            NMTR_MGR_GetEtherLikePauseMode(ifindex, ether_like_pause);

            SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
               trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaEtherLikePauseCounter(trunk_member_l_port, &ether_like_pause_trunk);
                NMTR_MGR_SumOfEtherLikePause(ether_like_pause, ether_like_pause_trunk);
            }

            NMTR_MGR_GetEtherLikePauseMode(ifindex, ether_like_pause);

            SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetSystemwideEtherLikePause () */

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next ether-like pause statistic of a interface
 * INPUT   : UI32_T *ifindex                           - interface index
 * OUTPUT  : UI32_T *ifindex                           - the next interface index
 *           SWDRV_EtherlikePause_T *ether_like_pause  - ether-like pause structure
 * RETURN  : BOOL_T                                    - true: success ; false: fail
 * NOTE    : 1. RFC2665
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideEtherLikePause(UI32_T *ifindex, SWDRV_EtherlikePause_T *ether_like_pause)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        retval = NMTR_MGR_GetSystemwideEtherLikePause (*ifindex, ether_like_pause);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    SYSFUN_RELEASE_CSC();
    return retval;
}


/*---------------------
 * RMON MIB Statistics 
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON statistics of a interface                
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;    
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {      
        if (rmon_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
    
        memset (rmon_stats, 0, sizeof (SWDRV_RmonStats_T));    
                         
        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }    
        
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
    
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            memcpy (rmon_stats, &nmtr_mgr_rmon_stats [ifindex-1], sizeof (SWDRV_RmonStats_T));
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
        
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* the chip does not support the trunk counters, need to calculate the 
               trunk counters 
             */  
            trunk_entry.trunk_index = trunk_id;            
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
                
            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfRmonStats (rmon_stats, nmtr_mgr_rmon_stats [trunk_member_l_port-1]);    
            }        
            
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
              
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }        
    }/* End of if (operation_mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
        
    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetRmonStats () */

        

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextRmonStats                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index 
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
       
        retval = NMTR_MGR_GetRmonStats(*ifindex, rmon_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
        
    //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaRmonCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide Rmon counter of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : SWDRV_RmonStats_T *rmon_stats   - rmon structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaRmonCounter(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats)
{
    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
        return;
    rmon_stats->etherStatsDropEvents     = nmtr_mgr_rmon_stats[ifindex-1].etherStatsDropEvents - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsDropEvents;
    rmon_stats->etherStatsOctets         = nmtr_mgr_rmon_stats[ifindex-1].etherStatsOctets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsOctets;
    rmon_stats->etherStatsPkts           = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts;
    rmon_stats->etherStatsBroadcastPkts  = nmtr_mgr_rmon_stats[ifindex-1].etherStatsBroadcastPkts - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsBroadcastPkts;
    rmon_stats->etherStatsMulticastPkts  = nmtr_mgr_rmon_stats[ifindex-1].etherStatsMulticastPkts - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsMulticastPkts;
    rmon_stats->etherStatsCRCAlignErrors = nmtr_mgr_rmon_stats[ifindex-1].etherStatsCRCAlignErrors - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsCRCAlignErrors;
    rmon_stats->etherStatsUndersizePkts  = nmtr_mgr_rmon_stats[ifindex-1].etherStatsUndersizePkts - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsUndersizePkts;
    rmon_stats->etherStatsOversizePkts   = nmtr_mgr_rmon_stats[ifindex-1].etherStatsOversizePkts - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsOversizePkts;
    rmon_stats->etherStatsFragments      = nmtr_mgr_rmon_stats[ifindex-1].etherStatsFragments - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsFragments;
    rmon_stats->etherStatsJabbers        = nmtr_mgr_rmon_stats[ifindex-1].etherStatsJabbers - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsJabbers;
    rmon_stats->etherStatsCollisions     = nmtr_mgr_rmon_stats[ifindex-1].etherStatsCollisions - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsCollisions;
    rmon_stats->etherStatsPkts64Octets   = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts64Octets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts64Octets;
    rmon_stats->etherStatsPkts65to127Octets    = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts65to127Octets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts65to127Octets;
    rmon_stats->etherStatsPkts128to255Octets   = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts128to255Octets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts128to255Octets;
    rmon_stats->etherStatsPkts256to511Octets   = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts256to511Octets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts256to511Octets;
    rmon_stats->etherStatsPkts512to1023Octets  = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts512to1023Octets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts512to1023Octets;
    rmon_stats->etherStatsPkts1024to1518Octets = nmtr_mgr_rmon_stats[ifindex-1].etherStatsPkts1024to1518Octets - nmtr_mgr_rmon_stats_base[ifindex-1].etherStatsPkts1024to1518Octets;

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON systemwide statistics of a interface
 * INPUT   : UI32_T ifindex                 - interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideRmonStats(UI32_T ifindex, SWDRV_RmonStats_T *rmon_stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    SWDRV_RmonStats_T   rmon_stats_trunk;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (rmon_stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (rmon_stats, 0, sizeof (SWDRV_RmonStats_T));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_GetDeltaRmonCounter(ifindex, rmon_stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
               trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaRmonCounter(trunk_member_l_port, &rmon_stats_trunk);
                NMTR_MGR_SumOfRmonStats (rmon_stats, rmon_stats_trunk);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetRmonStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemRmonStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next systemwide iftable of specific interface
 * INPUT   : UI32_T *ifindex                - interface index
 *           UI32_T *ifindex                - the next interface index
 * OUTPUT  : SWDRV_RmonStats_T *rome_stats  - RMON structure
 * RETURN  : BOOL_T                         - true: success ; false: fail
 * NOTE    : 1. RFC1757
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideRmonStats(UI32_T *ifindex, SWDRV_RmonStats_T *rmon_stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwideRmonStats(*ifindex, rmon_stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}


#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*---------------------
 * CoS Queue Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (stats, 0, sizeof (*stats));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            memcpy (stats, &nmtr_mgr_ifperq_stats[ifindex-1], sizeof (*stats));

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfIfPerQStats (stats, nmtr_mgr_ifperq_stats[trunk_member_l_port-1]);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetIfPerQStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetIfPerQStats (*ifindex, stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

   //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaIfPerQCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaIfPerQCounter(UI32_T ifindex, SWDRV_IfPerQStats_T *stats)
{
    int i;

    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
        return;

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
    {
        stats->cosq[i].ifOutOctets          = nmtr_mgr_ifperq_stats[ifindex-1].cosq[i].ifOutOctets - nmtr_mgr_ifperq_stats_base[ifindex-1].cosq[i].ifOutOctets;
        stats->cosq[i].ifOutPkts            = nmtr_mgr_ifperq_stats[ifindex-1].cosq[i].ifOutPkts - nmtr_mgr_ifperq_stats_base[ifindex-1].cosq[i].ifOutPkts;
        stats->cosq[i].ifOutDiscardOctets   = nmtr_mgr_ifperq_stats[ifindex-1].cosq[i].ifOutDiscardOctets - nmtr_mgr_ifperq_stats_base[ifindex-1].cosq[i].ifOutDiscardOctets;
        stats->cosq[i].ifOutDiscardPkts     = nmtr_mgr_ifperq_stats[ifindex-1].cosq[i].ifOutDiscardPkts - nmtr_mgr_ifperq_stats_base[ifindex-1].cosq[i].ifOutDiscardPkts;
    }

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide CoS queue statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideIfPerQStats(UI32_T ifindex, SWDRV_IfPerQStats_T *stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    SWDRV_IfPerQStats_T stats_trunk;

    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (stats, 0, sizeof(*stats));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_GetDeltaIfPerQCounter(ifindex, stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaIfPerQCounter(trunk_member_l_port, &stats_trunk);
                NMTR_MGR_SumOfIfPerQStats(stats, stats_trunk);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetSystemwideIfPerQStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideIfPerQStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideIfPerQStats(UI32_T *ifindex, SWDRV_IfPerQStats_T *stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwideIfPerQStats(*ifindex, stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*---------------------
 * PFC Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetPfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (stats, 0, sizeof (*stats));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            memcpy (stats, &nmtr_mgr_pfc_stats[ifindex-1], sizeof (*stats));

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfPfcStats (stats, nmtr_mgr_pfc_stats[trunk_member_l_port-1]);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetPfcStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextPfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetPfcStats (*ifindex, stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

   //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaPfcCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaPfcCounter(UI32_T ifindex, SWDRV_PfcStats_T *stats)
{
    int i;

    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
        return;

    stats->ieee8021PfcRequests      = nmtr_mgr_pfc_stats[ifindex-1].ieee8021PfcRequests - nmtr_mgr_pfc_stats_base[ifindex-1].ieee8021PfcRequests;
    stats->ieee8021PfcIndications   = nmtr_mgr_pfc_stats[ifindex-1].ieee8021PfcIndications - nmtr_mgr_pfc_stats_base[ifindex-1].ieee8021PfcIndications;

    for (i = 0; i < 8; i++)
    {
        stats->pri[i].ieee8021PfcRequests      = nmtr_mgr_pfc_stats[ifindex-1].pri[i].ieee8021PfcRequests - nmtr_mgr_pfc_stats_base[ifindex-1].pri[i].ieee8021PfcRequests;
        stats->pri[i].ieee8021PfcIndications   = nmtr_mgr_pfc_stats[ifindex-1].pri[i].ieee8021PfcIndications - nmtr_mgr_pfc_stats_base[ifindex-1].pri[i].ieee8021PfcIndications;
    }

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide PFC statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwidePfcStats(UI32_T ifindex, SWDRV_PfcStats_T *stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    SWDRV_PfcStats_T stats_trunk;

    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (stats, 0, sizeof(*stats));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_GetDeltaPfcCounter(ifindex, stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaPfcCounter(trunk_member_l_port, &stats_trunk);
                NMTR_MGR_SumOfPfcStats(stats, stats_trunk);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetSystemwidePfcStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwidePfcStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwidePfcStats(UI32_T *ifindex, SWDRV_PfcStats_T *stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwidePfcStats(*ifindex, stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}
#endif /* (SYS_CPNT_PFC == TRUE) */


#if (SYS_CPNT_CN == TRUE)
/*---------------------
 * QCN Statistics
 *---------------------*/
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (stats, 0, sizeof (*stats));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            memcpy (stats, &nmtr_mgr_qcn_stats[ifindex-1], sizeof (*stats));

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfQcnStats (stats, nmtr_mgr_qcn_stats[trunk_member_l_port-1]);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetQcnStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetQcnStats (*ifindex, stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

   //SYSFUN_RELEASE_CSC();
    return retval;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDeltaQcnCounter
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_GetDeltaQcnCounter(UI32_T ifindex, SWDRV_QcnStats_T *stats)
{
    int i;

    if ((ifindex < 1) || (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT+1)) /*protection, should not happen*/
        return;

    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        stats->cpq[i].qcnStatsOutCnms       = nmtr_mgr_qcn_stats[ifindex-1].cpq[i].qcnStatsOutCnms - nmtr_mgr_qcn_stats_base[ifindex-1].cpq[i].qcnStatsOutCnms;
    }

    return;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the systemwide QCN statistics of a interface
 * INPUT   : UI32_T ifindex                         - interface index
 * OUTPUT  : *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetSystemwideQcnStats(UI32_T ifindex, SWDRV_QcnStats_T *stats)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;

    SWDRV_QcnStats_T stats_trunk;

    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (stats == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset (stats, 0, sizeof(*stats));

        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            NMTR_MGR_GetDeltaQcnCounter(ifindex, stats);
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* the chip does not support the trunk counters, need to calculate the
             * trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
            	NMTR_MGR_GetDeltaQcnCounter(trunk_member_l_port, &stats_trunk);
                NMTR_MGR_SumOfQcnStats(stats, stats_trunk);
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }   /* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER)*/
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end of NMTR_MGR_GetSystemwideQcnStats () */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextSystemwideQcnStats
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next sysiftable of specific interface
 * INPUT   : UI32_T *ifindex                        - interface index
 * OUTPUT  : UI32_T *ifindex                        - the next interface index
 *           *stats                                 - statistics structure
 * RETURN  : BOOL_T                                 - true: success ; false: fail
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextSystemwideQcnStats(UI32_T *ifindex, SWDRV_QcnStats_T *stats)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* get the next ifindex (normal, trunk and trunk member)*/
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        retval = NMTR_MGR_GetSystemwideQcnStats(*ifindex, stats);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return retval;
}
#endif /* (SYS_CPNT_CN == TRUE) */


/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 4 
 *         (Port Table for Transparent Bridges)                          */
/*
 *          dot1dTpPortTable OBJECT-TYPE
 *              SYNTAX  SEQUENCE OF Dot1dTpPortEntry
 *              ::= { dot1dTp 4 }
 *          dot1dTpPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortTable 1 }
 *          Dot1dTpPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPort             INTEGER,
 *                  dot1dTpPortMaxInfo      INTEGER,
 *                  dot1dTpPortInFrames     Counter,
 *                  dot1dTpPortOutFrames    Counter,
 *                  dot1dTpPortInDiscards   Counter
 *              }
 */              
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_MGR_GetDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry)
{
    SWDRV_IfTableStats_T if_table_stats;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {      
        if (tp_port_entry == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        if (NMTR_MGR_IS_VLAN(tp_port_entry->dot1d_tp_port))
        {   /* dot1d does not has vlan concept */
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
            //SYSFUN_RELEASE_CSC();
            return FALSE; /* vlan */    
        }
        if (!NMTR_MGR_GetIfTableStats((*tp_port_entry).dot1d_tp_port, &if_table_stats))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        (*tp_port_entry).dot1d_tp_port_max_info     = 1522; /* max frame size (porting from MC2) */    
        (*tp_port_entry).dot1d_tp_port_in_frames    = if_table_stats.ifInUcastPkts +
                                                      if_table_stats.ifInNUcastPkts; 
                                                      /* +if_table_stats.ifInDiscards;*/
                                                      /*kinghong removed:ES4649-ZZ-00412 remove discard.*/
        (*tp_port_entry).dot1d_tp_port_out_frames   = if_table_stats.ifOutUcastPkts +
                                                      if_table_stats.ifOutNUcastPkts;
        (*tp_port_entry).dot1d_tp_port_in_discards  = if_table_stats.ifInDiscards;
            
        //SYSFUN_RELEASE_CSC();                                                          
        return TRUE;    
    } /* End of if (operation_mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
        
    //SYSFUN_RELEASE_CSC();
    return FALSE;    
} /* End of NMTR_MGR_GetDot1dTpPortEntry () */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_port_entry -- the next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T NMTR_MGR_GetNextDot1dTpPortEntry(NMTR_MGR_Dot1dTpPortEntry_T *tp_port_entry)
{
    SWDRV_IfTableStats_T if_table_stats;

    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (tp_port_entry == 0)
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_ERROR_MEMORY); */
            EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
        if (NMTR_MGR_IS_VLAN(tp_port_entry->dot1d_tp_port))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
            //SYSFUN_RELEASE_CSC();
            return FALSE; /* vlan */    
        }
        if (!NMTR_MGR_GetNextIfTableStats(&((*tp_port_entry).dot1d_tp_port), &if_table_stats))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;    
        }
        (*tp_port_entry).dot1d_tp_port_max_info     = 1522; /* max frame size (porting from MC2) */    
        (*tp_port_entry).dot1d_tp_port_in_frames    = if_table_stats.ifInUcastPkts +
                                                      if_table_stats.ifInNUcastPkts;
                                                      /*+if_table_stats.ifInDiscards;*/
                                                      /* kinghong modify: ES4649-ZZ-00412; remove discards*/
        (*tp_port_entry).dot1d_tp_port_out_frames   = if_table_stats.ifOutUcastPkts +
                                                      if_table_stats.ifOutNUcastPkts;
        (*tp_port_entry).dot1d_tp_port_in_discards  = if_table_stats.ifInDiscards;
        
        //SYSFUN_RELEASE_CSC();                                                  
        return TRUE;        
    } /* End of if (operation_mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
    
    //SYSFUN_RELEASE_CSC();
    return FALSE;         
} /* End of NMTR_MGR_GetNextDot1dTpPortEntry () */



BOOL_T NMTR_MGR_IsProvisionComplete(void)
{
    return nmtr_mgr_provision_complete;
}



/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetPortUtilization(UI32_T ifindex, NMTR_MGR_Utilization_T *utilization_entry)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit;
    UI32_T                  port; 
    UI32_T                  trunk_id;
    UI32_T                  trunk_member_l_port = 0;   
    UI32_T                  num_of_trunk_member;
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (utilization_entry == 0)
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
            
        memset (utilization_entry, 0, sizeof (NMTR_MGR_Utilization_T));
        
        /* the logical port is vid 
         */    
        if (NMTR_MGR_IS_VLAN(ifindex))  
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */ 
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */    
        }
    
        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);
        
        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {        
            memcpy (utilization_entry, &nmtr_mgr_utilization_stats [ifindex-1], sizeof (NMTR_MGR_Utilization_T));
        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
            
        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))       
        {     
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else  
            /* Mercury: the broadcom chip does not support the trunk counters, need to calculate the 
               trunk counters 
             */
            trunk_entry.trunk_index = trunk_id;   
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            num_of_trunk_member = 0;

            while (NMTR_MGR_GetNextIndexFromPortList (&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                NMTR_MGR_SumOfUtilizationStats (utilization_entry, nmtr_mgr_utilization_stats [trunk_member_l_port-1]);   
                num_of_trunk_member++;
            }

            if (num_of_trunk_member > 1)
            {
                utilization_entry->ifInOctets_utilization /= num_of_trunk_member;
                utilization_entry->ifInUcastPkts_utilization /= num_of_trunk_member;
                utilization_entry->ifInMulticastPkts_utilization /= num_of_trunk_member;
                utilization_entry->ifInBroadcastPkts_utilization /= num_of_trunk_member;
                utilization_entry->ifInErrors_utilization /= num_of_trunk_member;
            }
            
            //SYSFUN_RELEASE_CSC();        
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */   
        }   /* end of if (trunk port) */
            
        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }        
    }/* End of if (operation_mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }     
    
    //SYSFUN_RELEASE_CSC();
    return FALSE;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPortUtilization                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get next utilization of Port statistics 
 * INPUT   : ifindex            -- interface index                       
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value                                
 * RETURN  : None                                                           
 * NOTE    : 1. RFC2863
 *           2. ifindex is physical port ,trunk port or vid
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextPortUtilization(UI32_T *ifindex, NMTR_MGR_Utilization_T *utilization_entry)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;    
    
    /* BODY 
     */
    //SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }
       
        retval = NMTR_MGR_GetPortUtilization (*ifindex, utilization_entry);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }     
    
    //SYSFUN_RELEASE_CSC();
    return retval;
}

BOOL_T NMTR_MGR_HandleUpdateNmtrdrvStats(UI32_T update_type,UI32_T unit, UI32_T start_port, UI32_T port_amount)
{
    switch (update_type)
    {
        case NMTRDRV_UPDATE_IFTABLE_STATS:
            NMTR_MGR_UpdateIFTableStats_CallBack(unit, start_port, port_amount);
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
            NMTR_MGR_UpdateHistory_CallBack(unit, start_port, port_amount);
#endif
            break;
        case NMTRDRV_UPDATE_IFXTABLE_STATS:
            NMTR_MGR_UpdateIFXTableStats_CallBack(unit, start_port, port_amount);
            break;
        case NMTRDRV_UPDATE_RMON_STATS:
            NMTR_MGR_UpdateRmonStats_CallBack(unit, start_port, port_amount);
            break;
        case NMTRDRV_UPDATE_ETHERLIKE_STATS:
            NMTR_MGR_UpdateEtherLikeStats_CallBack(unit, start_port, port_amount);
            break;
        case NMTRDRV_UPDATE_ETHERLIKE_PAUSE_STATS:
            NMTR_MGR_UpdateEtherLikePause_CallBack(unit, start_port, port_amount);
            break;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        case NMTRDRV_UPDATE_IFPERQ_STATS:
            NMTR_MGR_UpdateIfPerQStats_CallBack(unit, start_port, port_amount);
            break;
#endif
#if (SYS_CPNT_PFC == TRUE)
        case NMTRDRV_UPDATE_PFC_STATS:
            NMTR_MGR_UpdatePfcStats_CallBack(unit, start_port, port_amount);
            break;
#endif
#if (SYS_CPNT_CN == TRUE)
        case NMTRDRV_UPDATE_QCN_STATS:
            NMTR_MGR_UpdateQcnStats_CallBack(unit, start_port, port_amount);
            break;
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
        case NMTRDRV_UPDATE_IFXTABLE_STATS_FOR_VLAN:
            NMTR_MGR_UpdateIFXTableStatsForVlan_CallBack(unit, start_port, port_amount);
            break;
#endif
        case NMTRDRV_UPDATE_300S_UTILIZATION:
#if (SYS_CPNT_NMTR_SYNC_NDEV == TRUE)
            NMTR_MGR_UpdateNetDevStatistics_CallBack();
#endif
            NMTR_MGR_Update300sUtilization_CallBack();
            break;
        default:
            printf("%s: Invalid update type = %lu\n",__FUNCTION__,update_type);
            break;
    }
    return TRUE;/*added by Jinhua Wei ,to remove warning ,becaued the function didn't return a exact value for an non-void function */
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_TrunkMemberAdd1st_Callback
 *-----------------------------------------------------------------
 * FUNCTION: Handle event: when the first port is added to a trunk
 * INPUT   : trunk_ifindex
 *           member_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_MGR_TrunkMemberAdd1st_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    NMTR_HIST_FollowAttribute(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_TrunkMemberAdd_Callback
 *-----------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is added to a trunk
 * INPUT   : trunk_ifindex
 *           member_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_MGR_TrunkMemberAdd_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    NMTR_HIST_FollowAttribute(member_ifindex, trunk_ifindex);
#endif
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is deleted from a trunk
 * INPUT   : trunk_ifindex
 *           member_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_MGR_TrunkMemberDeleteLst_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    NMTR_HIST_DestroyCtrlEntryByIfindex(trunk_ifindex);
#endif

#if (SYS_CPNT_NMTR_SYNC_NDEV == TRUE)
    NMTR_MGR_ResetNetDevStatistics(trunk_ifindex);
#endif
}

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers
 *-----------------------------------------------------------------
 * FUNCTION: This function will set trunk config to all trunk members
 * INPUT   : trunk_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : do nothing if trunk_ifindex is not a trunk
 *----------------------------------------------------------------*/
static BOOL_T NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(UI32_T trunk_ifindex)
{
    TRK_MGR_TrunkEntry_T trunk_entry;
    SWCTRL_Lport_Type_T port_type;
    UI32_T unit, port, trunk_id;
    UI32_T ifindex;

    port_type = SWCTRL_LogicalPortToUserPort(trunk_ifindex, &unit, &port, &trunk_id);
    if (!NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
    {
        return TRUE;
    }

    trunk_entry.trunk_index = trunk_id;            
    if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
    {
        return FALSE;
    }

    ifindex = 0;
    while (NMTR_MGR_GetNextIndexFromPortList(&ifindex, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        if (!NMTR_HIST_FollowAttribute(ifindex, trunk_ifindex))
        {
            return FALSE;
        }
    }    

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_CalculateUtilization
 *-----------------------------------------------------------------
 * FUNCTION: To calculate utilization
 * INPUT   : trunk_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *----------------------------------------------------------------*/
static void NMTR_MGR_CalculateUtilization(NMTR_TYPE_HistCounterInfo_T *counter_p)
{
    UI64_T utilization_basis_point;

    /* utilization_basis_point
     *     = 10000 * xmit_bytes * 8 / bandwidth_bps / interval_sec
     *     = 10000 * xmit_bytes * 8 / (bandwidth_kbps * 1000) / (interval_tick / ticks_per_sec)
     *     = xmit_bytes * 8 * ticks_per_sec / interval_tick / (bandwidth_kbps / 10)
     */
    if (counter_p->interval)
    {
        UI32_T bandwidth_kbps = NMTR_MGR_HIST_UTIL_BASE_BANDWIDTH_KBPS_FOR_XMIT_BYTES_STATISTIC;

        utilization_basis_point = counter_p->ifInUtilization.xmit_bytes;
        NMTR_MGR_UI64_Multi(&utilization_basis_point, 8 * SYS_BLD_TICKS_PER_SECOND);
        NMTR_MGR_UI64_Div(&utilization_basis_point, counter_p->interval);
        NMTR_MGR_UI64_Div(&utilization_basis_point, bandwidth_kbps / 10);

        counter_p->ifInUtilization.basis_point = L_STDLIB_UI64_L32(utilization_basis_point);

        if (counter_p->ifInUtilization.basis_point > 10000)
            counter_p->ifInUtilization.basis_point = 10000;

        utilization_basis_point = counter_p->ifOutUtilization.xmit_bytes;
        NMTR_MGR_UI64_Multi(&utilization_basis_point, 8 * SYS_BLD_TICKS_PER_SECOND);
        NMTR_MGR_UI64_Div(&utilization_basis_point, counter_p->interval);
        NMTR_MGR_UI64_Div(&utilization_basis_point, bandwidth_kbps / 10);

        counter_p->ifOutUtilization.basis_point = L_STDLIB_UI64_L32(utilization_basis_point);

        if (counter_p->ifOutUtilization.basis_point > 10000)
            counter_p->ifOutUtilization.basis_point = 10000;
    }
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetDefaultHistoryCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will create default ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetDefaultHistoryCtrlEntry(UI32_T ifindex)
{
    if (!SWCTRL_LogicalPortExisting(ifindex))
    {
        return FALSE;
    }

    if (!NMTR_HIST_DestroyCtrlEntryByIfindex(ifindex))
    {
        return FALSE;
    }

    if (!NMTR_HIST_SetDefaultCtrlEntry(ifindex))
    {
        return FALSE;
    }

    if (!NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(ifindex))
    {
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a field of ctrl entry
 * INPUT   : ctrl_idx
 *           field_idx
 *           data_p    - value of field
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetHistoryCtrlEntryField(UI32_T ctrl_idx, UI32_T field_idx, void *data_p)
{
    NMTR_TYPE_HistCtrlInfo_T ctrl_info;
    UI32_T old_data_source, new_data_source;

    old_data_source = new_data_source = 0;

    /* both original ifindex and new ifindex shall be logical port.
     */
    if (field_idx == NMTR_TYPE_HIST_CTRL_FIELD_DATA_SOURCE)
    {
        if (!SWCTRL_LogicalPortExisting(L_CVRT_PTR_TO_UINT(data_p)))
        {
            return FALSE;
        }
    }

    ctrl_info.ctrl_idx = ctrl_idx;
    if (NMTR_HIST_GetCtrlEntry(&ctrl_info, FALSE))
    {
        old_data_source = ctrl_info.data_source;

        if (!SWCTRL_LogicalPortExisting(ctrl_info.data_source))
        {
            return FALSE;
        }
    }

    /* update field
     */
    if (!NMTR_HIST_SetCtrlEntryField(ctrl_idx, field_idx, data_p))
    {
        return FALSE;
    }

    /* sync to trunk members if either original ifindex or new ifindex
     * is a trunk
     */
    ctrl_info.ctrl_idx = ctrl_idx;
    if (NMTR_HIST_GetCtrlEntry(&ctrl_info, FALSE))
    {
        /* if the number of ctrl entry on new ifindex is excess,
         * destroy it.
         */
        if (NMTR_HIST_GetNumOfCtrlEntryByIfindex(ctrl_info.data_source) >
            SYS_ADPT_NMTR_HIST_MAX_NBR_OF_CTRL_ENTRY_PER_PORT)
        {
            NMTR_HIST_SetCtrlEntryField(ctrl_info.ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_STATUS, (void *)NMTR_TYPE_HIST_CTRL_STATUS_DESTROY);
            return FALSE;
        }

        new_data_source = ctrl_info.data_source;
    }

    if (old_data_source != 0)
    {
        if (!NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(new_data_source))
        {
            return FALSE;
        }
    }

    if (new_data_source != 0 && new_data_source != old_data_source)
    {
        if (!NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(old_data_source))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : ctrl_p->ctrl_idx
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    if (!SWCTRL_LogicalPortExisting(ctrl_p->data_source))
    {
        return FALSE;
    }

    if (!NMTR_HIST_SetCtrlEntryByNameAndDataSrc(ctrl_p))
    {
        return FALSE;
    }

    /* if the number of ctrl entry on new ifindex is excess,
     * destroy it.
     */
    if (NMTR_HIST_GetNumOfCtrlEntryByIfindex(ctrl_p->data_source) >
        SYS_ADPT_NMTR_HIST_MAX_NBR_OF_CTRL_ENTRY_PER_PORT)
    {
        NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc(ctrl_p);
        return FALSE;
    }

    if (!NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(ctrl_p->data_source))
    {
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    if (!SWCTRL_LogicalPortExisting(ctrl_p->data_source))
    {
        return FALSE;
    }

    if (!NMTR_HIST_DestroyCtrlEntryByNameAndDataSrc(ctrl_p))
    {
        return FALSE;
    }

    if (!NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(ctrl_p->data_source))
    {
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_DestroyHistoryCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy ctrl entries related to 
 *           specified interface
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_MGR_DestroyHistoryCtrlEntryByIfindex(UI32_T ifindex)
{
    if (!SWCTRL_LogicalPortExisting(ifindex))
    {
        return FALSE;
    }

    if (!NMTR_HIST_DestroyCtrlEntryByIfindex(ifindex))
    {
        return FALSE;
    }

    if (!NMTR_MGR_HistoryCtrlEntrySyncToTrunkMembers(ifindex))
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: NMTR_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for NMTR MGR.
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
BOOL_T NMTR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    NMTR_MGR_IPCMsg_T *msg_p;
    
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (NMTR_MGR_IPCMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE()!= SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding NMTR_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case NMTR_MGR_IPC_GET_IFTABLE_STATS:
            if (msg_p->data.GetIfStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetIfTableStats(msg_p->data.GetIfStats_data.ifindex, 
                                       &(msg_p->data.GetIfStats_data.if_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextIfTableStats(&(msg_p->data.GetIfStats_data.ifindex), 
                                       &(msg_p->data.GetIfStats_data.if_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfStats_Data_S);            
            break;
        case NMTR_MGR_IPC_GET_IFXTABLE_STATS:
            if (msg_p->data.GetIfXStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetIfXTableStats(msg_p->data.GetIfXStats_data.ifindex, 
                                       &(msg_p->data.GetIfXStats_data.ifx_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextIfXTableStats(&(msg_p->data.GetIfXStats_data.ifindex), 
                                       &(msg_p->data.GetIfXStats_data.ifx_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfXStats_Data_S);
            break;
        case NMTR_MGR_IPC_GET_ETHERLIKE_STATS:
            if (msg_p->data.GetEtherlikeStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetEtherLikeStats(msg_p->data.GetEtherlikeStats_data.ifindex, 
                                       &(msg_p->data.GetEtherlikeStats_data.etherlike_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextEtherLikeStats(&(msg_p->data.GetEtherlikeStats_data.ifindex), 
                                       &(msg_p->data.GetEtherlikeStats_data.etherlike_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S);            
            break;
        case NMTR_MGR_IPC_SET_ETHERLIKE_PAUSE_ADMIN_MODE:
            msg_p->type.ret_bool = NMTR_MGR_SetEtherLikePauseAdminMode(msg_p->data.SetEtherlikePauseAdminMode_data.ifindex,
                                   msg_p->data.SetEtherlikePauseAdminMode_data.mode);
            msgbuf_p->msg_size = NMTR_MGR_MSGBUF_TYPE_SIZE;
            break;
        case NMTR_MGR_IPC_GET_ETHERLIKE_PAUSE_STATS:
            if (msg_p->data.GetEtherlikePause_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetEtherLikePause(msg_p->data.GetEtherlikePause_data.ifindex, 
                                       &(msg_p->data.GetEtherlikePause_data.etherlike_pause));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextEtherLikePause(&(msg_p->data.GetEtherlikePause_data.ifindex), 
                                       &(msg_p->data.GetEtherlikePause_data.etherlike_pause));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S);            
            break;
        case NMTR_MGR_IPC_GET_RMON_STATS:
            if (msg_p->data.GetRmonStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetRmonStats(msg_p->data.GetRmonStats_data.ifindex, 
                                       &(msg_p->data.GetRmonStats_data.rmon_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextRmonStats(&(msg_p->data.GetRmonStats_data.ifindex), 
                                       &(msg_p->data.GetRmonStats_data.rmon_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetRmonStats_Data_S);            
            break;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        case NMTR_MGR_IPC_GET_IFPERQ_STATS:
            if (msg_p->data.GetIfPerQStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetIfPerQStats(msg_p->data.GetIfPerQStats_data.ifindex, 
                                       &(msg_p->data.GetIfPerQStats_data.stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextIfPerQStats(&(msg_p->data.GetIfPerQStats_data.ifindex), 
                                       &(msg_p->data.GetIfPerQStats_data.stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S);            
            break;
#endif
#if (SYS_CPNT_PFC == TRUE)
        case NMTR_MGR_IPC_GET_PFC_STATS:
            if (msg_p->data.GetPfcStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetPfcStats(msg_p->data.GetPfcStats_data.ifindex, 
                                       &(msg_p->data.GetPfcStats_data.stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextPfcStats(&(msg_p->data.GetPfcStats_data.ifindex), 
                                       &(msg_p->data.GetPfcStats_data.stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPfcStats_Data_S);            
            break;
#endif
#if (SYS_CPNT_CN == TRUE)
        case NMTR_MGR_IPC_GET_QCN_STATS:
            if (msg_p->data.GetQcnStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetQcnStats(msg_p->data.GetQcnStats_data.ifindex, 
                                       &(msg_p->data.GetQcnStats_data.stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextQcnStats(&(msg_p->data.GetQcnStats_data.ifindex), 
                                       &(msg_p->data.GetQcnStats_data.stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetQcnStats_Data_S);            
            break;
#endif
        case NMTR_MGR_IPC_GET_DOT1D_TP_PORT_ENTRY:
            if (msg_p->data.GetDot1dTpPortEntry_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetDot1dTpPortEntry(&(msg_p->data.GetDot1dTpPortEntry_data.tp_port_entry));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextDot1dTpPortEntry(&(msg_p->data.GetDot1dTpPortEntry_data.tp_port_entry));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpPortEntry_Data_S);             
            break;
        case NMTR_MGR_IPC_GET_DOT1D_TP_HC_PORT_ENTRY:
            if (msg_p->data.GetDot1dTpHCPortEntry_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetDot1dTpHCPortEntry(&(msg_p->data.GetDot1dTpHCPortEntry_data.tp_hc_port_entry));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextDot1dTpHCPortEntry(&(msg_p->data.GetDot1dTpHCPortEntry_data.tp_hc_port_entry));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpHCPortEntry_Data_S);             
            break;
        case NMTR_MGR_IPC_GET_DOT1D_TP_PORT_OVERFLOW_ENTRY:
            if (msg_p->data.GetDot1dTpPortOverflowEntry_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetDot1dTpPortOverflowEntry(&(msg_p->data.GetDot1dTpPortOverflowEntry_data.tp_port_overflow_entry));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextDot1dTpPortOverflowEntry(&(msg_p->data.GetDot1dTpPortOverflowEntry_data.tp_port_overflow_entry));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetDot1dTpPortOverflowEntry_Data_S);             
            break;            
        case NMTR_MGR_IPC_GET_PORT_UTILIZATION:
            if (msg_p->data.GetPortUtilization_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetPortUtilization(msg_p->data.GetRmonStats_data.ifindex, 
                                       &(msg_p->data.GetPortUtilization_data.utilization_entry));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextPortUtilization(&(msg_p->data.GetRmonStats_data.ifindex), 
                                       &(msg_p->data.GetPortUtilization_data.utilization_entry));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPortUtilization_Data_S);             
            break;
        case NMTR_MGR_IPC_CLEAR_SYSTEMWIDE_STATS:
            NMTR_MGR_ClearSystemwideStats(msg_p->data.ClearSystemwideStats_data.ifindex);
            msg_p->type.ret_bool = TRUE;
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;             
            break;            
#if (SYS_CPNT_PFC == TRUE)
        case NMTR_MGR_IPC_CLEAR_SYSTEMWIDE_PFC_STATS:
            NMTR_MGR_ClearSystemwidePfcStats(msg_p->data.ClearSystemwideStats_data.ifindex);
            msg_p->type.ret_bool = TRUE;
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;             
            break;            
#endif
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_IFTABLE_STATS:
            if (msg_p->data.GetIfStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideIfTableStats(msg_p->data.GetIfStats_data.ifindex, 
                                       &(msg_p->data.GetIfStats_data.if_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideIfTableStats(&(msg_p->data.GetIfStats_data.ifindex), 
                                       &(msg_p->data.GetIfStats_data.if_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfStats_Data_S);            
            break;        
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_IFXTABLE_STATS:
            if (msg_p->data.GetIfXStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideIfXTableStats(msg_p->data.GetIfXStats_data.ifindex, 
                                       &(msg_p->data.GetIfXStats_data.ifx_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideIfXTableStats(&(msg_p->data.GetIfXStats_data.ifindex), 
                                       &(msg_p->data.GetIfXStats_data.ifx_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfXStats_Data_S);
            break;
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_STATS:
            if (msg_p->data.GetEtherlikeStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideEtherLikeStats(msg_p->data.GetEtherlikeStats_data.ifindex, 
                                       &(msg_p->data.GetEtherlikeStats_data.etherlike_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideEtherLikeStats(&(msg_p->data.GetEtherlikeStats_data.ifindex), 
                                       &(msg_p->data.GetEtherlikeStats_data.etherlike_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikeStats_Data_S);            
            break;
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_ETHERLIKE_PAUSE_STATS:
            if (msg_p->data.GetEtherlikePause_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideEtherLikePause(msg_p->data.GetEtherlikePause_data.ifindex, 
                                       &(msg_p->data.GetEtherlikePause_data.etherlike_pause));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideEtherLikePause(&(msg_p->data.GetEtherlikePause_data.ifindex), 
                                       &(msg_p->data.GetEtherlikePause_data.etherlike_pause));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherlikePause_Data_S);            
            break;
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_RMON_STATS:
            if (msg_p->data.GetRmonStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideRmonStats(msg_p->data.GetRmonStats_data.ifindex, 
                                       &(msg_p->data.GetRmonStats_data.rmon_stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideRmonStats(&(msg_p->data.GetRmonStats_data.ifindex), 
                                       &(msg_p->data.GetRmonStats_data.rmon_stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetRmonStats_Data_S);            
            break;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_IFPERQ_STATS:
            if (msg_p->data.GetIfPerQStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideIfPerQStats(msg_p->data.GetIfPerQStats_data.ifindex, 
                                       &(msg_p->data.GetIfPerQStats_data.stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideIfPerQStats(&(msg_p->data.GetIfPerQStats_data.ifindex), 
                                       &(msg_p->data.GetIfPerQStats_data.stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetIfPerQStats_Data_S);            
            break;
#endif
#if (SYS_CPNT_PFC == TRUE)
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_PFC_STATS:
            if (msg_p->data.GetPfcStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwidePfcStats(msg_p->data.GetPfcStats_data.ifindex, 
                                       &(msg_p->data.GetPfcStats_data.stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwidePfcStats(&(msg_p->data.GetPfcStats_data.ifindex), 
                                       &(msg_p->data.GetPfcStats_data.stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPfcStats_Data_S);            
            break;
#endif
#if (SYS_CPNT_CN == TRUE)
        case NMTR_MGR_IPC_GET_SYSTEMWIDE_QCN_STATS:
            if (msg_p->data.GetQcnStats_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetSystemwideQcnStats(msg_p->data.GetQcnStats_data.ifindex, 
                                       &(msg_p->data.GetQcnStats_data.stats));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextSystemwideQcnStats(&(msg_p->data.GetQcnStats_data.ifindex), 
                                       &(msg_p->data.GetQcnStats_data.stats));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetQcnStats_Data_S);            
            break;
#endif
        case NMTR_MGR_IPC_GET_PORT_UTILIZATION_300_SECS:
            if (msg_p->data.GetPortUtilization300secs_data.next == FALSE)
            {
                msg_p->type.ret_bool = NMTR_MGR_GetPortUtilization300secs(msg_p->data.GetPortUtilization300secs_data.ifindex, 
                                           &(msg_p->data.GetPortUtilization300secs_data.utilization_entry));
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_GetNextPortUtilization300secs(&(msg_p->data.GetPortUtilization300secs_data.ifindex),
                                           &(msg_p->data.GetPortUtilization300secs_data.utilization_entry));
            }
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetPortUtilization300secs_Data_S);            
            break;
        case NMTR_MGR_IPC_GET_ETHER_STATS_HIGH_CAPACITY_ENTRY:
            msg_p->type.ret_bool = NMTR_MGR_GetEtherStatsHighCapacityEntry(&(msg_p->data.GetEtherStatsHighCapacityEntry_data.es_hc_entry));
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_GetEtherStatsHighCapacityEntry_Data_S);
            break;
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
        case NMTR_MGR_IPC_SET_DEFAULT_HISTORY_CTRL_ENTRY:
            msg_p->type.ret_bool = NMTR_MGR_SetDefaultHistoryCtrlEntry(
                msg_p->data.HistoryCtrlEntry_data.entry.data_source);
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
        case NMTR_MGR_IPC_SET_HISTORY_CTRL_ENTRY_FIELD:
            if (msg_p->data.HistoryCtrlEntryField_data.field_idx == NMTR_TYPE_HIST_CTRL_FIELD_NAME)
            {
                msg_p->type.ret_bool = NMTR_MGR_SetHistoryCtrlEntryField(
                    msg_p->data.HistoryCtrlEntryField_data.ctrl_idx,
                    msg_p->data.HistoryCtrlEntryField_data.field_idx,
                    msg_p->data.HistoryCtrlEntryField_data.data.s);
            }
            else
            {
                msg_p->type.ret_bool = NMTR_MGR_SetHistoryCtrlEntryField(
                    msg_p->data.HistoryCtrlEntryField_data.ctrl_idx,
                    msg_p->data.HistoryCtrlEntryField_data.field_idx,
                    L_CVRT_UINT_TO_PTR(msg_p->data.HistoryCtrlEntryField_data.data.i));
            }
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
        case NMTR_MGR_IPC_SET_HISTORY_CTRL_ENTRY_BY_NAME_AND_DATASRC:
            msg_p->type.ret_bool = NMTR_MGR_SetHistoryCtrlEntryByNameAndDataSrc(
                &msg_p->data.HistoryCtrlEntry_data.entry);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
            break;
        case NMTR_MGR_IPC_DESTROY_HISTORY_CTRL_ENTRY_BY_NAME_AND_DATASRC:
            msg_p->type.ret_bool = NMTR_MGR_DestroyHistoryCtrlEntryByNameAndDataSrc(
                &msg_p->data.HistoryCtrlEntry_data.entry);
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
        case NMTR_MGR_IPC_DESTROY_HISTORY_CTRL_ENTRY_BY_IFINDEX:
            msg_p->type.ret_bool = NMTR_MGR_DestroyHistoryCtrlEntryByIfindex(
                msg_p->data.HistoryCtrlEntry_data.entry.data_source);
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
        case NMTR_MGR_IPC_GET_HISTORY_CTRL_ENTRY:
            msg_p->type.ret_bool = NMTR_HIST_GetCtrlEntry(
                &msg_p->data.HistoryCtrlEntry_data.entry,
                msg_p->data.HistoryCtrlEntry_data.next);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
            break;
        case NMTR_MGR_IPC_GET_HISTORY_CTRL_ENTRY_BY_DATASRC:
            msg_p->type.ret_bool = NMTR_HIST_GetCtrlEntryByDataSrc(
                &msg_p->data.HistoryCtrlEntry_data.entry,
                msg_p->data.HistoryCtrlEntry_data.next);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
            break;
        case NMTR_MGR_IPC_GET_HISTORY_CURRENT_ENTRY:
            msg_p->type.ret_bool = NMTR_HIST_GetCurrentEntry(
                &msg_p->data.HistorySampleEntry_data.entry,
                msg_p->data.HistorySampleEntry_data.next);
            NMTR_MGR_CalculateUtilization(&msg_p->data.HistorySampleEntry_data.entry.counter);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S);
            break;
        case NMTR_MGR_IPC_GET_HISTORY_PREVIOUS_ENTRY:
            msg_p->type.ret_bool = NMTR_HIST_GetPreviousEntry(
                &msg_p->data.HistorySampleEntry_data.entry,
                msg_p->data.HistorySampleEntry_data.next);
            NMTR_MGR_CalculateUtilization(&msg_p->data.HistorySampleEntry_data.entry.counter);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S);
            break;
        case NMTR_MGR_IPC_GET_NEXT_HISTORY_PREVIOUS_ENTRY_BY_CTRL_IDX:
            msg_p->type.ret_bool = NMTR_HIST_GetNextPreviousEntryByCtrlIdx(
                &msg_p->data.HistorySampleEntry_data.entry);
            NMTR_MGR_CalculateUtilization(&msg_p->data.HistorySampleEntry_data.entry.counter);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistorySampleEntry_Data_S);
            break;
        case NMTR_MGR_IPC_GET_NEXT_REMOVED_DEFAULT_HISTORY_CTRL_ENTRY_BY_DATASRC:
            msg_p->type.ret_bool = NMTR_HIST_GetNextRemovedDefaultCtrlEntryByDataSrc(
                &msg_p->data.HistoryCtrlEntry_data.entry);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
            break;
        case NMTR_MGR_IPC_GET_NEXT_USER_CFG_HISTORY_CTRL_ENTRY_BY_DATASRC:
            msg_p->type.ret_bool = NMTR_HIST_GetNextUserCfgCtrlEntryByDataSrc(
                &msg_p->data.HistoryCtrlEntry_data.entry);
            msgbuf_p->msg_size = NMTR_MGR_GET_MSGBUFSIZE(NMTR_MGR_IPCMsg_HistoryCtrlEntry_Data_S);
            break;
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = NMTR_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of NMTR_MGR_HandleIPCReqMsg */

/*-------------------------
 * LOCAL SUBPROGRAM BODIES
 *-------------------------*/

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_Init_Statistics                                      
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will initializet statistics                      
 * INPUT   : None                                                           
 * OUTPUT  : None                                                           
 * RETURN  : None                                                           
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_InitStatistics(void)
{
    NMTR_MGR_ENTER_CRITICAL();
    /* clear all statistics counters ( in memory )
     */
    memset(nmtr_mgr_if_table_stats,   0, sizeof(nmtr_mgr_if_table_stats));
    memset(nmtr_mgr_if_table_stats_base, 0, sizeof(nmtr_mgr_if_table_stats_base));

    memset(nmtr_mgr_if_xtable_stats,  0, sizeof(nmtr_mgr_if_xtable_stats));
    memset(nmtr_mgr_if_xtable_stats_base, 0, sizeof(nmtr_mgr_if_xtable_stats_base));

    memset(nmtr_mgr_ether_like_stats, 0, sizeof(nmtr_mgr_ether_like_stats));
    memset(nmtr_mgr_ether_like_stats_base, 0, sizeof(nmtr_mgr_ether_like_stats_base));
   
    memset(nmtr_mgr_ether_like_pause, 0, sizeof(nmtr_mgr_ether_like_pause));
    memset(nmtr_mgr_ether_like_pause_base, 0, sizeof(nmtr_mgr_ether_like_pause_base));

    memset(nmtr_mgr_rmon_stats,       0, sizeof(nmtr_mgr_rmon_stats));
    memset(nmtr_mgr_rmon_stats_base, 0, sizeof(nmtr_mgr_rmon_stats_base));

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    memset(nmtr_mgr_ifperq_stats,   0, sizeof(nmtr_mgr_ifperq_stats));
    memset(nmtr_mgr_ifperq_stats_base, 0, sizeof(nmtr_mgr_ifperq_stats_base));
#endif

#if (SYS_CPNT_PFC == TRUE)
    memset(nmtr_mgr_pfc_stats,   0, sizeof(nmtr_mgr_pfc_stats));
    memset(nmtr_mgr_pfc_stats_base, 0, sizeof(nmtr_mgr_pfc_stats_base));
#endif

#if (SYS_CPNT_CN == TRUE)
    memset(nmtr_mgr_qcn_stats,   0, sizeof(nmtr_mgr_qcn_stats));
    memset(nmtr_mgr_qcn_stats_base, 0, sizeof(nmtr_mgr_qcn_stats_base));
#endif

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
    memset(nmtr_mgr_if_xtable_stats_for_vlan,  0, sizeof(nmtr_mgr_if_xtable_stats));
    memset(nmtr_mgr_if_xtable_stats_base_for_vlan, 0, sizeof(nmtr_mgr_if_xtable_stats_base_for_vlan));
#endif

    memset(nmtr_mgr_utilization_stats, 0, sizeof(nmtr_mgr_utilization_stats));

    /* below is for Foundry % port inbound and outbound utilization */
    memset(nmtr_mgr_utilization_300_secs,   0, sizeof(nmtr_mgr_utilization_300_secs));
    memset(nmtr_mgr_utilization_300_secs_last_time,   0, sizeof(nmtr_mgr_utilization_300_secs_last_time));

    /* clear all statistics counters ( in chip )
     */
    NMTRDRV_ClearAllCounters();

    NMTR_MGR_EXIT_CRITICAL();
} /* end NMTR_MGR_Init_Statistics */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_SumOfUtilizationStats300secs
 * -------------------------------------------------------------------------
 * FUNCTION: get the sum of the utilization
 * INPUT   : NMTR_MGR_Utilization_T *destination_utilization_stats
 *           NMTR_MGR_Utilization_T source_utilization_stats
 * OUTPUT  : NMTR_MGR_Utilization_T *destination_utilization_stats
 * RETURN  : None
 * NOTE    : destination = destination + source
 * -------------------------------------------------------------------------
 */
static void NMTR_MGR_SumOfUtilizationStats300secs(NMTR_MGR_Utilization_300_SECS_T *destination_utilization_stats,
                                                  NMTR_MGR_Utilization_300_SECS_T source_utilization_stats )
{
    /*(*destination_utilization_stats).ifInOctets   += source_utilization_stats.ifInOctets;*/
    NMTR_MGR_UI64_Add(&(destination_utilization_stats->ifInOctets), source_utilization_stats.ifInOctets);


    /*(*destination_utilization_stats).ifOutOctets  += source_utilization_stats.ifOutOctets;*/
    NMTR_MGR_UI64_Add(&(destination_utilization_stats->ifOutOctets), source_utilization_stats.ifOutOctets);

    /*(*destination_utilization_stats).ifInPackets  += source_utilization_stats.ifInPackets;*/
    NMTR_MGR_UI64_Add(&(destination_utilization_stats->ifInPackets), source_utilization_stats.ifInPackets);

    /*(*destination_utilization_stats).ifOutPackets += source_utilization_stats.ifOutPackets;*/
    NMTR_MGR_UI64_Add(&(destination_utilization_stats->ifOutPackets), source_utilization_stats.ifOutPackets);

    (*destination_utilization_stats).ifInOctets_utilization  += source_utilization_stats.ifInOctets_utilization;
    (*destination_utilization_stats).ifOutOctets_utilization += source_utilization_stats.ifOutOctets_utilization;
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_MGR_GetPortUtilization300secs
 * -------------------------------------------------------------------------
 * FUNCTION: get the utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 *           utilization_entry  -- pointer of output buffer
 * OUTPUT  : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T NMTR_MGR_GetPortUtilization300secs(UI32_T ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry)
{
    SWCTRL_Lport_Type_T     port_type;
    TRK_MGR_TrunkEntry_T    trunk_entry;
    UI32_T                  unit, port, j;
    UI32_T                  trunk_id, trunk_member_l_port = 0;
    UI32_T                  num_of_trunk_member;
    NMTR_MGR_Utilization_300_SECS_T temp_utilization_entry;


    //SYSFUN_USE_CSC( FALSE );

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (utilization_entry == 0)
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        memset(utilization_entry, 0, sizeof(NMTR_MGR_Utilization_300_SECS_T));

        /* the logical port is vid
         */
        if (NMTR_MGR_IS_VLAN(ifindex))
        {
        #ifdef CHIP_SUPPORT_VLAN_COUNTER
            /* Not implemented */        
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #else   /* the chip does not support vlan counter */
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif /* end of CHIP_SUPPORT_VLAN_COUNTER */
        }

        port_type = SWCTRL_LogicalPortToUserPort (ifindex, &unit, &port, &trunk_id);

        if (NMTR_MGR_IS_USER_PORT_TYPE(port_type))
        {
            /* Calculate these values */
            for (j = 0; j <= NMTR_MGR_SLICE_NUMBER -1 ; j++)
            {
                /*utilization_entry->ifInOctets   += nmtr_mgr_utilization_300_secs[ifindex-1][j].ifInOctets;*/
                NMTR_MGR_UI64_Add(&(utilization_entry->ifInOctets), nmtr_mgr_utilization_300_secs[ifindex-1][j].ifInOctets);

                /*utilization_entry->ifOutOctets  += nmtr_mgr_utilization_300_secs[ifindex-1][j].ifOutOctets;*/
                NMTR_MGR_UI64_Add(&(utilization_entry->ifOutOctets), nmtr_mgr_utilization_300_secs[ifindex-1][j].ifOutOctets);

                /*utilization_entry->ifInPackets  += nmtr_mgr_utilization_300_secs[ifindex-1][j].ifInPackets;*/
                NMTR_MGR_UI64_Add(&(utilization_entry->ifInPackets), nmtr_mgr_utilization_300_secs[ifindex-1][j].ifInPackets);

                /*utilization_entry->ifOutPackets += nmtr_mgr_utilization_300_secs[ifindex-1][j].ifOutPackets;*/
                NMTR_MGR_UI64_Add(&(utilization_entry->ifOutPackets), nmtr_mgr_utilization_300_secs[ifindex-1][j].ifOutPackets);

                utilization_entry->ifInOctets_utilization  += nmtr_mgr_utilization_300_secs[ifindex-1][j].ifInOctets_utilization;
                utilization_entry->ifOutOctets_utilization += nmtr_mgr_utilization_300_secs[ifindex-1][j].ifOutOctets_utilization;
            }

            if (in_first_300_secs == TRUE)
            {
                /*utilization_entry->ifInOctets   = utilization_entry->ifInOctets  /((nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifInOctets), (nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);

                /*utilization_entry->ifOutOctets  = utilization_entry->ifOutOctets /((nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifOutOctets), (nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);

                /*utilization_entry->ifInPackets  = utilization_entry->ifInPackets /((nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifInPackets), (nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);

                /*utilization_entry->ifOutPackets = utilization_entry->ifOutPackets/((nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifOutPackets), (nmtr_mgr_slice_num+1) * NMTR_MGR_TIME_UNIT);

                utilization_entry->ifInOctets_utilization  = utilization_entry->ifInOctets_utilization /(nmtr_mgr_slice_num+1);
                utilization_entry->ifOutOctets_utilization = utilization_entry->ifOutOctets_utilization/(nmtr_mgr_slice_num+1);
            }
            else
            {
                /*utilization_entry->ifInOctets   = utilization_entry->ifInOctets  /NMTR_MGR_TIME_PERIOD;*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifInOctets), NMTR_MGR_TIME_PERIOD);

                /*utilization_entry->ifOutOctets  = utilization_entry->ifOutOctets /NMTR_MGR_TIME_PERIOD;*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifOutOctets), NMTR_MGR_TIME_PERIOD);

                /*utilization_entry->ifInPackets  = utilization_entry->ifInPackets /NMTR_MGR_TIME_PERIOD;*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifInPackets), NMTR_MGR_TIME_PERIOD);

                /*utilization_entry->ifOutPackets = utilization_entry->ifOutPackets/NMTR_MGR_TIME_PERIOD;*/
                NMTR_MGR_UI64_Div(&(utilization_entry->ifOutPackets), NMTR_MGR_TIME_PERIOD);

                utilization_entry->ifInOctets_utilization  = utilization_entry->ifInOctets_utilization  * NMTR_MGR_TIME_UNIT/NMTR_MGR_TIME_PERIOD;
                utilization_entry->ifOutOctets_utilization = utilization_entry->ifOutOctets_utilization * NMTR_MGR_TIME_UNIT/NMTR_MGR_TIME_PERIOD;

                if (utilization_entry->ifInOctets_utilization > 10000)
                    utilization_entry->ifInOctets_utilization = 10000;
                if (utilization_entry->ifOutOctets_utilization > 10000)
                    utilization_entry->ifOutOctets_utilization = 10000;
            }
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }

        if (NMTR_MGR_IS_TRUNK_PORT_TYPE(port_type))
        {
        #ifdef CHIP_SUPPORT_TRUNK_COUNTER
        #else
            /* Mercury: the broadcom chip does not support the trunk counters, need to calculate the
               trunk counters
             */
            trunk_entry.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_entry))
            {
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }

            num_of_trunk_member = 0;

            while (NMTR_MGR_GetNextIndexFromPortList(&trunk_member_l_port, trunk_entry.trunk_ports, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
            {
                if (NMTR_MGR_GetPortUtilization300secs(trunk_member_l_port, &temp_utilization_entry))
                {
                    NMTR_MGR_SumOfUtilizationStats300secs(utilization_entry, temp_utilization_entry);
                    num_of_trunk_member++;
                }
            }

            if (num_of_trunk_member > 1)
            {
                utilization_entry->ifInOctets_utilization /= num_of_trunk_member;
                utilization_entry->ifOutOctets_utilization /= num_of_trunk_member;
            }

            //SYSFUN_RELEASE_CSC();
            return TRUE;
        #endif  /* end of CHIP_SUPPORT_TRUNK_COUNTER */
        }/* end of if (trunk port) */

        if (NMTR_MGR_IS_UNKNOWN_PORT_TYPE(port_type))
        {
            /* UIMSG_MGR_SetErrorCode(NMTR_EH_PORT_NOT_EXIST); */
            EH_MGR_Handle_Exception1(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Interface");
        }
    }/* End of if (operation_mode == MASTER) */
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    }

    //SYSFUN_RELEASE_CSC();
    return FALSE;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextPortUtilization300secs                                         
 * -------------------------------------------------------------------------|
 * FUNCTION: This function will get the next utilization of Port statistics
 * INPUT   : ifindex            -- interface index
 * OUTPUT  : ifindex            -- interface index
 *         : utilization_entry  -- utilization value
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T NMTR_MGR_GetNextPortUtilization300secs(UI32_T *ifindex, NMTR_MGR_Utilization_300_SECS_T *utilization_entry)
{
    Port_Info_T port_info;
    BOOL_T      retval = FALSE;
    
    /* BODY 
     */
    SYSFUN_USE_CSC(FALSE);  

    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {     
        if (!SWCTRL_GetNextPortInfo(ifindex, &port_info))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }
       
        retval = NMTR_MGR_GetPortUtilization300secs(*ifindex, utilization_entry);
    }
    else
    {
        /* UIMSG_MGR_SetErrorCode(NMTR_EH_NOT_MASTER); */
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
    } 
        
    SYSFUN_RELEASE_CSC();
    return retval;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfIfTableStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of the iftable statistics
 * INPUT   : SWDRV_IfTableStats_T *destination_if_table_stats                       
 *           SWDRV_IfTableStats_T source_if_table_stats
 * OUTPUT  : SWDRV_IfTableStats_T *destination_if_table_stats                                                           
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source                                                           
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfIfTableStats(SWDRV_IfTableStats_T *destination_if_table_stats, 
                                   SWDRV_IfTableStats_T source_if_table_stats )
{
    (*destination_if_table_stats).ifInOctets += source_if_table_stats.ifInOctets;
    (*destination_if_table_stats).ifInUcastPkts += source_if_table_stats.ifInUcastPkts;
    (*destination_if_table_stats).ifInNUcastPkts += source_if_table_stats.ifInNUcastPkts;
    (*destination_if_table_stats).ifInDiscards += source_if_table_stats.ifInDiscards;
    (*destination_if_table_stats).ifInErrors += source_if_table_stats.ifInErrors;
    (*destination_if_table_stats).ifInUnknownProtos += source_if_table_stats.ifInUnknownProtos;
    (*destination_if_table_stats).ifOutOctets += source_if_table_stats.ifOutOctets;
    (*destination_if_table_stats).ifOutUcastPkts += source_if_table_stats.ifOutUcastPkts;
    (*destination_if_table_stats).ifOutNUcastPkts += source_if_table_stats.ifOutNUcastPkts;
    (*destination_if_table_stats).ifOutDiscards += source_if_table_stats.ifOutDiscards;
    (*destination_if_table_stats).ifOutErrors += source_if_table_stats.ifOutErrors;
    (*destination_if_table_stats).ifOutQLen += source_if_table_stats.ifOutQLen;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfIfXTableStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of extended iftable statistics
 * INPUT   : SWDRV_IfXTableStats_T *destination_if_xtable_stats                       
 *           SWDRV_IfXTableStats_T source_if_xtable_stats
 * OUTPUT  : SWDRV_IfXTableStats_T *destination_if_xtable_stats                                                           
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfIfXTableStats(SWDRV_IfXTableStats_T *destination_if_xtable_stats, 
                                    SWDRV_IfXTableStats_T source_if_xtable_stats )
{
    (*destination_if_xtable_stats).ifInMulticastPkts += source_if_xtable_stats.ifInMulticastPkts;
    (*destination_if_xtable_stats).ifInBroadcastPkts += source_if_xtable_stats.ifInBroadcastPkts;
    (*destination_if_xtable_stats).ifOutMulticastPkts += source_if_xtable_stats.ifOutMulticastPkts;
    (*destination_if_xtable_stats).ifOutBroadcastPkts += source_if_xtable_stats.ifOutBroadcastPkts;
    (*destination_if_xtable_stats).ifHCInOctets += source_if_xtable_stats.ifHCInOctets;
    (*destination_if_xtable_stats).ifHCInUcastPkts += source_if_xtable_stats.ifHCInUcastPkts;
    (*destination_if_xtable_stats).ifHCInMulticastPkts += source_if_xtable_stats.ifHCInMulticastPkts;
    (*destination_if_xtable_stats).ifHCInBroadcastPkts += source_if_xtable_stats.ifHCInBroadcastPkts;
    (*destination_if_xtable_stats).ifHCOutOctets += source_if_xtable_stats.ifHCOutOctets;
    (*destination_if_xtable_stats).ifHCOutUcastPkts += source_if_xtable_stats.ifHCOutUcastPkts;
    (*destination_if_xtable_stats).ifHCOutMulticastPkts += source_if_xtable_stats.ifHCOutMulticastPkts;
    (*destination_if_xtable_stats).ifHCOutBroadcastPkts += source_if_xtable_stats.ifHCOutBroadcastPkts;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfEtherLikeStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of ether-like statistics
 * INPUT   : SWDRV_EtherlikeStats_T *destination_ether_like_stats                       
 *           SWDRV_EtherlikeStats_T source_ether_like_stats
 * OUTPUT  : SWDRV_EtherlikeStats_T *destination_ether_like_stats                                                           
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfEtherLikeStats(SWDRV_EtherlikeStats_T *destination_ether_like_stats, 
                                     SWDRV_EtherlikeStats_T source_ether_like_stats )
{
    (*destination_ether_like_stats).dot3StatsAlignmentErrors += source_ether_like_stats.dot3StatsAlignmentErrors;
    (*destination_ether_like_stats).dot3StatsFCSErrors += source_ether_like_stats.dot3StatsFCSErrors;
    (*destination_ether_like_stats).dot3StatsSingleCollisionFrames += source_ether_like_stats.dot3StatsSingleCollisionFrames;
    (*destination_ether_like_stats).dot3StatsMultipleCollisionFrames += source_ether_like_stats.dot3StatsMultipleCollisionFrames;
    (*destination_ether_like_stats).dot3StatsSQETestErrors += source_ether_like_stats.dot3StatsSQETestErrors;
    (*destination_ether_like_stats).dot3StatsDeferredTransmissions += source_ether_like_stats.dot3StatsDeferredTransmissions;
    (*destination_ether_like_stats).dot3StatsLateCollisions += source_ether_like_stats.dot3StatsLateCollisions;
    (*destination_ether_like_stats).dot3StatsExcessiveCollisions += source_ether_like_stats.dot3StatsExcessiveCollisions;
    (*destination_ether_like_stats).dot3StatsInternalMacTransmitErrors += source_ether_like_stats.dot3StatsInternalMacTransmitErrors;
    (*destination_ether_like_stats).dot3StatsCarrierSenseErrors += source_ether_like_stats.dot3StatsCarrierSenseErrors;
    (*destination_ether_like_stats).dot3StatsFrameTooLongs += source_ether_like_stats.dot3StatsFrameTooLongs;
    (*destination_ether_like_stats).dot3StatsInternalMacReceiveErrors += source_ether_like_stats.dot3StatsInternalMacReceiveErrors;
    (*destination_ether_like_stats).dot3StatsSymbolErrors += source_ether_like_stats.dot3StatsSymbolErrors;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfEtherLikePause
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of ether-like pause statistics
 * INPUT   : SWDRV_EtherlikePause_T *destination_ether_like_pause
 *           SWDRV_EtherlikePause_T source_ether_like_pause
 * OUTPUT  : SWDRV_EtherlikePause_T *destination_ether_like_pause
 * RETURN  : None
 * NOTE    : destination = destination + source
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfEtherLikePause(SWDRV_EtherlikePause_T *destination_ether_like_pause,
                                     SWDRV_EtherlikePause_T source_ether_like_pause )
{
    (*destination_ether_like_pause).dot3InPauseFrames += source_ether_like_pause.dot3InPauseFrames;
    (*destination_ether_like_pause).dot3OutPauseFrames += source_ether_like_pause.dot3OutPauseFrames;
}


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfRmonStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of RMON statistics 
 * INPUT   : SWDRV_RmonStats_T *destination_rmon_stats                       
 *           SWDRV_RmonStats_T source_rmon_stats
 * OUTPUT  : SWDRV_RmonStats_T *destination_rmon_stats                                                           
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfRmonStats(SWDRV_RmonStats_T *destination_rmon_stats, 
                                SWDRV_RmonStats_T source_rmon_stats )
{
    (*destination_rmon_stats).etherStatsDropEvents += source_rmon_stats.etherStatsDropEvents;
    (*destination_rmon_stats).etherStatsOctets += source_rmon_stats.etherStatsOctets;
    (*destination_rmon_stats).etherStatsPkts += source_rmon_stats.etherStatsPkts;
    (*destination_rmon_stats).etherStatsBroadcastPkts += source_rmon_stats.etherStatsBroadcastPkts;
    (*destination_rmon_stats).etherStatsMulticastPkts += source_rmon_stats.etherStatsMulticastPkts;
    (*destination_rmon_stats).etherStatsCRCAlignErrors += source_rmon_stats.etherStatsCRCAlignErrors;
    (*destination_rmon_stats).etherStatsUndersizePkts += source_rmon_stats.etherStatsUndersizePkts;
    (*destination_rmon_stats).etherStatsOversizePkts += source_rmon_stats.etherStatsOversizePkts;
    (*destination_rmon_stats).etherStatsFragments += source_rmon_stats.etherStatsFragments;
    (*destination_rmon_stats).etherStatsJabbers += source_rmon_stats.etherStatsJabbers;
    (*destination_rmon_stats).etherStatsCollisions += source_rmon_stats.etherStatsCollisions;
    (*destination_rmon_stats).etherStatsPkts64Octets += source_rmon_stats.etherStatsPkts64Octets;
    (*destination_rmon_stats).etherStatsPkts65to127Octets += source_rmon_stats.etherStatsPkts65to127Octets;
    (*destination_rmon_stats).etherStatsPkts128to255Octets += source_rmon_stats.etherStatsPkts128to255Octets;
    (*destination_rmon_stats).etherStatsPkts256to511Octets += source_rmon_stats.etherStatsPkts256to511Octets;
    (*destination_rmon_stats).etherStatsPkts512to1023Octets += source_rmon_stats.etherStatsPkts512to1023Octets;
    (*destination_rmon_stats).etherStatsPkts1024to1518Octets += source_rmon_stats.etherStatsPkts1024to1518Octets;
}


#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfIfPerQStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of the CoS queue statistics
 * INPUT   : *destination_stats
 *           source_stats
 * OUTPUT  : *destination_stats
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source                                                           
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfIfPerQStats(SWDRV_IfPerQStats_T *destination_stats,
                                   SWDRV_IfPerQStats_T source_stats )
{
    int i;

    for (i = 0; i < 8; i++)
    {
        (*destination_stats).cosq[i].ifOutOctets += source_stats.cosq[i].ifOutOctets;
        (*destination_stats).cosq[i].ifOutPkts += source_stats.cosq[i].ifOutPkts;
        (*destination_stats).cosq[i].ifOutDiscardOctets += source_stats.cosq[i].ifOutDiscardOctets;
        (*destination_stats).cosq[i].ifOutDiscardPkts += source_stats.cosq[i].ifOutDiscardPkts;
    }
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */


#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfPfcStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of the PFC statistics
 * INPUT   : *destination_stats
 *           source_stats
 * OUTPUT  : *destination_stats
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source                                                           
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfPfcStats(SWDRV_PfcStats_T *destination_stats,
                                   SWDRV_PfcStats_T source_stats )
{
    int i;

    (*destination_stats).ieee8021PfcRequests += source_stats.ieee8021PfcRequests;
    (*destination_stats).ieee8021PfcIndications += source_stats.ieee8021PfcIndications;

    for (i = 0; i < 8; i++)
    {
        (*destination_stats).pri[i].ieee8021PfcRequests += source_stats.pri[i].ieee8021PfcRequests;
        (*destination_stats).pri[i].ieee8021PfcIndications += source_stats.pri[i].ieee8021PfcIndications;
    }
}
#endif /* (SYS_CPNT_PFC == TRUE) */


#if (SYS_CPNT_CN == TRUE)
/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfQcnStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of the QCN statistics
 * INPUT   : *destination_stats
 *           source_stats
 * OUTPUT  : *destination_stats
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source                                                           
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfQcnStats(SWDRV_QcnStats_T *destination_stats,
                                   SWDRV_QcnStats_T source_stats )
{
    int i;

    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        (*destination_stats).cpq[i].qcnStatsOutCnms += source_stats.cpq[i].qcnStatsOutCnms;
    }
}
#endif /* (SYS_CPNT_CN == TRUE) */


/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_SumOfUtilizationStats                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the sum of the utilization
 * INPUT   : NMTR_MGR_Utilization_T *destination_utilization_stats                       
 *           NMTR_MGR_Utilization_T source_utilization_stats
 * OUTPUT  : NMTR_MGR_Utilization_T *destination_utilization_stats                                                           
 * RETURN  : None                                                           
 * NOTE    : destination = destination + source                                                           
 * -------------------------------------------------------------------------*/
static void NMTR_MGR_SumOfUtilizationStats(NMTR_MGR_Utilization_T *destination_utilization_stats, 
                                   NMTR_MGR_Utilization_T source_utilization_stats )
{
    (*destination_utilization_stats).ifInOctets_utilization += source_utilization_stats.ifInOctets_utilization;
    (*destination_utilization_stats).ifInUcastPkts_utilization += source_utilization_stats.ifInUcastPkts_utilization;
    (*destination_utilization_stats).ifInMulticastPkts_utilization += source_utilization_stats.ifInMulticastPkts_utilization;
    (*destination_utilization_stats).ifInBroadcastPkts_utilization += source_utilization_stats.ifInBroadcastPkts_utilization;
    (*destination_utilization_stats).ifInErrors_utilization += source_utilization_stats.ifInErrors_utilization;
}

/* -------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetNextIndexFromPortList                                     
 * -------------------------------------------------------------------------|
 * FUNCTION: get the next port from a portlist
 * INPUT   : *index         - next 1-bit position                       
 *           port_list[]    - which port list
 *           max_port_num       - max port num
 * OUTPUT  : NMTR_MGR_Utilization_T *destination_utilization_stats                                                           
 * RETURN  : None                                                           
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_GetNextIndexFromPortList (UI32_T *index,
                                                 UI8_T  *port_list,
                                                 UI32_T max_port_num)
{
    UI32_T byte_num;
    I32_T byte;
    I32_T bit;
    I32_T init_byte;                /*byte: 0 1 2 ... */
    I32_T init_bit;                 /*bit:  7 6 5 ... 0*/

    if (*index >= max_port_num)
        return FALSE;

    byte_num = (max_port_num + 7) / 8;

    init_byte = (UI32_T)((*index)/8);
    init_bit  = 7 - ( (*index)%8 );

    for (byte = init_byte; byte < byte_num; byte++)
    {
        if (port_list[byte] == 0)
        {
            init_bit = 7;
            continue;
        }

        for (bit = init_bit; bit >= 0; bit--)
        {
            if ( port_list[byte] & (1<<bit))
            {
                *index = (8*byte) + (8-bit);
                return (*index <= max_port_num);
            }
        }
        init_bit = 7;
    }

    return FALSE;  
}

/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 5 ) -- dot1dTp 5
 *          dot1dTpHCPortTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpHCPortEntry
 *              ::= { dot1dTp 5 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpHCPortTable 1 }
 *          Dot1dTpHCPortEntry ::=
 *              SEQUENCE {
 *                  dot1dTpHCPortInFrames   Counter64,
 *                  dot1dTpHCPortOutFrames  Counter64,
 *                  dot1dTpHCPortInDiscards Counter64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry)
{

    SWDRV_IfTableStats_T if_table_stats;
    //SYSFUN_USE_CSC(FALSE);
    
    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (tp_hc_port_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }    
    if (!NMTR_MGR_GetIfTableStats(tp_hc_port_entry->dot1d_tp_port, &if_table_stats))
    {
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    tp_hc_port_entry->dot1d_tp_hc_port_in_discards = if_table_stats.ifInDiscards;
    tp_hc_port_entry->dot1d_tp_hc_port_in_frames = if_table_stats.ifInUcastPkts + if_table_stats.ifInNUcastPkts;
    tp_hc_port_entry->dot1d_tp_hc_port_out_frames = if_table_stats.ifOutUcastPkts + if_table_stats.ifOutNUcastPkts;
    //SYSFUN_RELEASE_CSC();
    return TRUE;
} /* End of NMTR_MGR_GetDot1dTpHCPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpHCPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_hc_port_entry->dot1d_tp_port -- which port (key)
 * OUTPUT   :   tp_hc_port_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpHCPortTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetNextDot1dTpHCPortEntry(NMTR_MGR_Dot1dTpHCPortEntry_T *tp_hc_port_entry)
{
    SWDRV_IfTableStats_T if_table_stats;
    //SYSFUN_USE_CSC(FALSE);
    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (tp_hc_port_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }    
    if (!NMTR_MGR_GetNextIfTableStats(&tp_hc_port_entry->dot1d_tp_port, &if_table_stats))
    {
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    tp_hc_port_entry->dot1d_tp_hc_port_in_discards = if_table_stats.ifInDiscards;
    tp_hc_port_entry->dot1d_tp_hc_port_in_frames = if_table_stats.ifInUcastPkts + if_table_stats.ifInNUcastPkts;
    tp_hc_port_entry->dot1d_tp_hc_port_out_frames = if_table_stats.ifOutUcastPkts + if_table_stats.ifOutNUcastPkts;
    
    //SYSFUN_RELEASE_CSC();
    return TRUE;
} /* End of NMTR_MGR_GetNextDot1dTpHCPortEntry */


/*-------------------------------------------------------------------------
 * The dot1dTp group (dot1dTp 6 ) -- dot1dTp 6
 *          dot1dTpPortOverflowTable OBJECT-TYPE
 *              SYNTAX      SEQUENCE OF Dot1dTpPortOverflowEntry
 *              ::= { dot1dTp 6 }
 *          dot1dTpHCPortEntry OBJECT-TYPE
 *              INDEX   { dot1dTpPort }
 *              ::= { dot1dTpPortOverflowTable 1 }
 *          Dot1dTpPortOverflowEntry ::=
 *              SEQUENCE {
 *                  dot1dTpPortInOverflowFrames     Counter32,
 *                  dot1dTpPortOutOverflowFrames    Counter32,
 *                  dot1dTpPortInOverflowDiscards   Counter32
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry)
{
    SWDRV_IfTableStats_T if_table_stats;
    UI64_T               temp;

    //SYSFUN_USE_CSC(FALSE);
    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (tp_port_overflow_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }    
    if (!NMTR_MGR_GetIfTableStats(tp_port_overflow_entry->dot1d_tp_port, &if_table_stats))
    {
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    tp_port_overflow_entry->dot1d_tp_port_in_overflow_discards = L_STDLIB_UI64_H32(if_table_stats.ifInDiscards);

    temp = if_table_stats.ifInUcastPkts;
    NMTR_MGR_UI64_Add(&temp, if_table_stats.ifInNUcastPkts);
    tp_port_overflow_entry->dot1d_tp_port_in_overflow_frames = L_STDLIB_UI64_H32(temp);

    temp = if_table_stats.ifOutUcastPkts;
    NMTR_MGR_UI64_Add(&temp, if_table_stats.ifOutNUcastPkts);
    tp_port_overflow_entry->dot1d_tp_port_out_overflow_frames = L_STDLIB_UI64_H32(temp);

    //SYSFUN_RELEASE_CSC();
    return TRUE; 
} /* End of NMTR_MGR_GetDot1dTpPortOverflowEntry */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified transparent port
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_port_overflow_entry->dot1d_tp_port   -- which port (key)
 * OUTPUT   :   tp_port_overflow_entry -- The next Port entry for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674/dot1dTpPortOverflowTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetNextDot1dTpPortOverflowEntry(NMTR_MGR_Dot1dTpPortOverflowEntry_T *tp_port_overflow_entry)
{
    SWDRV_IfTableStats_T if_table_stats;
    UI64_T               temp;

    //SYSFUN_USE_CSC(FALSE);
    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (tp_port_overflow_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }    
    if (!NMTR_MGR_GetNextIfTableStats(&tp_port_overflow_entry->dot1d_tp_port, &if_table_stats))
    {
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    tp_port_overflow_entry->dot1d_tp_port_in_overflow_discards = L_STDLIB_UI64_H32(if_table_stats.ifInDiscards);

    temp = if_table_stats.ifInUcastPkts;
    NMTR_MGR_UI64_Add(&temp, if_table_stats.ifInNUcastPkts);
    tp_port_overflow_entry->dot1d_tp_port_in_overflow_frames = L_STDLIB_UI64_H32(temp);

    temp = if_table_stats.ifOutUcastPkts;
    NMTR_MGR_UI64_Add(&temp, if_table_stats.ifOutNUcastPkts);
    tp_port_overflow_entry->dot1d_tp_port_out_overflow_frames = L_STDLIB_UI64_H32(temp);

    //SYSFUN_RELEASE_CSC();
    return TRUE;
} /* End of NMTR_MGR_GetNextDot1dTpPortOverflowEntry */


/*-------------------------------------------------------------------------
 * The etherStatsHighCapacityGroup group (statistics 7 ) -- statistics 7
 *          etherStatsHighCapacityTable OBJECT-TYPE
 *              SYNTAX     SEQUENCE OF EtherStatsHighCapacityEntry
 *              ::= { statistics 7 }
 *          etherStatsHighCapacityEntry OBJECT-TYPE
 *              INDEX   { etherStatsIndex }
 *              ::= { etherStatsHighCapacityTable 1 }
 *          EtherStatsHighCapacityEntry ::=
 *              SEQUENCE {
 *                  etherStatsHighCapacityOverflowPkts                 Counter32,
 *                  etherStatsHighCapacityPkts                         Counter64,
 *                  etherStatsHighCapacityOverflowOctets               Counter32,
 *                  etherStatsHighCapacityOctets                       Counter64,
 *                  etherStatsHighCapacityOverflowPkts64Octets         Counter32,
 *                  etherStatsHighCapacityPkts64Octets                 Counter64,
 *                  etherStatsHighCapacityOverflowPkts65to127Octets    Counter32,
 *                  etherStatsHighCapacityPkts65to127Octets            Counter64,
 *                  etherStatsHighCapacityOverflowPkts128to255Octets   Counter32,
 *                  etherStatsHighCapacityPkts128to255Octets           Counter64,
 *                  etherStatsHighCapacityOverflowPkts256to511Octets   Counter32,
 *                  etherStatsHighCapacityPkts256to511Octets           Counter64,
 *                  etherStatsHighCapacityOverflowPkts512to1023Octets  Counter32,
 *                  etherStatsHighCapacityPkts512to1023Octets          Counter64,
 *                  etherStatsHighCapacityOverflowPkts1024to1518Octets Counter32,
 *                  etherStatsHighCapacityPkts1024to1518Octets         Counter64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetEtherStatsHighCapacityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified ether statistics
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   es_hc_entry->ether_stats_index          -- (key)
 * OUTPUT   :   es_hc_entry         -- Ether stats high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherStatsHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetEtherStatsHighCapacityEntry(NMTR_MGR_EtherStatsHighCapacityEntry_T *es_hc_entry)
{
    SWDRV_RmonStats_T rmon_table_stats;
    //SYSFUN_USE_CSC(FALSE);
    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (es_hc_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }    
    if(! NMTR_MGR_GetRmonStats(es_hc_entry->ether_stats_index,&rmon_table_stats))
    {
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    
    es_hc_entry->ether_stats_high_capacity_pkts = rmon_table_stats.etherStatsPkts;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts);
    es_hc_entry->ether_stats_high_capacity_octets = rmon_table_stats.etherStatsOctets;
    es_hc_entry->ether_stats_high_capacity_overflow_octets =L_STDLIB_UI64_H32(rmon_table_stats.etherStatsOctets);

    es_hc_entry->ether_stats_high_capacity_pkts64_octets = rmon_table_stats.etherStatsPkts64Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts64_octets=L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts64Octets);
    es_hc_entry->ether_stats_high_capacity_pkts65to127_octets = rmon_table_stats.etherStatsPkts65to127Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts65to127_octets=L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts65to127Octets);

    es_hc_entry->ether_stats_high_capacity_pkts128to255_octets = rmon_table_stats.etherStatsPkts128to255Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts128to255_octets=L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts128to255Octets);
    es_hc_entry->ether_stats_high_capacity_pkts256to511_octets = rmon_table_stats.etherStatsPkts256to511Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts256to511_octets = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts256to511Octets);

    es_hc_entry->ether_stats_high_capacity_pkts512to1023_octets=rmon_table_stats.etherStatsPkts512to1023Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts512to1023_octets =  L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts512to1023Octets);
    es_hc_entry->ether_stats_high_capacity_pkts1024to1518_octets = rmon_table_stats.etherStatsPkts1024to1518Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts1024to1518_octets = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts1024to1518Octets);

    //SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextDot1dTpPortOverflowEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified ether statistics
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   es_hc_entry->ether_stats_index          -- (key)
 * OUTPUT   :   es_hc_entry         -- The next Ether stats high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherStatsHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetNextEtherStatsHighCapacityEntry(NMTR_MGR_EtherStatsHighCapacityEntry_T *es_hc_entry)
{
    SWDRV_RmonStats_T rmon_table_stats;
    //SYSFUN_USE_CSC(FALSE);
    if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (es_hc_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NMTR, NMTR_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NULL_POINTER, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }    
    if(! NMTR_MGR_GetNextRmonStats(&es_hc_entry->ether_stats_index,&rmon_table_stats))
    {
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    es_hc_entry->ether_stats_high_capacity_pkts = rmon_table_stats.etherStatsPkts;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts);
    es_hc_entry->ether_stats_high_capacity_octets = rmon_table_stats.etherStatsOctets;
    es_hc_entry->ether_stats_high_capacity_overflow_octets = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsOctets);

    es_hc_entry->ether_stats_high_capacity_pkts64_octets = rmon_table_stats.etherStatsPkts64Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts64_octets=L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts64Octets);
    es_hc_entry->ether_stats_high_capacity_pkts65to127_octets = rmon_table_stats.etherStatsPkts65to127Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts65to127_octets=L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts65to127Octets);

    es_hc_entry->ether_stats_high_capacity_pkts128to255_octets = rmon_table_stats.etherStatsPkts128to255Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts128to255_octets=L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts128to255Octets);
    es_hc_entry->ether_stats_high_capacity_pkts256to511_octets = rmon_table_stats.etherStatsPkts256to511Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts256to511_octets = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts256to511Octets);

    es_hc_entry->ether_stats_high_capacity_pkts512to1023_octets=rmon_table_stats.etherStatsPkts512to1023Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts512to1023_octets =  L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts512to1023Octets);
    es_hc_entry->ether_stats_high_capacity_pkts1024to1518_octets = rmon_table_stats.etherStatsPkts1024to1518Octets;
    es_hc_entry->ether_stats_high_capacity_overflow_pkts1024to1518_octets = L_STDLIB_UI64_H32(rmon_table_stats.etherStatsPkts1024to1518Octets);

    //SYSFUN_RELEASE_CSC();
    return TRUE;
}


/*-------------------------------------------------------------------------
 * The etherHistoryHighCapacityGroup group (history 6 ) -- history 6
 *          etherHistoryHighCapacityTable OBJECT-TYPE
 *              SYNTAX     SEQUENCE OF EtherHistoryHighCapacityEntry
 *              ::= { history 6 }
 *          etherHistoryHighCapacityEntry OBJECT-TYPE
 *              INDEX   { etherHistoryIndex, etherHistorySampleIndex }
 *              ::= { etherHistoryHighCapacityTable 1 }
 *          EtherHistoryHighCapacityEntry ::=
 *              SEQUENCE {
 *                  etherHistoryHighCapacityOverflowPkts    Gauge32,
 *                  etherHistoryHighCapacityPkts            CounterBasedGauge64,
 *                  etherHistoryHighCapacityOverflowOctets  Gauge32,
 *                  etherHistoryHighCapacityOctets          CounterBasedGauge64
 *              }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetEtherHistoryHighCapacityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified ether history
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   eh_hc_entry->ether_history_index        -- (key)
 *              eh_hc_entry->ether_history_sample_index -- (key)
 * OUTPUT   :   eh_hc_entry         -- Ether history high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherHistoryHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetEtherHistoryHighCapacityEntry(NMTR_MGR_EtherHistoryHighCapacityEntry_T *eh_hc_entry)
{
    /* Temporary code before final implementation */
    UI32_T  ether_history_index             = eh_hc_entry->ether_history_index;
    UI32_T  ether_history_sample_index      = eh_hc_entry->ether_history_sample_index;
    memset(eh_hc_entry, 0, sizeof(NMTR_MGR_EtherHistoryHighCapacityEntry_T) );
    eh_hc_entry->ether_history_index        = ether_history_index;
    eh_hc_entry->ether_history_sample_index = ether_history_sample_index;
    return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetNextEtherHistoryHighCapacityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified ether history
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   eh_hc_entry->ether_history_index        -- (key)
 *              eh_hc_entry->ether_history_sample_index -- (key)
 * OUTPUT   :   eh_hc_entry         -- The next Ether history high capacity entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC3273/etherHistoryHighCapacityTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T  NMTR_MGR_GetNextEtherHistoryHighCapacityEntry(NMTR_MGR_EtherHistoryHighCapacityEntry_T *eh_hc_entry)
{
    /* Temporary code before final implementation */
    UI32_T  ether_history_index             = eh_hc_entry->ether_history_index;
    UI32_T  ether_history_sample_index      = eh_hc_entry->ether_history_sample_index;
    memset(eh_hc_entry, 0, sizeof(NMTR_MGR_EtherHistoryHighCapacityEntry_T) );
    eh_hc_entry->ether_history_index        = ether_history_index;
    eh_hc_entry->ether_history_sample_index = ether_history_sample_index;
    return TRUE;
}

/* LOCAL ROUTINE
 */

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateIFTableStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update IF table statistics in Core OM.
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_port                 - start port
 *           UI32_T updated_port_num           - port num which will be update
 *           SWDRV_IfTableStats_T if_stats[]   - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : ex. this function will copy ifX_stats[] to nmtr_mgr_if_table_stats[]
 *               if_stats[0] --> nmtr_mgr_if_table_stats[unit/start_port]
 *               if_stats[1] --> nmtr_mgr_if_table_stats[unit/(start_port+1)]
 *                       :                               :
 *               if_stats[start_port-1] --> nmtr_mgr_if_table_stats[unit/(start_port+1)]                                                     
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateIFTableStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num)
{
    UI32_T      updated_ifindex;
    UI32_T      num_index;
    UI32_T      port_link_status;
    UI32_T      speed_duplex_oper;
    UI64_T      rx_packet_count_of_this_port=0;
    UI64_T      tx_packet_count_of_this_port=0;
    UI64_T      temp=0, temp1=0, temp2=0;    
    Port_Info_T p_info;
    SWDRV_IfTableStats_T if_stats;

    struct timespec cur_time;
    time_t now;
    char date_time_ar[40] = {0};
    char offset_ar[8] = {0};

    time(&now);
    clock_gettime(CLOCK_REALTIME, &cur_time);
    strftime(date_time_ar, sizeof(date_time_ar), "%FT%T", gmtime(&now));
    strftime(offset_ar, sizeof(offset_ar), "%z", gmtime(&now));
    sprintf(date_time_ar, "%s,%ld", date_time_ar, cur_time.tv_nsec);
    sprintf(date_time_ar, "%s%s", date_time_ar, offset_ar);

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetIfStats(unit,(start_port+num_index-1), &if_stats);
        /* calculate the different
         */
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifInOctets,if_stats.ifInOctets,&(nmtr_mgr_different_counter[updated_ifindex-1].ifInOctets));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifInUcastPkts,if_stats.ifInUcastPkts,&(nmtr_mgr_different_counter[updated_ifindex-1].ifInUcastPkts));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifInNUcastPkts,if_stats.ifInNUcastPkts,&(nmtr_mgr_different_counter[updated_ifindex-1].ifInNUcastPkts));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifInDiscards,if_stats.ifInDiscards,&(nmtr_mgr_different_counter[updated_ifindex-1].ifInDiscards));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifInErrors,if_stats.ifInErrors,&(nmtr_mgr_different_counter[updated_ifindex-1].ifInErrors));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutOctets,if_stats.ifOutOctets,&(nmtr_mgr_different_counter[updated_ifindex-1].ifOutOctets));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutUcastPkts,if_stats.ifOutUcastPkts,&(nmtr_mgr_different_counter[updated_ifindex-1].ifOutUcastPkts));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutNUcastPkts,if_stats.ifOutNUcastPkts,&(nmtr_mgr_different_counter[updated_ifindex-1].ifOutNUcastPkts));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutDiscards,if_stats.ifOutDiscards,&(nmtr_mgr_different_counter[updated_ifindex-1].ifOutDiscards));
        NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutErrors,if_stats.ifOutErrors,&(nmtr_mgr_different_counter[updated_ifindex-1].ifOutErrors));
        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_if_table_stats[updated_ifindex-1],&if_stats,sizeof (SWDRV_IfTableStats_T));
        strncpy(nmtr_mgr_if_table_stats[updated_ifindex-1].ifDateTime, date_time_ar,
              sizeof(nmtr_mgr_if_table_stats[updated_ifindex-1].ifDateTime));
        
        /* Update Utilization
         */
        if (SWCTRL_GetPortInfo(updated_ifindex, &p_info) == TRUE)
        {
            port_link_status = p_info.link_status;
            speed_duplex_oper = p_info.speed_duplex_oper;
            NMTR_MGR_SumThreeUI64Var(nmtr_mgr_different_counter[updated_ifindex-1].ifInUcastPkts,
                                     nmtr_mgr_different_counter[updated_ifindex-1].ifInNUcastPkts,
                                     nmtr_mgr_different_counter[updated_ifindex-1].ifInDiscards,
                                     &rx_packet_count_of_this_port);
            NMTR_MGR_SumThreeUI64Var(nmtr_mgr_different_counter[updated_ifindex-1].ifOutUcastPkts,
                                     nmtr_mgr_different_counter[updated_ifindex-1].ifOutNUcastPkts,
                                     nmtr_mgr_different_counter[updated_ifindex-1].ifOutDiscards,
                                     &tx_packet_count_of_this_port);
        }
        else
        {
            port_link_status = SWCTRL_LINK_DOWN;
        }
        
        if (p_info.link_status == SWCTRL_LINK_UP && ((rx_packet_count_of_this_port != 0) || (tx_packet_count_of_this_port != 0)))
        {
            switch(p_info.speed_duplex_oper)
            {
            case VAL_portSpeedDpxCfg_halfDuplex10:
            case VAL_portSpeedDpxCfg_fullDuplex10:
                /* in packets.
                 */
                temp = rx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifInOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_10);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = temp2;
                
                /* out packets
                 */
                temp = tx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifOutOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_10);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization = temp2;

                break;
            case VAL_portSpeedDpxCfg_halfDuplex100:
            case VAL_portSpeedDpxCfg_fullDuplex100:
                /* in packets.
                 */
                temp = rx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifInOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_100);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = temp2;
                
                /* out packets
                 */
                temp = tx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifOutOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_100);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization = temp2;
                break;
            case VAL_portSpeedDpxCfg_halfDuplex1000:
            case VAL_portSpeedDpxCfg_fullDuplex1000:
                /* in packets.
                 */
                temp = rx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifInOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_1000);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = temp2;
                
                /* out packets
                 */
                temp = tx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifOutOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_1000);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization = temp2;
                break;
            case VAL_portSpeedDpxCfg_halfDuplex10g:
            case VAL_portSpeedDpxCfg_fullDuplex10g:
                /* in packets.
                 */
                temp = rx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifInOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_1000);
                NMTR_MGR_UI64_Div(&temp2, 10);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = temp2;
                
                /* out packets
                 */
                temp = tx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifOutOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_1000);
                NMTR_MGR_UI64_Div(&temp2, 10);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization = temp2;
                break;
            case VAL_portSpeedDpxCfg_halfDuplex40g:
            case VAL_portSpeedDpxCfg_fullDuplex40g:
                /* in packets.
                 */
                temp = rx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifInOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_1000);
                NMTR_MGR_UI64_Div(&temp2, 40);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = temp2;
                
                /* out packets
                 */
                temp = tx_packet_count_of_this_port;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_different_counter[updated_ifindex-1].ifOutOctets;
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                NMTR_MGR_UI64_Multi(&temp2, 100);
                NMTR_MGR_UI64_Div(&temp2, MAX_BITS_NUMBER_OF_SPEED_1000);
                NMTR_MGR_UI64_Div(&temp2, 40);
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization = temp2;
                break;
            }

            if (nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization > 100)
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = 100;
            if (nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization > 100)
                nmtr_mgr_utilization_stats[updated_ifindex-1].ifOutOctets_utilization = 100;

            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInUcastPkts_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInMulticastPkts_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInBroadcastPkts_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInErrors_utilization = 0;
        }
        else /* link_down */
        {
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInOctets_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInUcastPkts_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInMulticastPkts_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInBroadcastPkts_utilization = 0;
            nmtr_mgr_utilization_stats[updated_ifindex-1].ifInErrors_utilization = 0;
        }
    }
    NMTR_MGR_EXIT_CRITICAL();    
    return TRUE;    
}

static BOOL_T NMTR_MGR_Update300sUtilization_CallBack(void)
{
    UI32_T      updated_ifindex;
    UI32_T      port_link_status;
    UI32_T      speed_duplex_oper;
    UI32_T      timepass=0;
    UI32_T      sysuptime = 0;
    Port_Info_T p_info;    
    UI64_T      temp, temp1, temp2;    
    UI64_T      total_in_packets;
    UI64_T      total_out_packets;

    /* Don't need to enter critical section.
     * Only NMTRDRV task set nmtr_mgr_utilization_300_secs[][].
     */

    sysuptime = SYSFUN_GetSysTick();
    if (sysuptime_base == 0)
    {
        /* first time, update sysuptime_base only.
         */
        sysuptime_base = sysuptime;
        return TRUE;
    }
    timepass = sysuptime - sysuptime_base;
        
    if (timepass >= NMTR_MGR_TIME_UNIT *100)
    {
        sysuptime_base = sysuptime;

        for (updated_ifindex = 1;updated_ifindex<=SYS_ADPT_TOTAL_NBR_OF_LPORT;updated_ifindex++)
        {           
            /* update nmtr_mgr_utilization_300_secs[][].ifInOctets
             */
            NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifInOctets,
                                    nmtr_mgr_if_table_stats[updated_ifindex-1].ifInOctets,
                                    &(nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets));
            /* update nmtr_mgr_utilization_300_secs[][].ifOutOctets
             */        
            NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifOutOctets,
                                    nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutOctets,
                                    &(nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets));
            /* update nmtr_mgr_utilization_300_secs[][].ifInPackets
             */        
            NMTR_MGR_SumThreeUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifInUcastPkts,
                                     nmtr_mgr_if_table_stats[updated_ifindex-1].ifInNUcastPkts,
                                     nmtr_mgr_if_table_stats[updated_ifindex-1].ifInDiscards,
                                     &total_in_packets);
            NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifInPackets,
                                    total_in_packets,
                                    &(nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInPackets));
            /* update nmtr_mgr_utilization_300_secs[][].ifOutPackets
             */                
            NMTR_MGR_SumThreeUI64Var(nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutUcastPkts,
                                     nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutNUcastPkts,
                                     nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutDiscards,
                                     &total_out_packets);
            NMTR_MGR_DiffTwoUI64Var(nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifOutPackets,
                                    total_out_packets,
                                    &(nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutPackets));
            
            /* save the new counters
             */
            nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifInPackets  = total_in_packets;
            nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifOutPackets = total_out_packets;    
            nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifInOctets   = nmtr_mgr_if_table_stats[updated_ifindex-1].ifInOctets;
            nmtr_mgr_utilization_300_secs_last_time[updated_ifindex-1].ifOutOctets  = nmtr_mgr_if_table_stats[updated_ifindex-1].ifOutOctets;       

            if (SWCTRL_GetPortInfo(updated_ifindex, &p_info) == TRUE)
            {
                port_link_status = p_info.link_status;
                speed_duplex_oper = p_info.speed_duplex_oper;
            }
            else
            {
                port_link_status = SWCTRL_LINK_DOWN;
            }

            if (p_info.link_status == SWCTRL_LINK_UP)
            {
                /* count nmtr_mgr_utilization_300_secs[][].ifInOctets_utilization
                 */
                temp  = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInPackets;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets ; /*use bit as unit*/
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                 /*10000 is because Foundry need 00.00%
                  * nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets_utilization = (temp2 * 10000 /(MAX_BITS_NUMBER_OF_SPEED*NMTR_MGR_TIME_UNIT))
                  */
                NMTR_MGR_UI64_Multi(&temp2, 10000);
                NMTR_MGR_UI64_Div(&temp2, 1000);
                NMTR_MGR_UI64_Div(&temp2, p_info.bandwidth);
                NMTR_MGR_UI64_Div(&temp2, NMTR_MGR_TIME_UNIT);
                nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets_utilization = temp2;

                /* count nmtr_mgr_utilization_300_secs[][].ifOutOctets_utilization
                 */
                temp  = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutPackets;
                NMTR_MGR_UI64_Multi(&temp, (64+96));/* 8*8= 64 -->premable, 96 --> interframe gap */
                temp1 = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets ; /*use bit as unit*/
                NMTR_MGR_UI64_Multi(&temp1, 8);/*use bit as unit*/
                temp2 = temp1;
                NMTR_MGR_UI64_Add(&temp2, temp);/*total bytes + inpacket * (premable + IFG)*/
                 /*10000 is because Foundry need 00.00%
                  * nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets_utilization = (temp2 * 10000 /(MAX_BITS_NUMBER_OF_SPEED*NMTR_MGR_TIME_UNIT))
                  */
                NMTR_MGR_UI64_Multi(&temp2, 10000);
                NMTR_MGR_UI64_Div(&temp2, 1000);
                NMTR_MGR_UI64_Div(&temp2, p_info.bandwidth);
                NMTR_MGR_UI64_Div(&temp2, NMTR_MGR_TIME_UNIT);
                nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets_utilization = temp2;
            }
            else /* link_down */
            {
                nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets_utilization  = 0;
                nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets_utilization = 0;
            }

            /*time period will be a lillte bigger than 5 secs, need to calculate the ratio*/
            /*nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets   = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets  * NMTR_MGR_TIME_UNIT *100 /timepass;*/
            temp = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets;
            NMTR_MGR_UI64_Multi(&temp, NMTR_MGR_TIME_UNIT * 100);                                                         /* 5 replace timepass  */
            NMTR_MGR_UI64_Div(&temp, timepass);
            nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets = temp;

            /*nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets  = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets * NMTR_MGR_TIME_UNIT *100 /timepass;*/
            temp = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets;
            NMTR_MGR_UI64_Multi(&temp, NMTR_MGR_TIME_UNIT * 100);
            NMTR_MGR_UI64_Div(&temp, timepass);
            nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets = temp;

            /*nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInPackets  = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInPackets * NMTR_MGR_TIME_UNIT *100 /timepass;*/
            temp = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInPackets;
            NMTR_MGR_UI64_Multi(&temp, NMTR_MGR_TIME_UNIT * 100);
            NMTR_MGR_UI64_Div(&temp, timepass);
            nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInPackets = temp;

            /*nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutPackets = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutPackets * NMTR_MGR_TIME_UNIT *100 /timepass;*/
            temp = nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutPackets;
            NMTR_MGR_UI64_Multi(&temp, NMTR_MGR_TIME_UNIT * 100);
            NMTR_MGR_UI64_Div(&temp, timepass);
            nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutPackets = temp;

            nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets_utilization  = (nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifInOctets_utilization * NMTR_MGR_TIME_UNIT *100) /timepass;
            nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets_utilization = (nmtr_mgr_utilization_300_secs[updated_ifindex-1][nmtr_mgr_slice_num].ifOutOctets_utilization * NMTR_MGR_TIME_UNIT *100) /timepass;
        } /*End of if()*/
        
        nmtr_mgr_slice_num++;
        if (nmtr_mgr_slice_num >= NMTR_MGR_SLICE_NUMBER)
        {
            if (in_first_300_secs == TRUE)
            {
                in_first_300_secs = FALSE;
            }
            nmtr_mgr_slice_num = 0; /*Reset*/
        }
    }/*End of for()*/
    
    return TRUE;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateIFXTableStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update IFX table statistics in Core OM.
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_port                 - start port
 *           UI32_T updated_port_num           - port num which will be update
 *           SWDRV_IfXTableStats_T ifX_stats[] - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : ex. this function will copy ifX_stats[] to nmtr_mgr_if_xtable_stats[]
 *               ifX_stats[0] --> nmtr_mgr_if_xtable_stats[unit/start_port]
 *               ifX_stats[1] --> nmtr_mgr_if_xtable_stats[unit/(start_port+1)]
 *                       :                               :
 *               ifX_stats[start_port-1] --> nmtr_mgr_if_xtable_stats[unit/(start_port+1)]                                                     
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateIFXTableStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num)
{
    UI32_T updated_ifindex;
    UI32_T num_index;
    SWDRV_IfXTableStats_T ifX_stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetIfXStats(unit,(start_port+num_index-1), &ifX_stats);
        /* count the different
         */
        NMTR_MGR_UpdateDiffCounter((UI32_T)nmtr_mgr_if_xtable_stats[updated_ifindex-1].ifInMulticastPkts,(UI32_T)ifX_stats.ifInMulticastPkts,nmtr_mgr_different_counter[updated_ifindex-1].ifInMulticastPkts);
        NMTR_MGR_UpdateDiffCounter((UI32_T)nmtr_mgr_if_xtable_stats[updated_ifindex-1].ifInBroadcastPkts,(UI32_T)ifX_stats.ifInBroadcastPkts,nmtr_mgr_different_counter[updated_ifindex-1].ifInBroadcastPkts);
        NMTR_MGR_UpdateDiffCounter((UI32_T)nmtr_mgr_if_xtable_stats[updated_ifindex-1].ifOutMulticastPkts,(UI32_T)ifX_stats.ifOutMulticastPkts,nmtr_mgr_different_counter[updated_ifindex-1].ifOutMulticastPkts);
        NMTR_MGR_UpdateDiffCounter((UI32_T)nmtr_mgr_if_xtable_stats[updated_ifindex-1].ifOutBroadcastPkts,(UI32_T)ifX_stats.ifOutBroadcastPkts,nmtr_mgr_different_counter[updated_ifindex-1].ifOutBroadcastPkts);        
        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_if_xtable_stats[updated_ifindex-1],&ifX_stats,sizeof (SWDRV_IfXTableStats_T));

        /* Update Utilization
         * already in NMTR_MGR_UpdateIFTableStats_CallBack().
         */
    }
    /*leave critical section
     */
    NMTR_MGR_EXIT_CRITICAL();    
    return TRUE;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateRmonStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update Rmon statistics in Core OM.
 * INPUT   : UI32_T unit                    - unit
 *           UI32_T start_port              - start port
 *           UI32_T updated_port_num        - port num which will be update
 *           SWDRV_RmonStats_T rmon_stats[] - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : ex. this function will copy rmon_stats[] to nmtr_mgr_rmon_stats[]
 *               rmon_stats[0] --> nmtr_mgr_rmon_stats[unit/start_port]
 *               rmon_stats[1] --> nmtr_mgr_rmon_stats[unit/(start_port+1)]
 *                       :                               :
 *               rmon_stats[start_port-1] --> nmtr_mgr_rmon_stats[unit/(start_port+1)]
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateRmonStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num)
{
    UI32_T updated_ifindex;
    UI32_T num_index;
    SWDRV_RmonStats_T rmon_stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetRmonStats(unit,(start_port+num_index-1), &rmon_stats);
        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_rmon_stats[updated_ifindex-1],&rmon_stats,sizeof (SWDRV_RmonStats_T));
    }
    /*leave critical section
     */
    NMTR_MGR_EXIT_CRITICAL();    
    return TRUE;    
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateEtherLikeStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update Rmon statistics in Core OM.
 * INPUT   : UI32_T unit                                - unit
 *           UI32_T start_port                          - start port
 *           UI32_T updated_port_num                    - port num which will be update
 *           SWDRV_EtherlikeStats_T ether_like_stats[]  - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : ex. this function will copy ether_like_stats[] to nmtr_mgr_if_table_stats[]
 *               ether_like_stats[0] --> nmtr_mgr_if_table_stats[unit/start_port]
 *               ether_like_stats[1] --> nmtr_mgr_if_table_stats[unit/(start_port+1)]
 *                       :                               :
 *               ether_like_stats[start_port-1] --> nmtr_mgr_if_table_stats[unit/(start_port+1)]
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateEtherLikeStats_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num)
{
    UI32_T updated_ifindex;
    UI32_T num_index;
    SWDRV_EtherlikeStats_T ether_like_stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetEtherlikeStats(unit,(start_port+num_index-1), &ether_like_stats);
        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_ether_like_stats[updated_ifindex-1],&ether_like_stats,sizeof (SWDRV_EtherlikeStats_T));
    }
    /*leave critical section
     */
    NMTR_MGR_EXIT_CRITICAL();    
    return TRUE;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateEtherLikePause_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update Rmon statistics in Core OM.
 * INPUT   : UI32_T unit                                - unit
 *           UI32_T start_port                          - start port
 *           UI32_T updated_port_num                    - port num which will be update
 *           SWDRV_EtherlikeStats_T ether_like_stats[]  - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : ex. this function will copy ether_like_stats[] to nmtr_mgr_if_table_stats[]
 *               ether_like_stats[0] --> nmtr_mgr_if_table_stats[unit/start_port]
 *               ether_like_stats[1] --> nmtr_mgr_if_table_stats[unit/(start_port+1)]
 *                       :                               :
 *               ether_like_stats[start_port-1] --> nmtr_mgr_if_table_stats[unit/(start_port+1)]
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateEtherLikePause_CallBack(UI32_T unit, 
                                                 UI32_T start_port, 
                                                 UI32_T updated_port_num)
{
    UI32_T updated_ifindex;
    UI32_T num_index;
    SWDRV_EtherlikePause_T ether_like_pause;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetEtherlikePause(unit,(start_port+num_index-1), &ether_like_pause);
        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_ether_like_pause[updated_ifindex-1],&ether_like_pause,sizeof (SWDRV_EtherlikePause_T));
    }
    /*leave critical section
     */
    NMTR_MGR_EXIT_CRITICAL();    
    return TRUE;
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateIfPerqStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update CoS queue statistics in Core OM.
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_port                 - start port
 *           UI32_T updated_port_num           - port num which will be update
 *           stats[]                           - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateIfPerQStats_CallBack(UI32_T unit, 
                                               UI32_T start_port, 
                                               UI32_T updated_port_num)
{
    UI32_T      updated_ifindex;
    UI32_T      num_index;
    SWDRV_IfPerQStats_T stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetIfPerQStats(unit,(start_port+num_index-1), &stats);

        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_ifperq_stats[updated_ifindex-1], &stats, sizeof(stats));
    }
    NMTR_MGR_EXIT_CRITICAL();
    return TRUE;    
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdatePfcStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update PFC statistics in Core OM.
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_port                 - start port
 *           UI32_T updated_port_num           - port num which will be update
 *           stats[]                           - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdatePfcStats_CallBack(UI32_T unit, 
                                               UI32_T start_port, 
                                               UI32_T updated_port_num)
{
    UI32_T      updated_ifindex;
    UI32_T      num_index;
    SWDRV_PfcStats_T stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetPfcStats(unit,(start_port+num_index-1), &stats);

        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_pfc_stats[updated_ifindex-1], &stats, sizeof(stats));
    }
    NMTR_MGR_EXIT_CRITICAL();
    return TRUE;    
}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateQcnStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update QCN statistics in Core OM.
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_port                 - start port
 *           UI32_T updated_port_num           - port num which will be update
 *           stats[]                           - new counters info
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateQcnStats_CallBack(UI32_T unit, 
                                               UI32_T start_port, 
                                               UI32_T updated_port_num)
{
    UI32_T      updated_ifindex;
    UI32_T      num_index;
    SWDRV_QcnStats_T stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        if (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
            continue;
        }
        NMTRDRV_OM_GetQcnStats(unit,(start_port+num_index-1), &stats);

        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_qcn_stats[updated_ifindex-1], &stats, sizeof(stats));
    }
    NMTR_MGR_EXIT_CRITICAL();
    return TRUE;    
}
#endif /* (SYS_CPNT_CN == TRUE) */


#if (SYS_CPNT_NMTR_HISTORY == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateHistoryXmitBytesByIfindex
 *------------------------------------------------------------------------|
 * FUNCTION: This function will calculate xmit_bytes for counter entry
 * INPUT   : updated_ifindex       - update for which ifindex
 *           counter_p             - counters
 * OUTPUT  : counter_p->ifInUtilization
 *           counter_p->ifOutUtilization
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTR_MGR_UpdateHistoryXmitBytesByIfindex(UI32_T updated_ifindex, NMTR_TYPE_HistCounterInfo_T *counter_p)
{
    Port_Info_T p_info;
    UI32_T bandwidth_kbps = NMTR_MGR_HIST_UTIL_BASE_BANDWIDTH_KBPS_FOR_XMIT_BYTES_STATISTIC;
    UI32_T bandwidth_factor = 0;

    /* To ensure accuracy of utilization,
     * store xmit_bytes instead of utililzation in internal sample entry.
     *
     * Here is to calculate equivalent transmitted bytes (xmit_bytes)
     * for a base bandwidth (bandwidth_kbps)
     *
     *     actual_xmit_bytes     xmit_bytes
     *    ------------------- = ----------------
     *     actual_bandwidth      bandwidth_kbps
     *
     * => xmit_bytes = actual_xmit_bytes / (actual_bandwidth / bandwidth_kbps)
     * 
     */
    if (SWCTRL_GetPortInfo(updated_ifindex, &p_info))
    {
        bandwidth_factor = p_info.bandwidth / bandwidth_kbps;
    }

    counter_p->ifInUtilization.xmit_bytes = 0;
    counter_p->ifOutUtilization.xmit_bytes = 0;

    if (bandwidth_factor)
    {
        UI64_T xmit_bytes;

        xmit_bytes = 0;
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifInUcastPkts);
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifInMulticastPkts);
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifInBroadcastPkts);
        NMTR_MGR_UI64_Multi(&xmit_bytes, 20);
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifInOctets);
        NMTR_MGR_UI64_Div(&xmit_bytes, bandwidth_factor);

        counter_p->ifInUtilization.xmit_bytes = L_STDLIB_UI64_L32(xmit_bytes);

        xmit_bytes = 0;
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifOutUcastPkts);
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifOutMulticastPkts);
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifOutBroadcastPkts);
        NMTR_MGR_UI64_Multi(&xmit_bytes, 20);
        NMTR_MGR_UI64_Add(&xmit_bytes, counter_p->ifOutOctets);
        NMTR_MGR_UI64_Div(&xmit_bytes, bandwidth_factor);

        counter_p->ifOutUtilization.xmit_bytes = L_STDLIB_UI64_L32(xmit_bytes);
    }
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateHistoryByIfindex
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update counter history for an interface
 * INPUT   : updated_ifindex       - update for which ifindex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateHistoryByIfindex(UI32_T updated_ifindex)
{
#define DIFF_IF_COUNTER(_field_) \
            (NMTR_MGR_UpdateDiffCounter( \
                nmtr_mgr_last_hist_counter[updated_ifindex-1]._field_, \
                nmtr_mgr_if_table_stats[updated_ifindex-1]._field_, \
                counter_p->_field_), \
            (nmtr_mgr_last_hist_counter[updated_ifindex-1]._field_ = \
                nmtr_mgr_if_table_stats[updated_ifindex-1]._field_))
#define DIFF_IFX_COUNTER(_field_) \
            (NMTR_MGR_UpdateDiffCounter( \
                nmtr_mgr_last_hist_counter[updated_ifindex-1]._field_, \
                nmtr_mgr_if_xtable_stats[updated_ifindex-1]._field_, \
                counter_p->_field_), \
            (nmtr_mgr_last_hist_counter[updated_ifindex-1]._field_ = \
                nmtr_mgr_if_xtable_stats[updated_ifindex-1]._field_))

    UI32_T      cur_ticks;
    Port_Info_T p_info;
    NMTR_TYPE_HistCounterInfo_T *counter_p = &nmtr_mgr_diff_hist_counter[updated_ifindex-1];

    SYS_TIME_GetSystemUpTimeByTick(&cur_ticks);

    memset(counter_p, 0, sizeof(*counter_p));

    counter_p->start_time = nmtr_mgr_last_hist_counter[updated_ifindex-1].start_time;

    /* When counter_p->start_time is 0, it means that is the first sampling data.
     * Although the value of counter_p->interval is incorrect when start_time
     * is 0, the counter_p->interval will not be updated to
     * nmtr_hist_cur_entry_buf[] because NMTR_MGR_GetDiffHistCounter() will
     * return NULL.
     */
    counter_p->interval = cur_ticks - counter_p->start_time;

    DIFF_IF_COUNTER(ifInOctets);
    DIFF_IF_COUNTER(ifInUcastPkts);
    DIFF_IFX_COUNTER(ifInMulticastPkts);
    DIFF_IFX_COUNTER(ifInBroadcastPkts);
    DIFF_IF_COUNTER(ifInDiscards);
    DIFF_IF_COUNTER(ifInErrors);
    DIFF_IF_COUNTER(ifInUnknownProtos);
    DIFF_IF_COUNTER(ifOutOctets);
    DIFF_IF_COUNTER(ifOutUcastPkts);
    DIFF_IFX_COUNTER(ifOutMulticastPkts);
    DIFF_IFX_COUNTER(ifOutBroadcastPkts);
    DIFF_IF_COUNTER(ifOutDiscards);
    DIFF_IF_COUNTER(ifOutErrors);

    NMTR_MGR_UpdateHistoryXmitBytesByIfindex(updated_ifindex, counter_p);

    nmtr_mgr_last_hist_counter[updated_ifindex-1].start_time = cur_ticks;

    /* for trunk
     * The counter statstics of the trunk is accumulated from the statistics
     * of the difference statistics of its member port, so it only needs to
     * be calculated when counter_p->start_time is not 0(i.e. it is not the
     * first sampling and the difference between this sampling and the previous
     * sampling is available.
     */
    if (counter_p->start_time > 0)
    {
        NMTR_TYPE_HistCounterInfo_T *tmp_counter_p = &nmtr_mgr_diff_hist_counter[SYS_ADPT_TOTAL_NBR_OF_LPORT];
        NMTR_TYPE_HistCounterInfo_T *trunk_counter_p;
        UI32_T unit, port, trunk_id;
        UI32_T updated_trunk_ifindex;
        UI32_T tmp_interval;

        if (SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_LogicalPortToUserPort(updated_ifindex, &unit, &port, &trunk_id))
        {
            SWCTRL_TrunkIDToLogicalPort(trunk_id, &updated_trunk_ifindex);

            trunk_counter_p = &nmtr_mgr_diff_hist_counter[updated_trunk_ifindex-1];

            if (trunk_counter_p->start_time == 0)
            {
                /* take the sampling time of the first trunk member port as
                 * the sampling time of this trunk
                 */
                trunk_counter_p->start_time = nmtr_mgr_last_hist_counter[updated_trunk_ifindex-1].start_time;
                trunk_counter_p->interval = cur_ticks - trunk_counter_p->start_time;

            nmtr_mgr_last_hist_counter[updated_trunk_ifindex-1].start_time = cur_ticks;
        }

            if (trunk_counter_p->start_time > 0)
            {
                *tmp_counter_p = *counter_p;

                NMTR_MGR_UpdateHistoryXmitBytesByIfindex(updated_trunk_ifindex, tmp_counter_p);

                tmp_interval = trunk_counter_p->interval;

                NMTR_HIST_AccumulateCounter(trunk_counter_p, tmp_counter_p);

                /* keep the interval of the first trunk member port as
                 * the interval of this trunk
                 */
                trunk_counter_p->interval = tmp_interval;
            }
        }
    }

    return TRUE;

#undef DIFF_IF_COUNTER
#undef DIFF_IFX_COUNTER
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_GetDiffHistCounter
 *------------------------------------------------------------------------|
 * FUNCTION: To retrieve diff counter for specified port
 * INPUT   : ifindex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Return NULL if the start_time field of the corresponding entry
 *           is nmtr_mgr_diff_hist_counter 0. When the start_time is 0,
 *           it means that is the first sampling data and the difference
 *           counter between this sampling and previous sampling is not
 *           avilable.
 *------------------------------------------------------------------------*/
static NMTR_TYPE_HistCounterInfo_T *NMTR_MGR_GetDiffHistCounter(UI32_T ifindex)
{
    NMTR_TYPE_HistCounterInfo_T *counter_p = &nmtr_mgr_diff_hist_counter[ifindex-1];

    if (counter_p->start_time == 0)
    {
        return FALSE;
    }

    return counter_p;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateHistory_CallBack
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update counter history
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_port                 - start port
 *           UI32_T updated_port_num           - port num which will be update
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateHistory_CallBack(UI32_T unit,
                                                 UI32_T start_port,
                                                 UI32_T updated_port_num)
{
    UI32_T      updated_ifindex;
    UI32_T      num_index;
    UI32_T      trunk_id, trunk_ifindex;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();

    /* reset diff counters
     */
    memset(nmtr_mgr_diff_hist_counter, 0, sizeof(nmtr_mgr_diff_hist_counter));

    /* update diff counters
     */
    for (num_index = 1;num_index<=updated_port_num;num_index++)
    {
        switch (SWCTRL_UserPortToIfindex(unit, (start_port+num_index-1), &updated_ifindex))
        {
            case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            case SWCTRL_LPORT_NORMAL_PORT:
                NMTR_MGR_UpdateHistoryByIfindex(updated_ifindex);
                break;

            default:
                /* do nothing */;
        }
    }

    /* update ctrl entries with diff counters
     */
    NMTR_HIST_UpdateAllCtrlEntryCounter(NMTR_MGR_GetDiffHistCounter);

    NMTR_MGR_EXIT_CRITICAL();
    return TRUE;
}
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateIFXTableStatsForVlan_CallBack
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update IFX table statistics in Core OM.
 * INPUT   : UI32_T unit                       - unit
 *           UI32_T start_vid                  - start vid
 *           UI32_T updated_num                - num which will be update
 *           SWDRV_IfXTableStats_T ifX_stats[] - new counters info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T NMTR_MGR_UpdateIFXTableStatsForVlan_CallBack(UI32_T unit, 
                                                 UI32_T start_vid, 
                                                 UI32_T updated_num)
{
    UI32_T vid;
    SWDRV_IfXTableStats_T stats, local_unit_stats;

    /*enter critical section
     */
    NMTR_MGR_ENTER_CRITICAL();    
    
    for (vid = start_vid; vid < start_vid + updated_num; vid++)
    {
        memset(&stats, 0, sizeof(stats));

        for (unit = 0; STKTPLG_OM_GetNextUnit(&unit); )
        {
            NMTRDRV_OM_GetIfXStatsForVlan(unit, vid, 1, &local_unit_stats);
            NMTR_MGR_SumOfIfXTableStats(&stats, local_unit_stats);
        }

        /* count the different
         */
        /* Not implemented */

        /* update new if table statistics
         */
        memcpy(&nmtr_mgr_if_xtable_stats_for_vlan[vid-1], &stats, sizeof(*nmtr_mgr_if_xtable_stats_for_vlan));

        /* Update Utilization
         * already in NMTR_MGR_UpdateIFTableStats_CallBack().
         */
    }
    /*leave critical section
     */
    NMTR_MGR_EXIT_CRITICAL();    
    return TRUE;
}
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateEtherLikeStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update Rmon statistics in Core OM.
 * INPUT   : UI64_T old_var
 *           UI64_T new_var
 * OUTPUT  : UI64_T *diff_var                                                         
 * RETURN  : None                                                         
 * NOTE    : diff_var = |new_var - old_var|
 *------------------------------------------------------------------------*/
static void NMTR_MGR_DiffTwoUI64Var(UI64_T old_var, UI64_T new_var, UI64_T *diff_var)
{
    NMTR_MGR_UI64_Sub(&new_var, old_var);
    *diff_var = new_var;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTR_MGR_UpdateEtherLikeStats_CallBack                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will update Rmon statistics in Core OM.
 * INPUT   : UI64_T var1
 *           UI64_T var2
 *           UI64_T var3
 * OUTPUT  : UI64_T *total_var                                                         
 * RETURN  : None                                                         
 * NOTE    : diff_var = |new_var - old_var|
 *------------------------------------------------------------------------*/
static void NMTR_MGR_SumThreeUI64Var(UI64_T var1, UI64_T var2, UI64_T var3, UI64_T *total_var)
{
    NMTR_MGR_UI64_Add(&var1, var2);
    NMTR_MGR_UI64_Add(&var1, var3);
    *total_var = var1;
}

static void NMTR_MGR_Backdoor_Menu(void)
{
    char    buf[16];
    int     ch;
    BOOL_T  exit;

    exit = FALSE;
    for(; !exit;)
    {
        BACKDOOR_MGR_Print("\r\n    NMTR_MGR BACKDOOR MENU");
        BACKDOOR_MGR_Print("\r\n=============================");
        BACKDOOR_MGR_Print("\r\n0. Exit");
        BACKDOOR_MGR_Print("\r\n1. Show unit/port IF Info");
        BACKDOOR_MGR_Print("\r\n2. Show unit/port IFX Info");
        BACKDOOR_MGR_Print("\r\n3. Show unit/port Rmon Info");
        BACKDOOR_MGR_Print("\r\n4. Show unit/port EtherLike Info");
        BACKDOOR_MGR_Print("\r\n5. Show unit/port Utilization300secs");
        BACKDOOR_MGR_Print("\r\n6. Show unit/port Utilization");
        BACKDOOR_MGR_Print("\r\n7. Show unit/port SystemwideIfXTableStats");        
        BACKDOOR_MGR_Print("\r\n8. Show unit/port EtherLike Pause Info");
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        BACKDOOR_MGR_Print("\r\n9. Show unit/port IfPerQ Info");
#endif
#if (SYS_CPNT_PFC == TRUE)
        BACKDOOR_MGR_Print("\r\n10. Show unit/port PFC Info");
#endif
#if (SYS_CPNT_CN == TRUE)
        BACKDOOR_MGR_Print("\r\n11. Show unit/port QCN Info");
#endif
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
        BACKDOOR_MGR_Print("\r\n12. History");
#endif
        BACKDOOR_MGR_Print("\r\nEnter you selection: ");

        BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
        BACKDOOR_MGR_Print("\r\n");

        if (1 != sscanf(buf, "%d", &ch))
            continue;

        switch(ch)
        {
            case 0:
                exit = TRUE;
                break;
            case 1:
                NMTR_MGR_BD_ShowUnitPortIFInfo();
                break;
            case 2:
                NMTR_MGR_BD_ShowUnitPortIFXInfo();
                break;
            case 3:
                NMTR_MGR_BD_ShowUnitPortRmonInfo();
                break;
            case 4:
                NMTR_MGR_BD_ShowUnitPortEtherLikeInfo();
                break;             
            case 5:
                NMTR_MGR_BD_ShowUnitPortUtilization300secs();
                break;                                
            case 6:
                NMTR_MGR_BD_ShowUnitPortUtilization();
                break;                                
            case 7:
                NMTR_MGR_BD_ShowSystemwideIfXTableStats();
                break;                                
            case 8:
                NMTR_MGR_BD_ShowUnitPortEtherLikePauseInfo();
                break;             
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
            case 9:
                NMTR_MGR_BD_ShowUnitPortIfPerQInfo();
                break;
#endif
#if (SYS_CPNT_PFC == TRUE)
            case 10:
                NMTR_MGR_BD_ShowUnitPortPfcInfo();
                break;
#endif
#if (SYS_CPNT_CN == TRUE)
            case 11:
                NMTR_MGR_BD_ShowUnitPortQcnInfo();
                break;
#endif
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
            case 12:
                NMTR_HIST_Backdoor();
                break;
#endif
            default:
                break;
        }
    }
}

static void NMTR_MGR_BD_ShowUnitPortIFInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_IfTableStats_T if_table_stats;
    
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }
    
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetIfTableStats(ifindex, &if_table_stats);

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifInOctets),L_STDLIB_UI64_L32(if_table_stats.ifInOctets),str);    
    BACKDOOR_MGR_Printf("\r\nifInOctets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifInUcastPkts),L_STDLIB_UI64_L32(if_table_stats.ifInUcastPkts),str);    
    BACKDOOR_MGR_Printf("\r\nifInUcastPkts:      %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifInNUcastPkts),L_STDLIB_UI64_L32(if_table_stats.ifInNUcastPkts),str);    
    BACKDOOR_MGR_Printf("\r\nifInNUcastPkts:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifInDiscards),L_STDLIB_UI64_L32(if_table_stats.ifInDiscards),str);    
    BACKDOOR_MGR_Printf("\r\nifInDiscards:       %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifInErrors),L_STDLIB_UI64_L32(if_table_stats.ifInErrors),str);    
    BACKDOOR_MGR_Printf("\r\nifInErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifInUnknownProtos),L_STDLIB_UI64_L32(if_table_stats.ifInUnknownProtos),str);    
    BACKDOOR_MGR_Printf("\r\nifInUnknownProtos:  %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifOutOctets),L_STDLIB_UI64_L32(if_table_stats.ifOutOctets),str);    
    BACKDOOR_MGR_Printf("\r\nifOutOctets:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifOutUcastPkts),L_STDLIB_UI64_L32(if_table_stats.ifOutUcastPkts),str);    
    BACKDOOR_MGR_Printf("\r\nifOutUcastPkts:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifOutNUcastPkts),L_STDLIB_UI64_L32(if_table_stats.ifOutNUcastPkts),str);    
    BACKDOOR_MGR_Printf("\r\nifOutNUcastPkts:    %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifOutDiscards),L_STDLIB_UI64_L32(if_table_stats.ifOutDiscards),str);    
    BACKDOOR_MGR_Printf("\r\nifOutDiscards:      %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifOutErrors),L_STDLIB_UI64_L32(if_table_stats.ifOutErrors),str);    
    BACKDOOR_MGR_Printf("\r\nifOutErrors:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_table_stats.ifOutQLen),L_STDLIB_UI64_L32(if_table_stats.ifOutQLen),str);    
    BACKDOOR_MGR_Printf("\r\nifOutQLen:          %s", str);
    return;

}

static void NMTR_MGR_BD_ShowUnitPortIFXInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_IfXTableStats_T if_xtable_stats;
    
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }
    
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetIfXTableStats(ifindex,&if_xtable_stats);

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifInMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifInMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInMulticastPkts:      %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifInBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifInBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInBroadcastPkts:      %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifOutMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifOutMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutMulticastPkts:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifOutBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifOutBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutBroadcastPkts:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInOctets),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInOctets),str);
    BACKDOOR_MGR_Printf("\r\nifHCInOctets:           %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInUcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInUcastPkts:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInMulticastPkts:    %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInBroadcastPkts:    %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutOctets),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutOctets),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutOctets:          %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutUcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutUcastPkts:       %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutMulticastPkts:   %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutBroadcastPkts:   %s", str);
    return;

}
    
static void NMTR_MGR_BD_ShowUnitPortRmonInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_RmonStats_T rmon_stats;
    
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }
    
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetRmonStats(ifindex,&rmon_stats);

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsDropEvents),L_STDLIB_UI64_L32(rmon_stats.etherStatsDropEvents),str);
    BACKDOOR_MGR_Printf("\r\netherStatsDropEvents:           %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsOctets),L_STDLIB_UI64_L32(rmon_stats.etherStatsOctets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsOctets:               %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts:                 %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsBroadcastPkts),L_STDLIB_UI64_L32(rmon_stats.etherStatsBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsBroadcastPkts:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsMulticastPkts),L_STDLIB_UI64_L32(rmon_stats.etherStatsMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsMulticastPkts:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsCRCAlignErrors),L_STDLIB_UI64_L32(rmon_stats.etherStatsCRCAlignErrors),str);
    BACKDOOR_MGR_Printf("\r\netherStatsCRCAlignErrors:       %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsUndersizePkts),L_STDLIB_UI64_L32(rmon_stats.etherStatsUndersizePkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsUndersizePkts:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsOversizePkts),L_STDLIB_UI64_L32(rmon_stats.etherStatsOversizePkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsOversizePkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsFragments),L_STDLIB_UI64_L32(rmon_stats.etherStatsFragments),str);
    BACKDOOR_MGR_Printf("\r\netherStatsFragments:            %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsJabbers),L_STDLIB_UI64_L32(rmon_stats.etherStatsJabbers),str);
    BACKDOOR_MGR_Printf("\r\netherStatsJabbers:              %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsCollisions),L_STDLIB_UI64_L32(rmon_stats.etherStatsCollisions),str);
    BACKDOOR_MGR_Printf("\r\netherStatsCollisions:           %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts64Octets),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts64Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts64Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts65to127Octets),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts65to127Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts65to127Octets:    %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts128to255Octets),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts128to255Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts128to255Octets:   %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts256to511Octets),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts256to511Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts256to511Octets:   %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts512to1023Octets),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts512to1023Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts512to1023Octets:  %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(rmon_stats.etherStatsPkts1024to1518Octets),L_STDLIB_UI64_L32(rmon_stats.etherStatsPkts1024to1518Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts1024to1518Octets: %s", str);
    return;

}
    
static void NMTR_MGR_BD_ShowUnitPortEtherLikeInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_EtherlikeStats_T ether_like_stats;
        
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }

    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetEtherLikeStats(ifindex, &ether_like_stats);
    
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsAlignmentErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsAlignmentErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsAlignmentErrors:           %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsFCSErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsFCSErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsFCSErrors:                 %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsSingleCollisionFrames),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsSingleCollisionFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsSingleCollisionFrames:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsMultipleCollisionFrames),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsMultipleCollisionFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsMultipleCollisionFrames:   %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsSQETestErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsSQETestErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsSQETestErrors:             %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsDeferredTransmissions),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsDeferredTransmissions),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsDeferredTransmissions:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsLateCollisions),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsLateCollisions),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsLateCollisions:            %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsExcessiveCollisions),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsExcessiveCollisions),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsExcessiveCollisions:       %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsInternalMacTransmitErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsInternalMacTransmitErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsInternalMacTransmitErrors: %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsCarrierSenseErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsCarrierSenseErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsCarrierSenseErrors:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsFrameTooLongs),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsFrameTooLongs),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsFrameTooLongs:             %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsInternalMacReceiveErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsInternalMacReceiveErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsInternalMacReceiveErrors:  %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsSymbolErrors),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsSymbolErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsSymbolErrors:              %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_stats.dot3StatsDuplexStatus),L_STDLIB_UI64_L32(ether_like_stats.dot3StatsDuplexStatus),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsDuplexStatus:              %s", str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsRateControlAbility:        %ld", ether_like_stats.dot3StatsRateControlAbility);
    BACKDOOR_MGR_Printf("\r\ndot3StatsRateControlStatus:         %ld", ether_like_stats.dot3StatsRateControlStatus);
    return;

} 

static void NMTR_MGR_BD_ShowUnitPortUtilization300secs(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    NMTR_MGR_Utilization_300_SECS_T utilization_300_secs;
        
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }

    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetPortUtilization300secs(ifindex, &utilization_300_secs);

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(utilization_300_secs.ifInOctets),L_STDLIB_UI64_L32(utilization_300_secs.ifInOctets),str);
    BACKDOOR_MGR_Printf("\r\nifInOctets:             %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(utilization_300_secs.ifOutOctets),L_STDLIB_UI64_L32(utilization_300_secs.ifOutOctets),str);
    BACKDOOR_MGR_Printf("\r\nifOutOctets:            %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(utilization_300_secs.ifInPackets),L_STDLIB_UI64_L32(utilization_300_secs.ifInPackets),str);
    BACKDOOR_MGR_Printf("\r\nifInPackets:            %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(utilization_300_secs.ifOutPackets),L_STDLIB_UI64_L32(utilization_300_secs.ifOutPackets),str);
    BACKDOOR_MGR_Printf("\r\nifOutPackets:           %s", str);
    BACKDOOR_MGR_Printf("\r\nifInOctets_utilization: %ld", utilization_300_secs.ifInOctets_utilization);
    BACKDOOR_MGR_Printf("\r\nifOutOctets_utilization:%ld", utilization_300_secs.ifOutOctets_utilization);
    return;
}

static void NMTR_MGR_BD_ShowUnitPortUtilization(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    NMTR_MGR_Utilization_T utilization_entry;
        
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }

    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetPortUtilization(ifindex, &utilization_entry);

    BACKDOOR_MGR_Printf("\r\nifInOctets_utilization:         %ld", utilization_entry.ifInOctets_utilization);
    BACKDOOR_MGR_Printf("\r\nifInUcastPkts_utilization:      %ld", utilization_entry.ifInUcastPkts_utilization);
    BACKDOOR_MGR_Printf("\r\nifInMulticastPkts_utilization:  %ld", utilization_entry.ifInMulticastPkts_utilization);
    BACKDOOR_MGR_Printf("\r\nifInBroadcastPkts_utilization:  %ld", utilization_entry.ifInBroadcastPkts_utilization);
    BACKDOOR_MGR_Printf("\r\nifInErrors_utilization:         %ld", utilization_entry.ifInErrors_utilization);
    return;
}

static void NMTR_MGR_BD_ShowSystemwideIfXTableStats(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_IfXTableStats_T if_xtable_stats;
        
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }

    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetSystemwideIfXTableStats(ifindex, &if_xtable_stats);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifInMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifInMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInMulticastPkts:      %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifInBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifInBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInBroadcastPkts:      %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifOutMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifOutMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutMulticastPkts:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifOutBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifOutBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutBroadcastPkts:     %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInOctets),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInOctets),str);
    BACKDOOR_MGR_Printf("\r\nifHCInOctets:           %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInUcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInUcastPkts:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInMulticastPkts:    %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCInBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCInBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInBroadcastPkts:    %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutOctets),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutOctets),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutOctets:          %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutUcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutUcastPkts:       %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutMulticastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutMulticastPkts:   %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(if_xtable_stats.ifHCOutBroadcastPkts),L_STDLIB_UI64_L32(if_xtable_stats.ifHCOutBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutBroadcastPkts:   %s", str);
    return;
}

static void NMTR_MGR_BD_ShowUnitPortEtherLikePauseInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_EtherlikePause_T ether_like_pause;

    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }

    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetEtherLikePause(ifindex, &ether_like_pause);

    BACKDOOR_MGR_Printf("\r\ndot3PauseAdminMode:        %lu", ether_like_pause.dot3PauseAdminMode);
    BACKDOOR_MGR_Printf("\r\ndot3PauseOperMode:         %lu", ether_like_pause.dot3PauseOperMode);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_pause.dot3InPauseFrames),L_STDLIB_UI64_L32(ether_like_pause.dot3InPauseFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3InPauseFrames:           %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(ether_like_pause.dot3OutPauseFrames),L_STDLIB_UI64_L32(ether_like_pause.dot3InPauseFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3OutPauseFrames:          %s", str);
    return;
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void NMTR_MGR_BD_ShowUnitPortIfPerQInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_IfPerQStats_T stats;
    int i;
    
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }
    
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetIfPerQStats(ifindex, &stats);

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
    {
        BACKDOOR_MGR_Printf("\r\nQueue: %d", i);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.cosq[i].ifOutOctets),L_STDLIB_UI64_L32(stats.cosq[i].ifOutOctets),str);    
        BACKDOOR_MGR_Printf("\r\nifOutOctets:               %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.cosq[i].ifOutPkts),L_STDLIB_UI64_L32(stats.cosq[i].ifOutPkts),str);    
        BACKDOOR_MGR_Printf("\r\nifOutPkts:                 %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.cosq[i].ifOutDiscardOctets),L_STDLIB_UI64_L32(stats.cosq[i].ifOutDiscardOctets),str);    
        BACKDOOR_MGR_Printf("\r\nifOutDiscardOctets:        %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.cosq[i].ifOutDiscardPkts),L_STDLIB_UI64_L32(stats.cosq[i].ifOutDiscardPkts),str);    
        BACKDOOR_MGR_Printf("\r\nifOutDiscardPkts:          %s", str);
    }

    return;

}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
static void NMTR_MGR_BD_ShowUnitPortPfcInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_PfcStats_T stats;
    int i;
    
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }
    
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetPfcStats(ifindex, &stats);

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.ieee8021PfcRequests),L_STDLIB_UI64_L32(stats.ieee8021PfcRequests),str);    
    BACKDOOR_MGR_Printf("\r\nieee8021PfcRequests:       %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.ieee8021PfcIndications),L_STDLIB_UI64_L32(stats.ieee8021PfcIndications),str);    
    BACKDOOR_MGR_Printf("\r\nieee8021PfcIndications:    %s", str);

    for (i = 0; i < 8; i++)
    {
        BACKDOOR_MGR_Printf("\r\nPriority: %d", i);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.pri[i].ieee8021PfcRequests),L_STDLIB_UI64_L32(stats.pri[i].ieee8021PfcRequests),str);    
        BACKDOOR_MGR_Printf("\r\nieee8021PfcRequests:       %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.pri[i].ieee8021PfcIndications),L_STDLIB_UI64_L32(stats.pri[i].ieee8021PfcIndications),str);    
        BACKDOOR_MGR_Printf("\r\nieee8021PfcIndications:    %s", str);
    }

    return;

}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
static void NMTR_MGR_BD_ShowUnitPortQcnInfo(void)
{
    UI32_T unit;
    UI32_T port;
    UI32_T ifindex;
    UI8_T buf[3];
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    SWDRV_QcnStats_T stats;
    int i;
    
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    unit = atoi((char*)buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    port = atoi((char*)buf);
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }
    
    if (SWCTRL_UserPortToIfindex(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        BACKDOOR_MGR_Print("\r\n Unit-Port does not exist");
        return;
    }    

    NMTR_MGR_GetQcnStats(ifindex, &stats);

    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        BACKDOOR_MGR_Printf("\r\nCP Queue: %d", i);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stats.cpq[i].qcnStatsOutCnms),L_STDLIB_UI64_L32(stats.cpq[i].qcnStatsOutCnms),str);    
        BACKDOOR_MGR_Printf("\r\nqcnStatsOutCnms:           %s", str);
    }

    return;

}
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_NMTR_SYNC_NDEV == TRUE)
#define NMTR_MGR_SINGAL_ENUM(a,b)    a,
#define NMTR_MGR_SINGAL_NAME(a,b)    #a,
#define NMTR_MGR_SYSFS_ATTR(a,b)     b,

/* enum, sysfs attr
 */
#define NMTR_MGR_SYSFS_UPD_FLDE_LST(_)                      \
    _(NMTR_MGR_SYSFS_UPD_FLDE_RXPKTS,    "rx_packets")      \
    _(NMTR_MGR_SYSFS_UPD_FLDE_RXBYTES,   "rx_bytes")        \
    _(NMTR_MGR_SYSFS_UPD_FLDE_RXERRS,    "rx_errors")       \
    _(NMTR_MGR_SYSFS_UPD_FLDE_TXPKTS,    "tx_packets")      \
    _(NMTR_MGR_SYSFS_UPD_FLDE_TXBYTES,   "tx_bytes")        \
    _(NMTR_MGR_SYSFS_UPD_FLDE_TXERRS,    "tx_errors")

enum NMTR_MGR_SYSFS_UpdId_E
{
    NMTR_MGR_SYSFS_UPD_FLDE_LST(NMTR_MGR_SINGAL_ENUM)
    NMTR_MGR_SYSFS_UPD_FLDE_MAX,         /* maximum no, for boundary checking  */
};

#define SYSFS_ATTR_PATH_STR_FMT     "/sys/class/net/%s/statistics/"
#define SYSFS_NDEV_STR_FMT_PORT     "swp%ld" /* refer toSTKTPLG_OM_InitiateSystemResources */
#define SYSFS_NDEV_STR_FMT_PCH      "swb%ld"

static char *nmtr_sysfs_attr_tbl[] = { NMTR_MGR_SYSFS_UPD_FLDE_LST(NMTR_MGR_SYSFS_ATTR) };

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetSysfsAttrPath
 * ------------------------------------------------------------------------
 * PURPOSE  : To get sysfs path string of input ifidx.
 * INPUT    : is_trk      - TRUE if ifidx is trunk
 *            ifidx       - ifidx to get
 * OUTPUT   : attr_path_p - pointer to output path string
 * RETURN   : lenght of path string
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static UI32_T NMTR_MGR_GetSysfsAttrPath(
    BOOL_T  is_trk,
    UI32_T  ifidx,
    char    *attr_path_p)
{
    UI32_T  ret_len =0;
    char    *ndev_str_fmt = SYSFS_NDEV_STR_FMT_PORT;
    char    inf_buf[10];

    if (NULL != attr_path_p)
    {
        if (TRUE == is_trk)
            ndev_str_fmt = SYSFS_NDEV_STR_FMT_PCH;

        sprintf(inf_buf, ndev_str_fmt, ifidx);

        ret_len = sprintf(attr_path_p, SYSFS_ATTR_PATH_STR_FMT, inf_buf);
    }

    return ret_len;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetFqSysfsAttrFname
 * ------------------------------------------------------------------------
 * PURPOSE  : To get fully qualified file name string of input attr_tag_p.
 * INPUT    : attr_tag_p  - pointer to input attribute tag
 *            attr_path_p - pointer to output path string
 *            path_len    - lenght of path string
 * OUTPUT   : attr_path_p - pointer to output fully qualified file name
 * RETURN   : pointer to output path string
 * NOTES    : file name ex: /sys/class/net/{inf}/statistics/rx_packets
 * ------------------------------------------------------------------------
 */
static char *NMTR_MGR_GetFqSysfsAttrFname(
    char    *attr_tag_p,
    char    *attr_path_p,
    int     path_len)
{
    if ((NULL != attr_tag_p) && (NULL != attr_path_p))
    {
        sprintf(&attr_path_p[path_len], "%s", attr_tag_p);
    }

    return attr_path_p;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_GetUpdValueByFldId
 * ------------------------------------------------------------------------
 * PURPOSE  : To get value of specified field id from if_stats.
 * INPUT    : fld_id    - field id to get
 *            if_stat_p - pointer to input if_stats
 * OUTPUT   : val_p     - pointer to output value
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T NMTR_MGR_GetUpdValueByFldId(
    UI32_T                  fld_id,
    SWDRV_IfTableStats_T    *if_stat_p,
    UI64_T                  *val_p)
{
    BOOL_T  ret = FALSE;

    if ((NULL != if_stat_p) && (NULL != val_p))
    {
        switch (fld_id)
        {
        case NMTR_MGR_SYSFS_UPD_FLDE_RXPKTS:
            *val_p = if_stat_p->ifInNUcastPkts + if_stat_p->ifInUcastPkts;
            break;
        case NMTR_MGR_SYSFS_UPD_FLDE_RXBYTES:
            *val_p = if_stat_p->ifInOctets;
            break;
        case NMTR_MGR_SYSFS_UPD_FLDE_RXERRS:
            *val_p = if_stat_p->ifInErrors;
            break;
        case NMTR_MGR_SYSFS_UPD_FLDE_TXPKTS:
            *val_p = if_stat_p->ifOutNUcastPkts + if_stat_p->ifOutUcastPkts;
            break;
        case NMTR_MGR_SYSFS_UPD_FLDE_TXBYTES:
            *val_p = if_stat_p->ifOutOctets;
            break;
        case NMTR_MGR_SYSFS_UPD_FLDE_TXERRS:
            *val_p = if_stat_p->ifOutErrors;
            break;
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_SysfsAttrWrite
 * ------------------------------------------------------------------------
 * PURPOSE  : To write value to specified sysfs attribute path.
 * INPUT    : attr_fqfname_p - fully qualified file name of sysfs attribute
 *            val            - value to write
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T NMTR_MGR_SysfsAttrWrite(
    char    *attr_fqfname_p,
    UI64_T  val)
{
    FILE    *fh;
    UI32_T  len;
    char    write_buf[40];
    BOOL_T  ret = FALSE;

    fh = fopen(attr_fqfname_p, "w");
    if (fh != NULL)
    {
        len = sprintf(write_buf, "%lu", (UI32_T) (val & 0xffffffff));

        if (fwrite(write_buf, 1, len, fh) == len)
        {
            ret = TRUE;
        }

        fclose(fh);
    }

    if (FALSE == ret)
    {
#if 0
        BACKDOOR_MGR_Printf("%s:%d %s fail\r\n",
            __FUNCTION__, __LINE__, attr_fqfname_p);
#endif
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_ResetNetDevStatistics
 * ------------------------------------------------------------------------
 * PURPOSE  : To reset statistics of sysfs attribute of netdevice.
 * INPUT    : ifidx - ifidx to reset
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void NMTR_MGR_ResetNetDevStatistics(UI32_T ifidx)
{
    SWDRV_IfTableStats_T    if_stats;
    UI64_T                  tmp_val;
    UI32_T                  path_len, tmp_ifidx, fld_id;
    char                    if_attr_sbuf[60];

    memset (&if_stats, 0, sizeof(if_stats));

    if (SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER <= ifidx)
    {
        tmp_ifidx = ifidx - SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER +1;

        path_len = NMTR_MGR_GetSysfsAttrPath(TRUE, tmp_ifidx, if_attr_sbuf);
    }
    else
    {
        path_len = NMTR_MGR_GetSysfsAttrPath(FALSE, ifidx, if_attr_sbuf);
    }

    if (path_len == 0)
        return;

    for (fld_id =0; fld_id <NMTR_MGR_SYSFS_UPD_FLDE_MAX; fld_id++)
    {
        tmp_val = 0;

        NMTR_MGR_GetFqSysfsAttrFname(
            nmtr_sysfs_attr_tbl[fld_id], if_attr_sbuf, path_len);

        NMTR_MGR_GetUpdValueByFldId(fld_id, &if_stats, &tmp_val);

        NMTR_MGR_SysfsAttrWrite(if_attr_sbuf, tmp_val);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NMTR_MGR_UpdateNetDevStatistics_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : To sync NMTR statistics to sysfs attribute of netdevice.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void NMTR_MGR_UpdateNetDevStatistics_CallBack(void)
{
    SWDRV_IfTableStats_T    if_stats;
    UI64_T                  tmp_val;
    UI32_T                  path_len, upd_ifidx,
                            fld_id;
    char                    if_attr_sbuf[60];

    for (upd_ifidx =1; upd_ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT; upd_ifidx++)
    {
        UI32_T  tmp_ifidx;

        if (FALSE == NMTR_MGR_GetIfTableStats(upd_ifidx, &if_stats))
            continue;

        if (SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER <= upd_ifidx)
        {
            tmp_ifidx = upd_ifidx - SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER +1;

            path_len = NMTR_MGR_GetSysfsAttrPath(TRUE, tmp_ifidx, if_attr_sbuf);
        }
        else
        {
            path_len = NMTR_MGR_GetSysfsAttrPath(FALSE, upd_ifidx, if_attr_sbuf);
        }

        if (path_len == 0)
            continue;

        for (fld_id =0; fld_id <NMTR_MGR_SYSFS_UPD_FLDE_MAX; fld_id++)
        {
            tmp_val = 0;

            NMTR_MGR_GetFqSysfsAttrFname(
                nmtr_sysfs_attr_tbl[fld_id], if_attr_sbuf, path_len);

            NMTR_MGR_GetUpdValueByFldId(fld_id, &if_stats, &tmp_val);

            NMTR_MGR_SysfsAttrWrite(if_attr_sbuf, tmp_val);
        }
    }
}
#endif /* #if (SYS_CPNT_NMTR_SYNC_NDEV == TRUE) */

/*-----------------------------------------------------------------------*
 *                        end nmtr_mgr.c                                     *
 *-----------------------------------------------------------------------*/
 

