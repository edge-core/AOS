/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file provides the APIs for POEDRV OM to read/write the database.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sys_time.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "sysrsc_mgr.h"
#include "sysfun.h"
#include "stktplg_pom.h"
#include "leaf_3621.h"
#include "backdoor_mgr.h"
#include "poedrv_type.h"
#include "poedrv_control.h"

/* NAME CONSTANT DECLARATIONS
 */
//#define POEDRV_OM_ENTER_CRITICAL_SECTION    SYSFUN_ENTER_CRITICAL_SECTION(poedrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
//#define POEDRV_OM_LEAVE_CRITICAL_SECTION    SYSFUN_LEAVE_CRITICAL_SECTION(poedrv_om_sem_id);
#define POEDRV_OM_EnterCriticalSection() SYSFUN_TakeSem(poedrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POEDRV_OM_LeaveCriticalSection() SYSFUN_GiveSem(poedrv_om_sem_id)

#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif
static UI32_T POE_PORT_L2P_MATRIX[POEDRV_NO_OF_POE_PORTS+1] = 
//{1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14,17,16,19,18,21,20,23,22};
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
static UI32_T POE_PORT_P2L_MATRIX[POEDRV_NO_OF_POE_PORTS+1] = 
//{2,1,4,3,6,5,8,7,10,9,12,11,14,13,16,15,18,17,20,19,22,21,24,23};
{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
static POEDRV_TYPE_ShmemData_T *shmem_data_p;

static UI32_T poedrv_om_sem_id;          /* PoE driver semaphore ID */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME : POEDRV_OM_Logical2PhyDevicePortID
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   port->logical port num;phy_port->address of physical port num
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T	POEDRV_OM_Logical2PhyDevicePortID(UI32_T port,UI32_T *phy_port)
{
    if(port > 0 && port <= POEDRV_NO_OF_POE_PORTS)
    {
        POEDRV_OM_EnterCriticalSection();

    	if(shmem_data_p->poedrv_port_L2P_matrix[port-1] != 0xFF)
    	{
            *phy_port = shmem_data_p->poedrv_port_L2P_matrix[port-1];

			POEDRV_OM_LeaveCriticalSection();

            return TRUE;
        }
        else
        {
			POEDRV_OM_LeaveCriticalSection();

            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

/* FUNCTION NAME : POEDRV_OM_Physical2LogicalPort
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   port->phisical port num; lport->address of logical port num
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T POEDRV_OM_Physical2LogicalPort(UI32_T port, UI32_T *lport)
{
    if(port >=0 && port < POEDRV_NO_OF_POE_PORTS)
    {
        POEDRV_OM_EnterCriticalSection();

    	if(shmem_data_p->poedrv_port_P2L_matrix[port] != 0xFF)        /* Add by Andy  1104-05 */
    	{
            *lport = shmem_data_p->poedrv_port_P2L_matrix[port];

			POEDRV_OM_LeaveCriticalSection();

            return TRUE;
        }
        else
        {
            POEDRV_OM_LeaveCriticalSection();

            return FALSE;
        }
    }
    else
        return FALSE;
}


void POEDRV_OM_Reset(void)
{
    UI32_T port;

    POEDRV_OM_EnterCriticalSection();
#if 0
    memset(shmem_data_p->swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T) *(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 1));
    memset(shmem_data_p->swdrv_trunk_info, 0, sizeof(SWDRV_Trunk_Info_T) *(SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM + 1));

    /* water_huang add; 95_9_13
     */
    memset(shmem_data_p->swdrv_port_link_status, 0, sizeof(SWDRV_LinkStatus_T)*(SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK));
    shmem_data_p->swdrv_receive_events = 0;
    #if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
    memset(shmem_data_p->combo_force_mode, 0, sizeof(UI32_T) *(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 1));
    #endif
    #if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    memset(shmem_data_p->workaround_status, 0, sizeof(UI32_T) *(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 1));
    #endif
    /* clear register function list */
    shmem_data_p->swdrv_unit_bitmap = 0;
    #if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
    shmem_data_p->swdrv_running_queue_method = 0;
    #endif
    /* clear register function list */
    shmem_data_p->swdrv_provision_complete = FALSE;
#else	

    memset(&shmem_data_p->poedrv_mainpower_info, 0, sizeof(POEDRV_Mainpower_Info_T));
    memset(shmem_data_p->poedrv_port_info, 0, sizeof(POEDRV_Port_Info_T)*(POEDRV_NO_OF_POE_PORTS));
    for (port=0;port<POEDRV_NO_OF_POE_PORTS;port++)
        memset(shmem_data_p->poedrv_port_counter[port], 0, sizeof(UI32_T)*POEDRV_MAX_COUNTER_TYPE);
    memset(shmem_data_p->poedrv_pingpong_info, 0, sizeof(POEDRV_PINGPONG_DETECTION_T)*POEDRV_NO_OF_POE_PORTS);

    for (port=0;port<POEDRV_NO_OF_POE_PORTS;port++)
    {
        shmem_data_p->poedrv_port_L2P_matrix[port] = 0xff;
        shmem_data_p->poedrv_port_P2L_matrix[port] = 0xff;
        shmem_data_p->per_port_power_max_allocation[port] = SYS_DFLT_PSE_PORT_POWER_MAX_ALLOCATION;
    }

    memcpy(shmem_data_p->poedrv_port_L2P_matrix,POE_PORT_L2P_MATRIX,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
    memcpy(shmem_data_p->poedrv_port_P2L_matrix,POE_PORT_P2L_MATRIX,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

    shmem_data_p->main_pse_power_max_allocation = POEDRV_TYPE_PSE_POWER_MAX_ALLOCATION_LOCAL;

    shmem_data_p->poedrv_min_port_number = POEDRV_TYPE_PSE_MIN_PORT_NUMBER;
    shmem_data_p->poedrv_max_port_number = POEDRV_TYPE_PSE_MAX_PORT_NUMBER;
    shmem_data_p->poedrv_poe_image_version = 0xFF;
	
    /* Initialize Main PSE with default value
     */
    shmem_data_p->poedrv_mainpower_info.unit_id              = (UI8_T)shmem_data_p->poedrv_my_unit_id;
    shmem_data_p->poedrv_mainpower_info.main_pse_power       = MIN_pethMainPsePower;
    shmem_data_p->poedrv_mainpower_info.main_pse_oper_status = VAL_pethMainPseOperStatus_faulty;
    shmem_data_p->poedrv_mainpower_info.main_pse_consumption = 0;
    shmem_data_p->poedrv_mainpower_info.legacy_detection_enable = 0;
    /* Initialize state transition on all ports
     */
    // memset(poedrv_port_state, POEDRV_PORT_IS_OFF, sizeof(UI8_T)*(POEDRV_NO_OF_POE_PORTS+1));


    /* Initialize port PSE with default value
     */
    for ( port=0; port<POEDRV_NO_OF_POE_PORTS; port++)   /* And  */
    {
        shmem_data_p->poedrv_port_info[port].detection_status  = VAL_pethPsePortDetectionStatus_disabled;
        shmem_data_p->poedrv_port_info[port].power_class       = VAL_pethPsePortPowerClassifications_class0;
        shmem_data_p->poedrv_port_info[port].power_consumption = 0;
        shmem_data_p->poedrv_port_info[port].is_overload       = FALSE;
    }
	
    shmem_data_p->is_stop_polling = TRUE;
	
    /* Initialize flag for program mode to download software to PoE controller
     */
#if 0
    shmem_data_p->is_enter_program_mode = FALSE;
#endif

    shmem_data_p->poedrv_provision_complete = FALSE;

#endif
	
    POEDRV_OM_LeaveCriticalSection();
}


void POEDRV_OM_Init(void)
{
    UI32_T ret_value;

    shmem_data_p = (POEDRV_TYPE_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_POEDRV_SHMEM_SEGID);

    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_om_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }

    POEDRV_OM_Reset();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for POEDRV_OM
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_AttachSystemResources(void)
{
    UI32_T ret_value;

    shmem_data_p = (POEDRV_TYPE_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_POEDRV_SHMEM_SEGID);

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_om_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }

}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterMasterMode (void)
{
DBG_PRINT();
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterSlaveMode (void)
{
DBG_PRINT();
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_SetTransitionMode (void)
{
DBG_PRINT();
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterTransitionMode (void)
{
DBG_PRINT();
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T POEDRV_OM_GetOperatingMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

BOOL_T POEDRV_OM_SetProvisionComplete(BOOL_T status)
{

    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_provision_complete = status;
    POEDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T POEDRV_OM_GetProvisionComplete(BOOL_T *status)
{
    POEDRV_OM_EnterCriticalSection();
    *status = shmem_data_p->poedrv_provision_complete;
    POEDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T POEDRV_OM_SetThreadId(UI32_T thread_id)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_thread_id = thread_id;
    POEDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T POEDRV_OM_GetThreadId(UI32_T *poedrv_thread_id)
{
    POEDRV_OM_EnterCriticalSection();
    *poedrv_thread_id = shmem_data_p->poedrv_thread_id;
    POEDRV_OM_LeaveCriticalSection();

    return TRUE;
}

/* FUNCTION NAME : POEDRV_OM_IsStopMonitorFlagOn
 * PURPOSE: This function is used to get the status of query function in PoE
 * INPUT   : 
 * OUTPUT  : none
 * RETURN:  TRUE  -- stop to poll
 *          FALSE -- in polling
 * NOTES:   
 */
BOOL_T POEDRV_OM_IsStopMonitorFlagOn()
{
    BOOL_T is_stop_polling;
	
    POEDRV_OM_EnterCriticalSection();
    is_stop_polling = shmem_data_p->is_stop_polling;
    POEDRV_OM_LeaveCriticalSection();

    return is_stop_polling;
}

/* FUNCTION NAME : POEDRV_OM_SetStopMonitorFlag
 * PURPOSE: This function is used to set the flag of poe monitor function
 * INPUT   : state - TRUE: stop, FALSE - polling
 * OUTPUT  : none
 * RETURN:  
 *          
 * NOTES:   
 */
void POEDRV_OM_SetStopMonitorFlag(BOOL_T state)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->is_stop_polling = state;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMyUnitID(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_my_unit_id = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMyUnitID(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_my_unit_id;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetNumOfUnits(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_num_of_units = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetNumOfUnits(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_num_of_units;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerMaxAllocation(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->main_pse_power_max_allocation;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPOEPortNumber(UI32_T *min, UI32_T *max)
{
    POEDRV_OM_EnterCriticalSection();
    *min = shmem_data_p->poedrv_min_port_number;
    *max = shmem_data_p->poedrv_max_port_number;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetImageVersion(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_poe_image_version = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetImageVersion(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_poe_image_version;
    POEDRV_OM_LeaveCriticalSection();
}


void POEDRV_OM_SetMainPowerInfoUnitID(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.unit_id = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoUnitID(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.unit_id;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoMainOperStatus(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.main_pse_oper_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoMainOperStatus(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.main_pse_oper_status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoMainPower(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.main_pse_power = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoMainPower(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.main_pse_power;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoMainConsumption(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.main_pse_consumption = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoMainConsumption(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.main_pse_consumption;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.legacy_detection_enable = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoLegacyDectionEnable(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.legacy_detection_enable;
    POEDRV_OM_LeaveCriticalSection();
}



void POEDRV_OM_SetPortInfoPowerConsumption(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].power_consumption = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoPowerConsumption(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].power_consumption;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoTemperature(UI32_T port, I32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].temperature = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoTemperature(UI32_T port, I32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].temperature;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoVoltage(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].voltage = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoVoltage(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].voltage;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoCurrent(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].current = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoCurrent(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].current;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoLedStatus(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].led_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoLedStatus(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].led_status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoDetectionStatus(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].detection_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoDetectionStatus(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].detection_status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoPowerClass(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].power_class = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoPowerClass(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].power_class;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoIsOverload(UI32_T port, BOOL_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-1].is_overload = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoIsOverload(UI32_T port, BOOL_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-1].is_overload;
    POEDRV_OM_LeaveCriticalSection();
}



void POEDRV_OM_AddPortCounter(UI32_T port, UI32_T value1, UI32_T value2)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_counter[port-1][value1] += value2;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortCounter(UI32_T port, UI32_T value1, UI32_T *value2)
{
    POEDRV_OM_EnterCriticalSection();
    *value2 = shmem_data_p->poedrv_port_counter[port-1][value1];
    POEDRV_OM_LeaveCriticalSection();
}


void POEDRV_OM_SetPingPongInfoStartTicks(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_pingpong_info[port-1].start_ticks = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPingPongInfoStartTicks(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_pingpong_info[port-1].start_ticks;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_AddPingPongInfoTimesOfPowerDenied(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_pingpong_info[port-1].times_of_power_denied += value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPingPongInfoTimesOfPowerDenied(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_pingpong_info[port-1].times_of_power_denied;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_ResetOnePingPongInfo(UI32_T port)
{
    POEDRV_OM_EnterCriticalSection();
    memset(&shmem_data_p->poedrv_pingpong_info[port-1], 0, sizeof(POEDRV_PINGPONG_DETECTION_T));
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_ResetAllPingPongInfo(void)
{
    POEDRV_OM_EnterCriticalSection();
    memset(shmem_data_p->poedrv_pingpong_info, 0, sizeof(POEDRV_PINGPONG_DETECTION_T)*POEDRV_NO_OF_POE_PORTS);
    POEDRV_OM_LeaveCriticalSection();
}


























