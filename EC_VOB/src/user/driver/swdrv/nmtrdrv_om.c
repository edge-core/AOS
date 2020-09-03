#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "nmtrdrv_om.h"
#include "sysrsc_mgr.h"
#include "string.h"

#define NMTRDRV_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(nmtrdrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define NMTRDRV_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(nmtrdrv_om_sem_id)
#define ATOM_EXPRESSION(exp) { NMTRDRV_OM_ENTER_CRITICAL_SECTION();\
                               exp;\
                               NMTRDRV_OM_LEAVE_CRITICAL_SECTION();\
                             }

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    BOOL_T                    is_allocated;
    UI32_T                    my_unit_id;
    UI32_T                    task_id;
    void                      *timer_id;
    BOOL_T                    debug_flag;
    BOOL_T                    provision_complete;
    SWDRV_IfTableStats_T      if_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];/*changed by charles.chen   SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT */
    SWDRV_IfXTableStats_T     ifx_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    SWDRV_RmonStats_T         rmon_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    SWDRV_EtherlikeStats_T    etherlike_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    SWDRV_EtherlikePause_T    etherlike_pause[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
    SWDRV_IfXTableStats_T     ifx_stats_for_vlan[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_VLAN_ID];
#endif
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    SWDRV_IfPerQStats_T       ifperq_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_PFC == TRUE)
    SWDRV_PfcStats_T          pfc_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_CN == TRUE)
    SWDRV_QcnStats_T          qcn_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
} NMTRDRV_Shmem_Data_T;


static NMTRDRV_Shmem_Data_T   *shmem_data_p;
static UI32_T                 nmtrdrv_om_sem_id;

void NMTRDRV_OM_InitiateSystemResources(void)
{
    shmem_data_p = (NMTRDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_NMTRDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    shmem_data_p->debug_flag = FALSE;
}

void NMTRDRV_OM_AttachSystemResources(void)
{
    shmem_data_p = (NMTRDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_NMTRDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NMTRDRV_OM, &nmtrdrv_om_sem_id);
}

void NMTRDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_NMTRDRV_SHMEM_SEGID;
    *seglen_p = sizeof(NMTRDRV_Shmem_Data_T);
}

SYS_TYPE_Stacking_Mode_T NMTRDRV_OM_GetOperatingMode(void)
{
    SYS_TYPE_Stacking_Mode_T oper_mode;

    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    oper_mode = SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return oper_mode;
}

void NMTRDRV_OM_SetTransitionMode(void)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_EnterTransitionMode(void)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_EnterMasterMode(void)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_EnterSlaveMode(void)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

UI32_T NMTRDRV_OM_GetMyUnitId(void)
{
    return shmem_data_p->my_unit_id;
}

void NMTRDRV_OM_SetMyUnitId(UI32_T my_unit_id)
{
    ATOM_EXPRESSION(shmem_data_p->my_unit_id = my_unit_id)
}

BOOL_T NMTRDRV_OM_GetDebugFlag(void)
{
    BOOL_T debug_flag;

    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    debug_flag = shmem_data_p->debug_flag;
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return debug_flag;
}

void NMTRDRV_OM_SetDebugFlag(BOOL_T debug_flag)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->debug_flag = debug_flag;
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

BOOL_T NMTRDRV_OM_GetProvisionComplete(void)
{
    BOOL_T provision_complete;

    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    provision_complete = shmem_data_p->provision_complete;
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return provision_complete;
}

void NMTRDRV_OM_SetProvisionComplete(BOOL_T provision_complete)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->provision_complete = provision_complete;
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_ClearIfStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->if_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_IfTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetIfStats(UI32_T lunit,UI32_T lport, SWDRV_IfTableStats_T *if_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->if_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],if_stats_p,sizeof(SWDRV_IfTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetIfStats(UI32_T lunit,UI32_T lport, SWDRV_IfTableStats_T *if_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(if_stats_p,&shmem_data_p->if_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_IfTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_ClearIfXStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->ifx_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_IfXTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetIfXStats(UI32_T lunit,UI32_T lport, SWDRV_IfXTableStats_T *ifx_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->ifx_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],ifx_stats_p,sizeof(SWDRV_IfXTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetIfXStats(UI32_T lunit,UI32_T lport, SWDRV_IfXTableStats_T *ifx_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(ifx_stats_p,&shmem_data_p->ifx_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_IfXTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_ClearRmonStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->rmon_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_RmonStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetRmonStats(UI32_T lunit,UI32_T lport, SWDRV_RmonStats_T *rmon_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->rmon_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],rmon_stats_p,sizeof(SWDRV_RmonStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetRmonStats(UI32_T lunit,UI32_T lport, SWDRV_RmonStats_T *rmon_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(rmon_stats_p,&shmem_data_p->rmon_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_RmonStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_ClearEtherlikeStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->etherlike_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_EtherlikeStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetEtherlikeStats(UI32_T lunit,UI32_T lport, SWDRV_EtherlikeStats_T *etherlike_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->etherlike_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],etherlike_stats_p,sizeof(SWDRV_EtherlikeStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetEtherlikeStats(UI32_T lunit,UI32_T lport, SWDRV_EtherlikeStats_T *etherlike_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(etherlike_stats_p,&shmem_data_p->etherlike_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_EtherlikeStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_ClearEtherlikePause(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->etherlike_pause[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_EtherlikePause_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetEtherlikePause(UI32_T lunit,UI32_T lport, SWDRV_EtherlikePause_T *etherlike_pause_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->etherlike_pause[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],etherlike_pause_p,sizeof(SWDRV_EtherlikePause_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetEtherlikePause(UI32_T lunit,UI32_T lport, SWDRV_EtherlikePause_T *etherlike_pause_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(etherlike_pause_p,&shmem_data_p->etherlike_pause[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_EtherlikePause_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
void NMTRDRV_OM_ClearIfPerQStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->ifperq_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_IfPerQStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetIfPerQStats(UI32_T lunit,UI32_T lport, SWDRV_IfPerQStats_T *stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->ifperq_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],stats_p,sizeof(SWDRV_IfPerQStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetIfPerQStats(UI32_T lunit,UI32_T lport, SWDRV_IfPerQStats_T *stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(stats_p,&shmem_data_p->ifperq_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_IfPerQStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
void NMTRDRV_OM_ClearPfcStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->pfc_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_PfcStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetPfcStats(UI32_T lunit,UI32_T lport, SWDRV_PfcStats_T *stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->pfc_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],stats_p,sizeof(SWDRV_PfcStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetPfcStats(UI32_T lunit,UI32_T lport, SWDRV_PfcStats_T *stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(stats_p,&shmem_data_p->pfc_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_PfcStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
void NMTRDRV_OM_ClearQcnStats(UI32_T lunit,UI32_T lport)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->qcn_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1], 0, sizeof(SWDRV_QcnStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetQcnStats(UI32_T lunit,UI32_T lport, SWDRV_QcnStats_T *stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->qcn_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],stats_p,sizeof(SWDRV_QcnStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetQcnStats(UI32_T lunit,UI32_T lport, SWDRV_QcnStats_T *stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(stats_p,&shmem_data_p->qcn_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*(lunit-1)+lport-1],sizeof(SWDRV_QcnStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
void NMTRDRV_OM_ClearIfXStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T num_of_vid)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->ifx_stats_for_vlan[unit-1][start_vid-1], 0, num_of_vid * sizeof(SWDRV_IfXTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_SetIfXStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T num_of_vid, SWDRV_IfXTableStats_T *ifx_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->ifx_stats_for_vlan[unit-1][start_vid-1], ifx_stats_p, num_of_vid * sizeof(SWDRV_IfXTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void NMTRDRV_OM_GetIfXStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T num_of_vid, SWDRV_IfXTableStats_T *ifx_stats_p)
{
    NMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(ifx_stats_p, &shmem_data_p->ifx_stats_for_vlan[unit-1][start_vid-1], num_of_vid * sizeof(SWDRV_IfXTableStats_T));
    NMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

