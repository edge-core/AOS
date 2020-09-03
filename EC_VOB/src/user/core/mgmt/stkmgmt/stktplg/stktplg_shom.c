#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "sys_bld.h"
#include "stktplg_shom.h"
#include "stktplg_board.h"
#define STKTPLG_SHOM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(stktplg_shom_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define STKTPLG_SHOM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(stktplg_shom_sem_id)
#define ATOM_EXPRESSION(exp) { STKTPLG_SHOM_ENTER_CRITICAL_SECTION();\
                               exp;\
                               STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();\
                             }

typedef struct 
{
 SYSFUN_DECLARE_CSC_ON_SHMEM
 UI32_T hguplink_state;
 UI32_T hgdonwlink_state;
 BOOL_T stacking_is_enable;
 BOOL_T config_topology_info_done;
 UI32_T uplink_port,downlink_port;
 STKTPLG_OM_Ctrl_Info_T  ctrl_info;
 STK_UNIT_CFG_T          stk_unit_cfg[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
 STKTPLG_BOARD_BoardInfo_T units_board_info_p[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
} STKTPLG_Shmem_Data_T;
static STKTPLG_Shmem_Data_T   *shmem_data_p;
static UI32_T             stktplg_shom_sem_id;

void STKTPLG_SHOM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p  = SYSRSC_MGR_STKTPLG_SHMEM_SEGID;
    *seglen_p = sizeof(STKTPLG_Shmem_Data_T);   
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_OM_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for ISC OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void STKTPLG_SHOM_InitiateSystemResources(void)
{
    shmem_data_p = (STKTPLG_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_STKTPLG_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    shmem_data_p->hgdonwlink_state = 0;
    shmem_data_p->hguplink_state = 0;
    memset(&(shmem_data_p->ctrl_info),0,sizeof(STKTPLG_OM_Ctrl_Info_T));
    memset(shmem_data_p->stk_unit_cfg,0,sizeof(shmem_data_p->stk_unit_cfg));
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_STKTPLG_SHOM, &stktplg_shom_sem_id); 
    shmem_data_p->config_topology_info_done = FALSE;

    STKTPLG_OM_InitiateSystemResources();
}
 
/*---------------------------------------------------------------------------------
 * FUNCTION : ISC_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for ISC OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void STKTPLG_SHOM_AttachSystemResources(void)
{
    shmem_data_p = (STKTPLG_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_STKTPLG_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_STKTPLG_SHOM, &stktplg_shom_sem_id);    
}

BOOL_T STKTPLG_SHOM_GetMyUnitID(UI32_T *my_unit_id)
{
   ATOM_EXPRESSION(*my_unit_id = shmem_data_p->ctrl_info.my_unit_id );
    return TRUE;
}
void STKTPLG_SHOM_SetMyUnitID(UI32_T my_unit_id)
{
   ATOM_EXPRESSION(shmem_data_p->ctrl_info.my_unit_id = my_unit_id);
    return;
}

BOOL_T STKTPLG_SHOM_SetHgDownPortLinkState(UI32_T state)
{
    ATOM_EXPRESSION(shmem_data_p->hgdonwlink_state = state);
   return TRUE;

}
BOOL_T STKTPLG_SHOM_SetHgUpPortLinkState(UI32_T state)
{
    ATOM_EXPRESSION(shmem_data_p->hguplink_state = state);
    return TRUE;

}

BOOL_T STKTPLG_SHOM_GetHgDownPortLinkState(UI32_T *state)
{

   ATOM_EXPRESSION( *state = shmem_data_p->hgdonwlink_state);

    return TRUE;

}

BOOL_T STKTPLG_SHOM_GetHgPortLinkState(UI32_T *up_state,UI32_T *down_state)
{
    STKTPLG_SHOM_ENTER_CRITICAL_SECTION();
    *down_state = shmem_data_p->hgdonwlink_state;
    *up_state = shmem_data_p->hguplink_state ;
    STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
    return TRUE;

}

BOOL_T STKTPLG_SHOM_GetHgUpPortLinkState(UI32_T *state)
{

    ATOM_EXPRESSION(*state = shmem_data_p->hguplink_state) ;
 
    return TRUE;

}

BOOL_T STKTPLG_SHOM_SetStktplgInfo(STKTPLG_OM_Ctrl_Info_T* ctrl_info,STK_UNIT_CFG_T *stk_unit_cfg,STKTPLG_BOARD_BoardInfo_T *units_board_info_p)
{
   STKTPLG_SHOM_ENTER_CRITICAL_SECTION();
   memcpy(&(shmem_data_p->ctrl_info),ctrl_info,sizeof(STKTPLG_OM_Ctrl_Info_T)) ;
   memcpy(shmem_data_p->stk_unit_cfg,stk_unit_cfg,sizeof(shmem_data_p->stk_unit_cfg)) ;
   memcpy(shmem_data_p->units_board_info_p,units_board_info_p,sizeof(shmem_data_p->units_board_info_p)) ;
    STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
    return TRUE;

}

BOOL_T STKTPLG_SHOM_GetCtrlInfo(STKTPLG_OM_Ctrl_Info_T* ctrl_info)
{
    ATOM_EXPRESSION(memcpy(ctrl_info,&(shmem_data_p->ctrl_info),sizeof(STKTPLG_OM_Ctrl_Info_T))) ; 
 
    return TRUE;

}
BOOL_T STKTPLG_SHOM_SetUnitBoardInfo(UI8_T unit, STKTPLG_BOARD_BoardInfo_T *board_info)
{
    if(unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK|| unit <1)
     return FALSE;

    ATOM_EXPRESSION(memcpy(&(shmem_data_p->units_board_info_p[unit-1]),board_info,sizeof(STKTPLG_BOARD_BoardInfo_T)));
    return TRUE;

}

BOOL_T STKTPLG_SHOM_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number)
{
    if(unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    ATOM_EXPRESSION(*max_port_number = shmem_data_p->units_board_info_p[unit-1].max_port_number_on_board);
    return TRUE;

}

BOOL_T STKTPLG_SHOM_IsOptionModule()
{
     return FALSE;

}

BOOL_T STKTPLG_SHOM_UnitExist(UI32_T unit_id)
{

    UI32_T index;

    if (unit_id >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ||unit_id <1)
        return FALSE;

  
    STKTPLG_SHOM_ENTER_CRITICAL_SECTION();
    for (index = 0; index < shmem_data_p->ctrl_info.total_units_up; index++)
    {
        if ( shmem_data_p->ctrl_info.stable_hbt_up.payload[index].unit_id == unit_id)
         {
            STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
            return TRUE;
         }
    }

    for (index = 0; index < shmem_data_p->ctrl_info.total_units_down; index++)
    {
        if ( shmem_data_p->ctrl_info.stable_hbt_down.payload[index].unit_id  == unit_id)
        {
           STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }  
    }

  
    STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
    return FALSE;

}

BOOL_T STKTPLG_SHOM_SetStackingPortInfo(UI32_T state,UI32_T uplinkport,UI32_T downlinkport)
{
   STKTPLG_SHOM_ENTER_CRITICAL_SECTION();
   shmem_data_p->stacking_is_enable = state;
   shmem_data_p->uplink_port = uplinkport ;
   shmem_data_p->downlink_port =  downlinkport;
   STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
   return TRUE;
}

BOOL_T STKTPLG_SHOM_GetStackingPortInfo(UI32_T *state,UI32_T *uplinkport,UI32_T *downlinkport)
{
    if(!state || !uplinkport || !downlinkport)
        return FALSE;
    *uplinkport = *downlinkport = 0;
    STKTPLG_SHOM_ENTER_CRITICAL_SECTION();
    *state = shmem_data_p->stacking_is_enable;
    if(shmem_data_p->stacking_is_enable)
    {
        *uplinkport =shmem_data_p->uplink_port;
        *downlinkport = shmem_data_p->downlink_port;
    }
    STKTPLG_SHOM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* FUNCTION NAME : STKTPLG_SHOM_GetConfigTopologyInfoDoneStatus
 * PURPOSE: To get the status of configuring topology info.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- STKTPLG had configured topology
 *                    info to lower layer.
 *          FALSE  -- STKTPLG had not configured
 *                    topology info to lower layer.
 * NOTES:
 *          Tables in ASIC need to be initialized after
 *          the module id had been configured through
 *          the operation "Configure Topology Info" in
 *          STKTPLG. Thus in SWDRV_EnterTransitionMode(),
 *          it needs to poll this status and call
 *          DEV_PMGR_ResetOnDemand() to re-initialize ASIC
 *          after "Configure Topology Info" is finished.
 */
BOOL_T STKTPLG_SHOM_GetConfigTopologyInfoDoneStatus(void)
{
    /* harmless to get without critical section
     */
    return shmem_data_p->config_topology_info_done;
}

/* FUNCTION NAME : STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus
 * PURPOSE: To set the status of configuring topology info.
 * INPUT:   TRUE   -- STKTPLG had configured topology
 *                    info to lower layer.
 *          FALSE  -- STKTPLG had not configured
 *                    topology info to lower layer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *          This function is called by stktplg_engine to
 *          set ths status of configuring topology info to
 *          lower layer.
 */
void STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus(BOOL_T isDone)
{
    /* harmless to set without critical section
     */
    shmem_data_p->config_topology_info_done = isDone;
}

