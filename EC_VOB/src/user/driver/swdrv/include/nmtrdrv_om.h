#ifndef _NMTRDRV_OM_H
#define _NMTRDRV_OM_H

#include "sys_type.h"
#include "sysrsc_mgr.h"
#include "swdrv_type.h"

void NMTRDRV_OM_InitiateSystemResources(void);

void NMTRDRV_OM_AttachSystemResources(void);

void NMTRDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

SYS_TYPE_Stacking_Mode_T NMTRDRV_OM_GetOperatingMode(void);

void NMTRDRV_OM_SetTransitionMode(void);

void NMTRDRV_OM_EnterTransitionMode(void);

void NMTRDRV_OM_EnterMasterMode(void);

void NMTRDRV_OM_EnterSlaveMode(void);

UI32_T NMTRDRV_OM_GetMyUnitId(void);

void NMTRDRV_OM_SetMyUnitId(UI32_T my_unit_id);

BOOL_T NMTRDRV_OM_GetDebugFlag(void);

void NMTRDRV_OM_SetDebugFlag(BOOL_T debug_flag);

BOOL_T NMTRDRV_OM_GetProvisionComplete(void);

void NMTRDRV_OM_SetProvisionComplete(BOOL_T provision_complete);

void NMTRDRV_OM_ClearIfStats(UI32_T lunit,UI32_T lport);

void NMTRDRV_OM_SetIfStats(UI32_T lunit,UI32_T lport, SWDRV_IfTableStats_T *if_stats_p);
void NMTRDRV_OM_GetIfStats(UI32_T lunit,UI32_T lport, SWDRV_IfTableStats_T *if_stats_p);

void NMTRDRV_OM_ClearIfXStats(UI32_T lunit,UI32_T lport);

void NMTRDRV_OM_SetIfXStats(UI32_T lunit,UI32_T lport, SWDRV_IfXTableStats_T *ifx_stats_p);
void NMTRDRV_OM_GetIfXStats(UI32_T lunit,UI32_T lport, SWDRV_IfXTableStats_T *ifx_stats_p);

void NMTRDRV_OM_ClearRmonStats(UI32_T lunit,UI32_T lport);

void NMTRDRV_OM_SetRmonStats(UI32_T lunit,UI32_T lport, SWDRV_RmonStats_T *rmon_stats_p);
void NMTRDRV_OM_GetRmonStats(UI32_T lunit,UI32_T lport, SWDRV_RmonStats_T *rmon_stats_p);

void NMTRDRV_OM_ClearEtherlikeStats(UI32_T lunit,UI32_T lport);

void NMTRDRV_OM_SetEtherlikeStats(UI32_T lunit,UI32_T lport, SWDRV_EtherlikeStats_T *etherlike_stats_p);
void NMTRDRV_OM_GetEtherlikeStats(UI32_T lunit,UI32_T lport, SWDRV_EtherlikeStats_T *etherlike_stats_p);

void NMTRDRV_OM_ClearEtherlikePause(UI32_T lunit,UI32_T lport);

void NMTRDRV_OM_SetEtherlikePause(UI32_T lunit,UI32_T lport, SWDRV_EtherlikePause_T *etherlike_pause_p);
void NMTRDRV_OM_GetEtherlikePause(UI32_T lunit,UI32_T lport, SWDRV_EtherlikePause_T *etherlike_pause_p);

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
void NMTRDRV_OM_ClearIfPerQStats(UI32_T lunit,UI32_T lport);
void NMTRDRV_OM_SetIfPerQStats(UI32_T lunit,UI32_T lport, SWDRV_IfPerQStats_T *stats_p);
void NMTRDRV_OM_GetIfPerQStats(UI32_T lunit,UI32_T lport, SWDRV_IfPerQStats_T *stats_p);
#endif

#if (SYS_CPNT_PFC == TRUE)
void NMTRDRV_OM_ClearPfcStats(UI32_T lunit,UI32_T lport);
void NMTRDRV_OM_SetPfcStats(UI32_T lunit,UI32_T lport, SWDRV_PfcStats_T *stats_p);
void NMTRDRV_OM_GetPfcStats(UI32_T lunit,UI32_T lport, SWDRV_PfcStats_T *stats_p);
#endif

#if (SYS_CPNT_CN == TRUE)
void NMTRDRV_OM_ClearQcnStats(UI32_T lunit,UI32_T lport);
void NMTRDRV_OM_SetQcnStats(UI32_T lunit,UI32_T lport, SWDRV_QcnStats_T *stats_p);
void NMTRDRV_OM_GetQcnStats(UI32_T lunit,UI32_T lport, SWDRV_QcnStats_T *stats_p);
#endif

void NMTRDRV_OM_ClearIfXStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T num_of_vid);
void NMTRDRV_OM_SetIfXStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T num_of_vid, SWDRV_IfXTableStats_T *ifx_stats_p);
void NMTRDRV_OM_GetIfXStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T num_of_vid, SWDRV_IfXTableStats_T *ifx_stats_p);

#endif

